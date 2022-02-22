#include "StraightPhiPass.h"
#include "StraightCodeGen.h"
#include "StraightInstrInfo.h"
#include "StraightFrameLowering.h"
#include "StraightOpTraits.h"
#include "SpillOptimizer/StraightPhiUtil.h"
#include "SpillOptimizer/StraightPhiSpillAnalysis.h"

#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

#include <map>
#include <unordered_set>
#include <vector>

#include <iomanip>
#include <iostream>
#include <string>

#define DEBUG_TYPE "straight-phi-opt-pass"

char forStraightAddPhiAndSpillPass;
char forStraightAddJmpAndBasicBlocksPass;
char forStraightAddTrampolinePass;

using namespace llvm::StraightSpillOptimizer;

class BB_Vreg_Info {
	// mapは配列アクセスすると変更されうるのでconst関数で使えない（不便）が、変わっても無害なのでmutable
	mutable std::map<LLVMBasicBlockID, std::map<LLVMVarReg, const llvm::MachineInstr*>> phi;
	mutable std::map<LLVMBasicBlockID, std::map<LLVMVarReg, const llvm::MachineInstr*>> spill_in;
	mutable std::map<LLVMVarReg, FrameIndex> spill_slot;
public:
	void record_phi(const llvm::MachineBasicBlock& bb, const llvm::MachineOperand& operand, const llvm::MachineInstr* phi_instr) { phi[toID(bb)][toID(operand)] = phi_instr; }
	const llvm::MachineInstr* find_phi(const llvm::MachineBasicBlock& bb, const llvm::MachineOperand& operand) const { return phi[toID(bb)][toID(operand)]; }

	void record_spill_frame_index(const llvm::MachineOperand& operand, FrameIndex frame_index) { spill_slot[toID(operand)] = frame_index; }
	FrameIndex find_spill_frame_index(const llvm::MachineOperand& operand) const { return find_spill_frame_index(toID(operand)); }
	FrameIndex find_spill_frame_index(LLVMVarReg lvreg) const { return spill_slot.count(lvreg) ? spill_slot[lvreg] : not_found_FrameIndex; }

	void record_spill_in(const llvm::MachineBasicBlock& bb, LLVMVarReg lvreg, const llvm::MachineInstr* spill_in_instr) { spill_in[toID(bb)][lvreg] = spill_in_instr; }
	const llvm::MachineInstr* find_spill_in(const llvm::MachineBasicBlock* bb, const llvm::MachineOperand& operand) const {
		while (spill_in[toID(*bb)][toID(operand)] == nullptr) {
            if (bb->pred_size() != 1) {
				// 見つからなかった
				return nullptr;
            }
			assert( bb->pred_size() == 1 );
			bb = *bb->pred_begin();
		}
		return spill_in[toID(*bb)][toID(operand)];
	}
};

class AddPhiAndSpill : private BB_Vreg_Info {
	PhiSpillAnalysis PSA;
	
	void AddPhiForSTRAIGHT(llvm::MachineFunction& MF);
	void AddSpillOutForSTRAIGHT(llvm::MachineFunction& MF);
	void AddSpillInForSTRAIGHT(llvm::MachineFunction& MF);
	void ChangeNormalInstructionOperandToPhi(llvm::MachineFunction& MF);
	void ChangePhiInstructionOperandToPhi(llvm::MachineFunction& MF);
	void ChangeNormalInstructionOperandToSpillIn(llvm::MachineFunction& MF);
	void ChangePhiInstructionOperandToSpillIn(llvm::MachineFunction& MF);
public:
	AddPhiAndSpill(llvm::MachineFunction& MF, const llvm::MachineLoopInfo& MLI);
};

