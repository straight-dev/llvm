#ifndef LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_ROUTEITERATOR_H
#define LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_ROUTEITERATOR_H
#include "StraightOptimizer2/Optimizer2.h"

namespace Optimizer2 {

	class RouteIterator {
		struct EndOfRange {};
		exempt_ptr<const RegOperand> parent;
		decltype(parent->getTransitBasicBlocks().begin()) existsBB;
		InstructionIterator iter;
	public:
		RouteIterator() = default;
		RouteIterator( const RouteIterator& ) = default;
		RouteIterator& operator=( const RouteIterator& ) = default;
		explicit RouteIterator( exempt_ptr<const RegOperand> parent )
			: parent( parent ), existsBB( parent->getTransitBasicBlocks().begin() ), iter( parent->getConsumer() ) {
			assert( parent->getProducer()->getParent() == parent->getTransitBasicBlocks().back() );
		}
		RouteIterator& operator++();
		exempt_ptr<Instruction> operator*() const;
		bool operator==( const RouteIterator& rhs ) const;
		bool operator!=( const RouteIterator& rhs ) const;


		// SSIteratorRange interface
		auto begin() const { return *this; }
		auto end() const { return EndOfRange{}; }
		bool operator!=( EndOfRange ) const;
	};

} // namespace Optimizer2
#endif // LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_ROUTEITERATOR_H
