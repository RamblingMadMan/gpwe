#include "gpwe/physics.hpp"

using namespace gpwe;
using namespace gpwe::physics;

template<class Iter, class T>
inline Iter binary_find(Iter begin, Iter end, T val){
	Iter i = std::lower_bound(begin, end, val);

	if(i != end && !(val < *i)){
		return i;
	}
	else{
		return end;
	}
}

template<class Iter, class T, class Compare>
inline Iter binary_find(Iter begin, Iter end, T val, Compare cmp){
	Iter i = std::lower_bound(begin, end, val, cmp);

	if(i != end && !cmp(val, *i)){
		return i;
	}
	else{
		return end;
	}
}

template<typename T>
inline T *insertUnique(List<UniquePtr<T>> &xs, UniquePtr<T> ptr){
	if(!ptr) return nullptr;
	auto ret = ptr.get();
	auto it = std::upper_bound(xs.begin(), xs.end(), ptr);
	xs.insert(it, std::move(ptr));
	return ret;
}

template<typename T>
inline bool eraseUnique(List<UniquePtr<T>> &xs, T *ptr){
	auto it = binary_find(
		xs.begin(), xs.end(), ptr,
		[](auto &&lhs, auto &&rhs){
			using Lhs = std::remove_cv_t<std::remove_reference_t<decltype(lhs)>>;
			using Rhs = std::remove_cv_t<std::remove_reference_t<decltype(rhs)>>;

			if constexpr(std::is_same_v<Lhs, Rhs>){
				return lhs < rhs;
			}
			else if constexpr(std::is_same_v<Lhs, UniquePtr<T>>){
				return lhs.get() < rhs;
			}
			else if constexpr(std::is_same_v<Rhs, UniquePtr<T>>){
				return lhs < rhs.get();
			}
		}
	);

	if(it != xs.end()){
		xs.erase(it);
		return true;
	}

	return false;
}

void physics::Manager::update(float dt){
	for(auto &&world : managed<World>()){
		world->update(dt);
	}
}