void AddPhiAndSpill::AddPhiForSTRAIGHT(llvm::MachineFunction& MF) {
	for (const auto& basic_block : MF) {
		for (const auto& instruction : basic_block.instrs()) {
			if (srcOperandBegin(instruction) == 0) { continue; }
			const auto& dest = instruction.getOperand(0);
			if (isVarReg(dest)) {
				for (auto& bb : MF) {
					if (PSA.needsPhi(toID(bb), toID(dest))) {
						const auto*const new_phi_instr = CreatePhi(MF, bb, instruction);
						record_phi(bb, dest, new_phi_instr);
					}
				}
			}
		}
	}
}
void AddPhiAndSpill::AddSpillOutForSTRAIGHT(llvm::MachineFunction& MF) {
	for (auto& MBB : MF) {
		for (auto II = MBB.instr_begin(); II != MBB.instr_end(); ++II) {
			const auto& dest = (*II).getOperand(0);
			if (llvm::Straight::has_dstReg(*II) && PSA.needsSpillOut(toID(MBB), toID(dest))) {
				const auto& TII = *MF.getSubtarget().getInstrInfo();
				const auto& DL = (*II).getDebugLoc();
				constexpr unsigned RegisterSize = 8;

				auto& MFI = MF.getFrameInfo();
				const auto newSpillFI = FrameIndex{MFI.CreateSpillStackObject(RegisterSize, llvm::Align(8))};

				record_spill_frame_index(dest, newSpillFI);

				BuildMI(MBB, MBB.SkipPHIsAndLabels(std::next(II)), DL, TII.get(llvm::Straight::ST_64))
					.addReg(dest.getReg())
					.addFrameIndex(static_cast<int>(newSpillFI))
					.addReg(llvm::Straight::ZeroReg);
			} // ++IIすると、この追加した命令を示すことになるが、has_destReg(*II)がfalseなので問題ない
		}
	}
}
void AddPhiAndSpill::AddSpillInForSTRAIGHT(llvm::MachineFunction& MF) {
	for (auto [bb, lvreg] : PSA.eachNeedsSpillIn()) {
		auto& MBB = *MF.getBlockNumbered(static_cast<unsigned int>(bb));
		const auto& TII = *MF.getSubtarget().getInstrInfo();
		const auto II = MBB.getFirstNonPHI();

		const FrameIndex fi = find_spill_frame_index(lvreg);

		auto& MRI = MF.getRegInfo();
		const unsigned newReg = MRI.createVirtualRegister(&llvm::Straight::GPRRegClass);

		const auto MIB = BuildMI(MBB, II, llvm::DebugLoc(), TII.get(llvm::Straight::LD_64), newReg)
			.addFrameIndex(static_cast<int>(fi))
			.addReg(llvm::Straight::ZeroReg);

		record_spill_in(MBB, lvreg, MIB.getInstr());
	}
}
void AddPhiAndSpill::ChangeNormalInstructionOperandToPhi(llvm::MachineFunction& MF) {
	for (auto& basic_block : MF) {
		// 上方を見ていくとphiより先に関数コールが見つかる場合は付け替えない
		// SpillInがあってそれにつけ変わるはず
		if (!getFirstJoinBasicBlockWithoutFunctionCall(basic_block)) { continue; }
		// SpillInが近くにあっても、上方のphiに付け替えたほうがよいので、先に探索する

		const auto& phi_bb = getFirstJoinBasicBlock(basic_block);
		for (auto& instruction : basic_block.instrs()) {
			if (instruction.isPHI()) { continue; }
			for (unsigned int i = srcOperandBegin(instruction); i < instruction.getNumOperands(); ++i) {
				auto& operand = instruction.getOperand(i);
				if (isVarReg(operand)) {
					if (const auto*const phi = find_phi(phi_bb, operand)) {
						operand.ChangeToRegister(phi->getOperand(0).getReg(), false);
					}
				}
			}
		}
	}
}
void AddPhiAndSpill::ChangePhiInstructionOperandToPhi(llvm::MachineFunction& MF) {
	for (auto& basic_block : MF) {
		for (auto& instruction : basic_block.instrs()) {
			if (!instruction.isPHI()) { continue; }
			for (unsigned int i = 1; i < instruction.getNumOperands(); i += 2) {
				auto& operand = instruction.getOperand(i);
				const auto& incoming_bb = *instruction.getOperand(i+1).getMBB();

				// 上方を見ていくとphiより先に関数コールが見つかる場合は付け替えない
				// SpillInがあってそれにつけ変わるはず
				if (!getFirstJoinBasicBlockWithoutFunctionCall(incoming_bb)) { continue; }
				// SpillInが近くにあっても、上方のphiに付け替えたほうがよいので、先に探索する

				const auto& phi_bb = getFirstJoinBasicBlock(incoming_bb);
				if (const auto*const phi = find_phi(phi_bb, operand)) {
					operand.ChangeToRegister(phi->getOperand(0).getReg(), false);
				}
			}
		}
	}
}
void AddPhiAndSpill::ChangeNormalInstructionOperandToSpillIn(llvm::MachineFunction& MF) {
	for (auto& basic_block : MF) {
		for (auto& instruction : basic_block.instrs()) {
			if (instruction.isPHI()) { continue; }
			for (unsigned int i = srcOperandBegin(instruction); i < instruction.getNumOperands(); ++i) {
				auto& operand = instruction.getOperand(i);
				if (isVarReg(operand)) {
					if (const auto*const spill_in = find_spill_in(&basic_block, operand)) {
						operand.ChangeToRegister(spill_in->getOperand(0).getReg(), false);
					}
				}
			}
		}
	}
}
void AddPhiAndSpill::ChangePhiInstructionOperandToSpillIn(llvm::MachineFunction& MF) {
	for (auto& basic_block : MF) {
		for (auto& instruction : basic_block.instrs()) {
			if (!instruction.isPHI()) { continue; }
			for (unsigned int i = 1; i < instruction.getNumOperands(); i += 2) {
				auto& operand = instruction.getOperand(i);
				const auto*const incoming_bb = instruction.getOperand(i+1).getMBB();
				if (isVarReg(operand)) {
					if (const auto*const spill_in = find_spill_in(incoming_bb, operand)) {
						operand.ChangeToRegister(spill_in->getOperand(0).getReg(), false);
					}
				}
			}
		}
	}
}


