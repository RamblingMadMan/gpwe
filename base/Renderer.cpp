#include <type_traits>

#include "gpwe/Renderer.hpp"

using namespace gpwe;

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
inline T *insertUnique(plf::list<std::unique_ptr<T>> &xs, std::unique_ptr<T> ptr){
	if(!ptr) return nullptr;
	auto ret = ptr.get();
	auto it = std::upper_bound(xs.begin(), xs.end(), ptr);
	xs.insert(it, std::move(ptr));
	return ret;
}

template<typename T>
inline bool eraseUnique(plf::list<std::unique_ptr<T>> &xs, T *ptr){
	auto it = binary_find(
		xs.begin(), xs.end(), ptr,
		[](auto &&lhs, auto &&rhs){
			using Lhs = std::remove_cv_t<std::remove_reference_t<decltype(lhs)>>;
			using Rhs = std::remove_cv_t<std::remove_reference_t<decltype(rhs)>>;

			T *lhsPtr, *rhsPtr;

			if constexpr(std::is_same_v<Lhs, std::unique_ptr<T>>){
				lhsPtr = lhs.get();
			}
			else{
				lhsPtr = lhs;
			}

			if constexpr(std::is_same_v<Rhs, std::unique_ptr<T>>){
				rhsPtr = rhs.get();
			}
			else{
				rhsPtr = rhs;
			}

			return lhsPtr < rhsPtr;
		}
	);

	if(it != xs.end()){
		xs.erase(it);
		return true;
	}

	return false;
}

RenderGroup *Renderer::createGroup(std::uint32_t numShapes, const Shape **shapes){ return insertUnique(m_groups, doCreateGroup(numShapes, shapes)); }
bool Renderer::destroyGroup(RenderGroup *group){ return eraseUnique(m_groups, group); }

RenderProgram *Renderer::createProgram(RenderProgram::Kind kind, std::string_view src){ return insertUnique(m_progs, doCreateProgram(kind, src)); }
bool Renderer::destroyProgram(RenderProgram *program){ return eraseUnique(m_progs, program); }

RenderPipeline *Renderer::createPipeline(const std::vector<RenderProgram*> &programs){ return insertUnique(m_pipelines, doCreatePipeline(programs)); }
bool Renderer::destroyPipeline(RenderPipeline *pipeline){ return eraseUnique(m_pipelines, pipeline); }

RenderFramebuffer *Renderer::createFramebuffer(std::uint16_t w, std::uint16_t h, const std::vector<Texture::Kind> &attachments){
	return insertUnique(m_fbs, doCreateFramebuffer(w, h, attachments));
}

bool Renderer::destroyFramebuffer(RenderFramebuffer *fb){ return eraseUnique(m_fbs, fb); }
