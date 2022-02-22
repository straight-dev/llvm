#include "StraightOptimizer2/RouteIterator.h"

#include <cassert>

namespace Optimizer2 {

	RouteIterator& RouteIterator::operator++() {
		if( iter.isTopOfBasicBlock() ) {
			const auto bb = *existsBB;
			++existsBB;
			if( *this != EndOfRange{} ) {
				iter = InstructionIterator( (*existsBB)->getBranchInstructionTo( bb ) );
			} else {
				// iterは無効
			}
		} else {
			--iter;
		}
		return *this;
	}

	exempt_ptr<Instruction> RouteIterator::operator*() const {
		return make_exempt( *iter );
	}

	bool RouteIterator::operator==( const RouteIterator& rhs ) const {
		assert( parent == rhs.parent );
		return iter == rhs.iter;
	}

	bool RouteIterator::operator!=( const RouteIterator& rhs ) const {
		assert( parent == rhs.parent );
		return iter != rhs.iter;
	}

	bool RouteIterator::operator!=( EndOfRange ) const {
		// これだとProducerが含まれなくて、用途によっては困るか？
		return *(*this) != parent->getProducer();
	}
}