AddPhiAndSpill::AddPhiAndSpill(llvm::MachineFunction& MF, const llvm::MachineLoopInfo& MLI) : PSA(MF, MLI) {
	AddPhiForSTRAIGHT(MF);
	AddSpillOutForSTRAIGHT(MF);
	AddSpillInForSTRAIGHT(MF);
	ChangeNormalInstructionOperandToPhi(MF);
	ChangePhiInstructionOperandToPhi(MF);
	ChangeNormalInstructionOperandToSpillIn(MF);
	ChangePhiInstructionOperandToSpillIn(MF);
}


namespace llvm {

	void addImplicitDefinedRetAddr(llvm::MachineFunction& MF) {
		auto& MBB = *MF.begin();
		const auto& II = MBB.instr_begin();
		const auto& TII = *MF.getSubtarget().getInstrInfo();
		const auto& DL = II == MBB.instr_end() ? DebugLoc() : (*II).getDebugLoc();

		auto& MRI = MF.getRegInfo();
		const unsigned newRetAddrReg = MRI.createVirtualRegister(&llvm::Straight::GPRRegClass);

		// 全てのRETADDRの参照をnewRetAddrRegに置換
		for (auto& basic_block : MF) {
			for (auto& instruction : basic_block.instrs()) {
				if (instruction.isPHI()) { continue; }
				for (unsigned int i = srcOperandBegin(instruction); i < instruction.getNumOperands(); ++i) {
					auto& operand = instruction.getOperand(i);
					if (isVarReg(operand) && operand.getReg() == llvm::Straight::RETADDR) {
						operand.ChangeToRegister(newRetAddrReg, false);
					}
				}
			}
		}
		// RETADDRをコピーする命令を挿入(上のループで書き換えられないように後で追加)
		BuildMI(MBB, II, DL, TII.get(llvm::Straight::COPY), newRetAddrReg)
			.addReg(llvm::Straight::RETADDR);
	}

