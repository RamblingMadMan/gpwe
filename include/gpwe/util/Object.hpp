#ifndef GPWE_OBJECT_HPP
#define GPWE_OBJECT_HPP 1

#include <typeindex>

#include "algo.hpp"
#include "Variant.hpp"
#include "Str.hpp"
#include "Map.hpp"
#include "List.hpp"

namespace gpwe{
	/**
	 * @defgroup Objects
	 * @{
	 */

	class Public;
	class Protected;

	class PropertyBase;
	class ObjectClassBase;
	class ObjectBase;

	template<auto Name, typename T, typename Access = Public>
	class Property;

	class ObjectInitializerBase{
		public:
			virtual ~ObjectInitializerBase() = default;
			virtual Nat64 numArgs() const noexcept = 0;
	};

	template<typename ... Args>
	class ObjectInitializer: public ObjectInitializerBase{
		public:
			template<typename ... UArgs>
			ObjectInitializer(UArgs &&... args)
				: m_args(std::make_tuple(std::forward<UArgs>(args)...)){}

			Nat64 numArgs() const noexcept override{ return sizeof...(Args); }

		private:
			std::tuple<Args...> m_args;
	};

	class ObjectClassBase{
		public:
			virtual UniquePtr<ObjectBase> instantiate(ObjectInitializerBase&&) const{
				return nullptr;
			}
	};

	template<typename Obj>
	class ObjectClass: public ObjectClassBase{
		public:
			constexpr ObjectClass() = default;
	};

	namespace detail{
		template<typename ObjT, typename ManagerT, typename = void>
		class SubObjectHelper;

		template<typename ObjT, typename ManagerT>
		class SubObjectHelper<
				ObjT, ManagerT,
				std::enable_if_t<ManagerT::template hasManager<ObjT>()>
			>
		{
			public:

		};
	}

	class ObjectBase{
		public:
			explicit ObjectBase(ObjectBase *parent_ = nullptr): m_parent(parent_){}
			virtual ~ObjectBase() = default;

			ObjectBase *objParent() const noexcept{ return m_parent; }

			virtual StrView objName() const noexcept = 0;
			virtual const ObjectClassBase *objClass() const noexcept = 0;

			Nat64 numChildObjects() noexcept{ return m_children.size(); }

			Nat64 numProperties() const noexcept{ return m_props.size(); }

			auto childrenBegin() noexcept{ return m_children.begin(); }
			auto childrenEnd() noexcept{ return m_children.end(); }

			auto childrenBegin() const noexcept{ return m_children.begin(); }
			auto childrenEnd() const noexcept{ return m_children.end(); }

			auto propertiesBegin() noexcept{ return m_propMap.begin(); }
			auto propertiesEnd() noexcept{ return m_propMap.end(); }

			auto propertiesBegin() const noexcept{ return m_propMap.begin(); }
			auto propertiesEnd() const noexcept{ return m_propMap.end(); }

			template<auto Name, typename T, typename Access = Public, typename ... Args>
			PropertyBase *createProperty(Args &&... args){
				const auto name = std::string(Name.str, Name.length);

				auto res = m_propMap.find(name);
				if(res != m_propMap.end()) return nullptr;

				auto ptr = makeUnique<Property<Name, T, Access>>(std::forward<Args>(args)...);
				auto ret = ptr.get();

				auto emplaceRes = m_propMap.try_emplace(name, ret);
				if(!emplaceRes.second) return nullptr;

				auto emplaceIt = std::upper_bound(m_props.begin(), m_props.end(), ptr);
				m_props.emplace(emplaceIt, std::move(ptr));

				return static_cast<PropertyBase*>(ret);
			}

			PropertyBase *findProperty(const Str &name){
				auto it = m_propMap.find(std::string(name.c_str(), name.length()));
				return it == m_propMap.end() ? nullptr : it->second;
			}

			// wouldn't 'const(auto)' or vice versa be nice
			const PropertyBase *findProperty(const Str &name) const{
				auto it = m_propMap.find(std::string(name.c_str(), name.length()));
				return it == m_propMap.end() ? nullptr : it->second;
			}

			SysManager *sysManager() const noexcept;
			InputManager *inputManager() const noexcept;
			RenderManager *renderManager() const noexcept;
			PhysicsManager *physicsManager() const noexcept;
			WorldManager *worldManager() const noexcept;
			UiManager *uiManager() const noexcept;
			AppManager *appManager() const noexcept;

			template<typename Obj, typename ... Args>
			Obj *createChildObject(Args &&... args){
				auto manager = meta::ObjectManagerT<Obj>::get();

				auto ptr = manager->template create<Obj>(std::forward<Args>(args)...);
				if(!ptr) return nullptr;

				auto emplaceIt = std::upper_bound(m_children.begin(), m_children.end(), ptr);
				return *m_children.emplace(emplaceIt, ptr);
			}

		protected:
			HashMap<std::string, PropertyBase*> m_propMap;

		private:
			ObjectBase *m_parent;
			Vector<ObjectBase*> m_children;
			List<UniquePtr<PropertyBase>> m_props;
	};

	namespace detail{
		template<meta::CStr Name, typename T>
		class ObjectPropertyHelper{
			protected:
				T &getProperty(){}
		};
	}

