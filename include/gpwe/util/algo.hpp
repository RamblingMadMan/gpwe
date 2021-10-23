#ifndef GPWE_ALGO_HPP
#define GPWE_ALGO_HPP 1

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <tuple>

#include "types.hpp"
#include "meta.hpp"
#include "Vector.hpp"

namespace gpwe{
#ifdef __GNUC__
	inline Int32 popCount(Nat32 bits) noexcept{ return __builtin_popcount(bits); }
	inline Int64 popCount(Nat64 bits) noexcept{ return __builtin_popcountll(bits); }
#elif defined(_MSC_VER)
	inline Int32 popCount(Nat32 bits) noexcept{ return __popcnt(bits); }
	inline Int64 popCount(Nat64 bits) noexcept{ return __popcnt64(bits); }
#endif

	template<typename C, typename V, class Compare = std::less<void>>
	inline auto binaryFind(C &&c, V &&v, Compare cmp = Compare{})
		-> typename std::decay_t<C>::const_iterator
	{
		// these forward probably aren't necessary
		// probably...
		auto cBeg = std::forward<C>(c).begin();
		auto cEnd = std::forward<C>(c).end();

		auto it = std::lower_bound(cBeg, cEnd, std::forward<V>(v), cmp);

		if(it != cEnd && !cmp(std::forward<V>(v), *it)){
			return it;
		}

		return cEnd;
	}

	namespace detail{
		template<typename Types, typename Void = void>
		struct MapHelper{
			using Results = meta::Map<meta::Fn<std::decay>, Types>;
			using Result = meta::Instantiate<std::tuple, Results>;

			template<typename ... Args>
			static Result result(Args &&... args){
				return std::make_tuple(std::forward<Args>(args)...);
			}
		};

		template<typename ... Xs>
		struct MapHelper<
			meta::Types<Xs...>,
			std::void_t<std::common_type_t<std::invoke_result_t<Xs>...>>
		>
		{
			using FResult = std::common_type_t<std::invoke_result_t<Xs>...>;
			using Result = Vector<std::decay_t<FResult>>;

			template<typename ... Args>
			static Result result(Args &&... args){
				return Result{ std::forward<Args>(args)... };
			}
		};

		template<typename Tuple>
		struct MapTupleHelper;

		template<typename ... Xs>
		struct MapTupleHelper<std::tuple<Xs...>>{
			template<typename F, typename Tuple>
			static auto result(F &&f, Tuple &&tup){
				return resultImpl(
					std::forward<F>(f),
					std::forward<Tuple>(tup),
					std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}
				);
			}

			template<typename F, typename Tuple, std::size_t ... Indices>
			static auto resultImpl(F &&f, Tuple &&tup, std::index_sequence<Indices...>){
				return MapHelper<meta::Types<Xs...>>::result(
					std::forward<F>(f),
					std::get<Indices>(std::forward<Tuple>(tup))...
				);
			}
		};
	}

	template<typename F, typename ... Xs>
	inline auto map(F &&f, Xs &&... xs){
		return detail::MapHelper<F, meta::Types<Xs...>>::result(
			std::forward<F>(f)(std::forward<Xs>(xs))...
		);
	}

	template<typename F, typename Tuple>
	inline auto mapTuple(F &&f, Tuple &&tup){
		return detail::MapTupleHelper<std::decay_t<Tuple>>::result(
			std::forward<F>(f), std::forward<Tuple>(tup)
		);
	}
}

#endif // !GPWE_ALGO_HPP