	bool StraightAddPhiAndSpillPass::runOnMachineFunction(MachineFunction& MF) {
		addImplicitDefinedRetAddr(MF);
		const auto& MLI = getAnalysis<MachineLoopInfo>();
		AddPhiAndSpill _(MF, MLI);

		// これがないとassertで落ちる(LLVM6)
		MF.getProperties().set(llvm::MachineFunctionProperties::Property::NoVRegs);

		return true;
	}

	// optionalの代わりのPointer/nullptr
	MachineInstr* fromConditionalBranch(MachineBasicBlock*const from_block, const MachineBasicBlock& to_block) {
		for (auto& instr : *from_block) {
			if (instr.isConditionalBranch()) {
				assert(instr.getOperand(2).isMBB());
				if (instr.getOperand(2).getMBB()->getNumber() == to_block.getNumber()) {
					return &instr;
				}
			}
		}
		return nullptr;
	}

	int countFixedRegion(MachineFunction::iterator block) {
		// このBasicBlockのFixed領域で実行される命令数を計算
		MachineFunction::iterator merge = block->getParent()->end();
		for (auto succ_block : block->successors()) {
			if (succ_block->begin() != succ_block->end() && succ_block->begin()->isPHI()) {
				if (merge != block->getParent()->end()) {
					llvm_unreachable("危険辺が取り除かれていない！");
				} else {
					merge = succ_block->getIterator();
				}
			}
		}
		if (merge == block->getParent()->end()) {
			return 0;
		}
		// このBasicBlockにfixed領域ができる＝このBasicBlockの次のBasicBlockのうち一つが合流点である場合
		int instruction_num = 0;
		for (const auto& inst : merge->instrs()) {
			if (inst.isPHI()) {
				// 次のBasicBlockにあるPHI命令一つにつきRMOV命令が一つ追加される
				++instruction_num;
			}
		}
		return instruction_num;
	}

	int countInstruction(MachineFunction::iterator block) {
		// このBasicBlockで実行される命令数を計算
		int instruction_num = 0;
		for (const auto& inst : block->instrs()) {
			if (!inst.isPHI()) {
				// この命令自体
				++instruction_num;
			}
		}
		return instruction_num;
	}


	int countInstructionsBetween(const MachineFunction& MF, MachineFunction::iterator from_block, MachineFunction::iterator to_block) {
		bool from_to;
		for (auto bb_iter = MF.begin(); bb_iter != MF.end(); ++bb_iter) {
			if (bb_iter == to_block) { from_to = false; break; }	// toの方はBB頭なので先
			if (bb_iter == from_block) { from_to = true; break; }	// fromの方はBB尾なので後
		}
		if (from_to) {
			// fromの方がPCの小さい位置にある
			int instruction_num = countFixedRegion(from_block);
			for (; ++from_block != to_block;) {
				instruction_num += countInstruction(from_block) + countFixedRegion(from_block);
			}
			return instruction_num;
		} else {
			// toの方がPCの小さい位置にある
			int instruction_num = countInstruction(to_block) + countFixedRegion(to_block);
			for (; ++to_block != from_block;) {
				instruction_num += countInstruction(to_block) + countFixedRegion(to_block);
			}
			return instruction_num;
		}
	}

