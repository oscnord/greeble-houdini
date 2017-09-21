#pragma once
#include <SYS/SYS_Math.h>

namespace hreeble {
	template <template <class> class T, class S> inline
	const S& rand_choice(const T<S> &collection, uint &seed) {
		auto index = SYSfastFloor(SYSfastRandom(seed) * collection.entries());
		return collection(index);
	}

	inline
	bool rand_bool(const uint &seed) {
		uint seed_ = seed;
		return SYSfastRandom(seed_) > 0.5 ? true : false;
	}
}

