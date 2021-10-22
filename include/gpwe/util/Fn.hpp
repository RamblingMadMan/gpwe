#ifndef GPWE_FN_HPP
#define GPWE_FN_HPP 1

#include <exception>

#include "Allocator.hpp"

namespace gpwe{
	class EmptyFnError: public std::exception{
		public:
			const char *what() const noexcept override{ return "empty function called"; }
	};

	template<typename...>
	class Fn;

	template<typename Ret, typename ... Args>
	class Fn<Ret(Args...)>{
		public:
			using Sig = Ret(Args...);
			using FnPtr = Sig*;

			Fn() noexcept
				: Fn([](Args...) -> Ret{ throw EmptyFnError{}; }){}

			template<typename F>
			Fn(F &&f) noexcept{
				new(m_fnBuffer) Functor<F>(std::forward<F>(f));
			}

			Fn(Ret(*ptr)(Args...)) noexcept{
				new(m_fnBuffer) Functor<Ret(*)(Args...)>(ptr);
			}

			Fn(Fn &&other) noexcept{
				auto otherFn = reinterpret_cast<FunctorBase*>(other.m_fnBuffer);
				otherFn->cloneMove(m_fnBuffer);
				other.reset();
			}

			Fn(const Fn &other) noexcept{
				auto otherFn = reinterpret_cast<const FunctorBase*>(other.m_fnBuffer);
				otherFn->clone(m_fnBuffer);
			}

			~Fn(){
				reinterpret_cast<FunctorBase*>(m_fnBuffer)->~FunctorBase();
			}

			Fn &operator=(Fn &&other) noexcept{
				if(&other != this){
					reinterpret_cast<FunctorBase*>(m_fnBuffer)->~FunctorBase();
					auto otherFn = reinterpret_cast<FunctorBase*>(other.m_fnBuffer);
					otherFn->cloneMove(m_fnBuffer);
					other.reset();
				}

				return *this;
			}

			Fn &operator=(const Fn &other) noexcept{
				if(&other != this){
					reinterpret_cast<FunctorBase*>(m_fnBuffer)->~FunctorBase();
					auto otherFn = reinterpret_cast<const FunctorBase*>(other.m_fnBuffer);
					otherFn->clone(m_fnBuffer);
				}

				return *this;
			}

			template<typename ... UArgs>
			Ret operator()(UArgs &&... args) const{
				const auto fn = reinterpret_cast<const FunctorBase*>(m_fnBuffer);
				return fn->call(std::forward<UArgs>(args)...);
			}

			void reset() noexcept{
				auto ptr = reinterpret_cast<FunctorBase*>(m_fnBuffer);
				ptr->~FunctorBase();
				construct([](Args...) -> Ret{ throw EmptyFnError{}; });
			}

		private:
			template<typename F>
			void construct(F &&f){
				new(m_fnBuffer) Functor<F>(std::forward<F>(f));
			}

			struct FunctorBase{
				virtual ~FunctorBase() = default;
				virtual Ret call(Args...) const = 0;
				virtual void clone(void *mem) const = 0;
				virtual void cloneMove(void *mem) = 0;
			};

			template<typename F>
			struct Functor: FunctorBase{
				static_assert(sizeof(F) <= sizeof(SmallBuffer), "GPWE_STATIC_BUFFER_SIZE too small");

				template<typename G>
				Functor(G &&g)
					: fn(std::forward<G>(g))
				{}

				Ret call(Args ... args) const override{
					if constexpr(std::is_same_v<void, Ret>){
						fn(args...);
					}
					else{
						return fn(args...);
					}
				}

				void clone(void *mem) const override{
					new(mem) Functor<F>(fn);
				}

				void cloneMove(void *mem) override{
					new(mem) Functor<F>(std::move(fn));
				}

				F fn;
			};

			SmallBuffer m_fnBuffer;
	};
}

#endif // !GPWE_FN_HPP