	// ------ Add JMP and Basic Blocks Pass ------
	bool StraightAddJmpAndBasicBlocksPass::runOnMachineFunction(MachineFunction& MF) {
		std::unordered_set<int> created_bb;
		for (auto bb_iter = MF.begin(); bb_iter != MF.end(); ++bb_iter) {
			auto& basic_block = *bb_iter;
			// 二か所以上から入ってくるBBに入場する際の加工
			//   Bccで入ってくる場合は、その直前に新たなBBを作る
			//   fallthroughで入ってくる場合は、JMP命令を付ける(上で作った新たなBBを跳び越すため)
			if (basic_block.pred_size() > 1) {
				for (auto*const pred_block : basic_block.predecessors()) {
					if (created_bb.find(pred_block->getNumber()) != created_bb.end()) {
						// 以前にこのBasicBlockを作成済み、処理不要
						continue;
					}
					auto*const from_conditional_branch_instr = fromConditionalBranch(pred_block, basic_block);
					if (from_conditional_branch_instr) {
						// llvm/lib/Target/X86/X86CmovConversion.cpp を参考にした
						// BB作成
						const BasicBlock* BB = basic_block.getBasicBlock();
						MachineBasicBlock* newMBB = MF.CreateMachineBasicBlock(BB);
						MF.insert(basic_block.getIterator(), newMBB);

						// 作成したBBの末尾にJMP命令を追加
						const TargetInstrInfo*const TII = MF.getSubtarget().getInstrInfo();
						const DebugLoc DL = basic_block.empty() ? DebugLoc() : basic_block.front().getDebugLoc();
						MachineInstrBuilder MIB = BuildMI(*newMBB, newMBB->end(), DL, TII->get(Straight::JMP));
						MIB.addMBB(&basic_block);
						newMBB->addSuccessor(&basic_block, BranchProbability::getOne());

						// Bccを、作成したBBに飛ぶように変更
						from_conditional_branch_instr->getOperand(2).setMBB(newMBB);
						pred_block->addSuccessor(newMBB);
						pred_block->removeSuccessor(&basic_block);

						// このBBのPHI命令の出所を新しく作ったBBに変更
						for( auto& instr : basic_block ) {
							if( instr.isPHI() ) {
								for( unsigned int n = 2; n < instr.getNumOperands(); n += 2 ) {
									if( instr.getOperand( n ).getMBB()->getNumber() == pred_block->getNumber() ) {
										instr.getOperand( n ).setMBB( newMBB );
									}
								}
							}
						}
						// イテレータが無効になるので最初から
						created_bb.insert(newMBB->getNumber());
						bb_iter = MF.begin();
						break;
					} else {
						if (pred_block->begin() == pred_block->end() || pred_block->back().getOpcode() != Straight::JMP) {
							// JMPがないBBの末尾にJMP命令を追加
							const TargetInstrInfo*const TII = MF.getSubtarget().getInstrInfo();
							const DebugLoc DL = basic_block.empty() ? DebugLoc() : basic_block.front().getDebugLoc();

							MachineInstrBuilder MIB = BuildMI(*pred_block, pred_block->end(), DL, TII->get(Straight::JMP));
							MIB.addMBB(&basic_block);
						} else {
							assert(pred_block->back().getOpcode() == Straight::JMP);
							if (pred_block->succ_size() > 1) {
								auto &jump_instr = pred_block->back();
								const BasicBlock* BB = basic_block.getBasicBlock();
								MachineBasicBlock* newMBB = MF.CreateMachineBasicBlock(BB);
								MF.insert(++pred_block->getIterator(), newMBB);

								// 作成したBBの末尾にJMP命令を追加
								const TargetInstrInfo*const TII = MF.getSubtarget().getInstrInfo();
								const DebugLoc DL = basic_block.empty() ? DebugLoc() : basic_block.front().getDebugLoc();
								MachineInstrBuilder MIB = BuildMI(*newMBB, newMBB->end(), DL, TII->get(Straight::JMP));
								MIB.addMBB(&basic_block);
								newMBB->addSuccessor(&basic_block, BranchProbability::getOne());

								// jmpを、作成したBBに飛ぶように変更
								jump_instr.getOperand(0).setMBB(newMBB);
								pred_block->addSuccessor(newMBB);
								pred_block->removeSuccessor(&basic_block);

								// このBBのPHI命令の出所を新しく作ったBBに変更
								for( auto& instr : basic_block ) {
									if( instr.isPHI() ) {
										for( unsigned int n = 2; n < instr.getNumOperands(); n += 2 ) {
											if( instr.getOperand( n ).getMBB()->getNumber() == pred_block->getNumber() ) {
												instr.getOperand( n ).setMBB( newMBB );
											}
										}
									}
								}
								// イテレータが無効になるので最初から
								created_bb.insert(newMBB->getNumber());
								bb_iter = MF.begin();
								break;
							}
						}
					}
				}

			} else if (basic_block.pred_size() == 1) {
				auto*const pred_block = *basic_block.predecessors().begin();
				auto*const from_conditional_branch_instr = fromConditionalBranch(pred_block, basic_block);

				if (from_conditional_branch_instr) {
					// このBasicBlockに飛んでくる分岐命令があるなら、処理不要
				} else if (pred_block->begin() == pred_block->end() || (pred_block->back().getOpcode() != Straight::JMP && pred_block->back().getOpcode() != Straight::JAL && pred_block->back().getOpcode() != Straight::JALR)) {
					// JMPがないBBの末尾にJMP命令を追加
					const TargetInstrInfo*const TII = MF.getSubtarget().getInstrInfo();
					const DebugLoc DL = basic_block.empty() ? DebugLoc() : basic_block.front().getDebugLoc();

					MachineInstrBuilder MIB = BuildMI(*pred_block, pred_block->end(), DL, TII->get(Straight::JMP));
					MIB.addMBB(&basic_block);
				}
			}
		}

		return true;
	}