	template<typename Derived, typename ... Props>
	class Object: public virtual ObjectBase{
		public:
			using ObjType = Object<Derived, Props...>;
			static constexpr auto ObjName = meta::TypeName<Derived>::Name;

			explicit Object(ObjectBase *parent_ = nullptr)
				: ObjectBase(parent_)
			{
				setPropMap(std::make_index_sequence<sizeof...(Props)>{});
			}

			virtual ~Object() = default;

			StrView objName() const noexcept override{
				return strView<ObjName>();
			}

			const ObjectClass<Derived> *objClass() const noexcept override{
				return &m_class;
			}

			template<meta::CStr Name>
			decltype(auto) property(){
				return getProperty<Name>(std::make_index_sequence<sizeof...(Props)>{});
			}

		private:
			template<Nat64 Idx, Nat64 ... Indices>
			void setPropMap(std::index_sequence<Idx, Indices...>){
				auto &prop = std::get<Idx>(m_objProps);
				auto propName = prop.name();
				m_propMap[std::string(propName.begin(), propName.end())] = &prop;

				if constexpr(sizeof...(Indices)){
					setPropMap(std::index_sequence<Indices...>{});
				}
			}

			template<meta::CStr Name, Nat64 Idx, Nat64 ... Indices>
			decltype(auto) getProperty(std::index_sequence<Idx, Indices...>){
				using Prop = meta::At<Idx, meta::Types<Props...>>;
				if constexpr(Name == Prop::Name){
					return std::get<Idx>(m_objProps).value();
				}
				else if constexpr(sizeof...(Indices)){
					return getProperty<Name>(std::index_sequence<Indices...>{});
				}
				else{
					throw std::runtime_error("No property with given name");
				}
			}

			std::tuple<Props...> m_objProps;
			static constexpr auto m_class = ObjectClass<Derived>{};
	};

	template<typename Derived>
	class Object<Derived>: public virtual ObjectBase{
		public:
			static constexpr auto ObjName = meta::TypeName<Derived>::Name;

			explicit Object(ObjectBase *parent_ = nullptr)
				: ObjectBase(parent_){}

			virtual ~Object() = default;

			StrView objName() const noexcept override{
				return strView<ObjName>();
			}

			const ObjectClass<Derived> *objClass() const noexcept override{
				return &m_class;
			}

		private:
			static constexpr auto m_class = ObjectClass<Derived>{};
	};

	/**
	 * @defgroup Properties
	 * @{
	 */

	class PropertyBase{
		public:
			template<typename T>
			PropertyBase(meta::Type<T>): m_rtti(typeid(T)){}

			virtual ~PropertyBase() = default;

			virtual StrView name() const noexcept = 0;

			template<typename T>
			T *as() noexcept{
				if(std::type_index(typeid(T)) == m_rtti){
					return reinterpret_cast<T*>(ptr());
				}
				else{
					return nullptr;
				}
			}

			template<typename T>
			const T *as() const noexcept{
				if(std::type_index(typeid(T)) == m_rtti){
					return reinterpret_cast<const T*>(ptr());
				}
				else{
					return nullptr;
				}
			}

		protected:
			virtual void *ptr() noexcept = 0;
			virtual const void *ptr() const noexcept = 0;

		private:
			std::type_index m_rtti;
	};

	namespace detail{
		template<typename T, typename Access>
		class PropertyHelper;

		template<typename T>
		class PropertyHelper<T, Protected>: public PropertyBase{
			public:
				template<typename ... Args>
				PropertyHelper(Args &&... args)
					: PropertyBase(meta::type<T>), m_val(std::forward<Args>(args)...){}

				const T &value() const noexcept{ return m_val; }

			protected:
				template<typename U>
				void setValueImpl(U &&other){
					m_val = std::forward<U>(other);
				}

				T &valueImpl() noexcept{ return m_val; }

				virtual void *ptr() noexcept override{ return nullptr; }
				const void *ptr() const noexcept override{ return &m_val; }

			private:
				T m_val;
		};

		template<typename T>
		class PropertyHelper<T, Public>: public PropertyHelper<T, Protected>{
			public:
				template<typename ... Args>
				PropertyHelper(Args &&... args)
					: PropertyHelper<T, Protected>(std::forward<Args>(args)...){}

				template<typename U>
				void setValue(U &&other){
					setValueImpl(std::forward<U>(other));
				}

				T &value() noexcept{ return PropertyHelper<T, Protected>::valueImpl(); }

			protected:
				void *ptr() noexcept override{ return &value(); }
		};
	}

	template<meta::CStr NameStr, typename T, typename Access>
	class Property<NameStr, T, Access>:
		public detail::PropertyHelper<T, Access>
	{
		public:
			static_assert(
				meta::isIn(meta::type<Access>, meta::types<Public, Protected>),
				"Access must be gpwe::Public or gpwe::Protected"
			);

			static constexpr auto Name = NameStr;

			using Type = T;

			template<typename ... Args>
			explicit Property(Args &&... args)
				: detail::PropertyHelper<T, Access>(std::forward<Args>(args)...){}

			StrView name() const noexcept override{ return strView<Name>(); }
	};

	/**
	 * @}
	 */

	/**
	 * @}
	 */
}

#endif // !GPWE_OBJECT_HPP
