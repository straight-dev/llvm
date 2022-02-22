#ifndef LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_ROUTERANGE_H
#define LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_ROUTERANGE_H
#include "StraightOptimizer2/Optimizer2.h"
#include "StraightOptimizer2/RouteIterator.h"

#include <stack>

namespace Optimizer2 {

	// consumer ->-> producer (not contain producer)
	using ReverseRouteRange = RouteIterator;

	// producer ->-> consumer (not contain producer)
	class RouteRange {
		struct EndOfRange {};
		std::stack<exempt_ptr<Instruction>> elements;
	public:
		explicit RouteRange( exempt_ptr<const RegOperand> parent ) {
			for( auto elem : ReverseRouteRange( parent ) ) {
				elements.emplace( std::move( elem ) );
			}
		}
		auto& operator++() { elements.pop(); return *this; }
		auto operator*() const { return elements.top(); }

		// SSIteratorRange interface
		auto begin() const { return *this; }
		auto end() const { return EndOfRange {}; }
		bool operator!=( EndOfRange ) const { return !elements.empty(); }

	};

} // namespace Optimizer2
#endif // LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_ROUTERANGE_H