	// ------ Add Trampoline Pass ------
	bool StraightAddTrampolinePass::runOnMachineFunction(MachineFunction& MF) {
		// あまりに長いオフセットを持つ条件分岐の場合はそのBBの直後に新たなBBを設け、そこをクッションにしてジャンプする
		// TODO: 分岐の条件を入れ替えれば長いオフセットにならないかも？
		for (auto bb_iter = MF.begin(); bb_iter != MF.end();) {
			auto& basic_block = *bb_iter;
			for (auto& instr : basic_block) {
				if (instr.isConditionalBranch()) {
					assert(instr.getOperand(2).isMBB());
					auto*const brTarget = instr.getOperand(2).getMBB();
					const int instruction_num = countInstructionsBetween(MF, bb_iter, brTarget->getIterator());

					// 12bitのオフセットに収めるには1024命令以内に抑えないといけない
					// Globalや二命令かかるスタックアクセス、中継のRMOVなどを考慮して計算していないので、適当に余裕を見る必要がある
					constexpr int max_instruction_num = 512;

					if (instruction_num > max_instruction_num) {
						// BB作成
						const BasicBlock* BB = basic_block.getBasicBlock();
						MachineBasicBlock* newMBB = MF.CreateMachineBasicBlock(BB);
						MF.insert(++basic_block.getIterator(), newMBB);

						// 作成したBBの末尾にJMP命令を追加
						const TargetInstrInfo*const TII = MF.getSubtarget().getInstrInfo();
						const DebugLoc DL = basic_block.empty() ? DebugLoc() : basic_block.front().getDebugLoc();
						MachineInstrBuilder MIB = BuildMI(*newMBB, newMBB->end(), DL, TII->get(Straight::JMP));
						MIB.addMBB(brTarget);
						newMBB->addSuccessor(brTarget, BranchProbability::getOne());

						// Bccを、作成したBBに飛ぶように変更
						instr.getOperand(2).setMBB(newMBB);
						basic_block.addSuccessor(newMBB);
						basic_block.removeSuccessor(brTarget);

						// brTargetBBのPHI命令の出所を新しく作ったBBに変更
						for (auto& instr : *brTarget) {
							if (instr.isPHI()) {
								for (unsigned int n = 2; n < instr.getNumOperands(); n += 2) {
									if (instr.getOperand(n).getMBB()->getNumber() == basic_block.getNumber()) {
										instr.getOperand(n).setMBB(newMBB);
									}
								}
							}
						}
						// イテレータが無効になるので最初から
						bb_iter = MF.begin();
						break;
					}
				}
			}
			++bb_iter;
		}

		return true;
	}
}
