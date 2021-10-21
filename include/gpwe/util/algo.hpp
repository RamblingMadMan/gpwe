#ifndef GPWE_ALGO_HPP
#define GPWE_ALGO_HPP 1

#include <tuple>

namespace gpwe{
	template<typename C, typename V, class Compare = std::less<void>>
	auto binaryFind(const C &c, V &&v, Compare cmp = Compare{}) -> typename C::const_iterator{
		auto cBeg = c.begin();
		auto cEnd = c.end();

		auto it = std::lower_bound(cBeg, cEnd, std::forward<V>(v), cmp);

		if(it != cEnd && !cmp(std::forward<V>(v), *it)){
			return it;
		}

		return cEnd;
	}

	template<typename F, typename ... XS>
	auto map(F &&f, XS &&... xs){
		return std::make_tuple(std::forward<F>(f)(std::forward<XS>(xs))...);
	}
}

#endif // !GPWE_ALGO_HPP
