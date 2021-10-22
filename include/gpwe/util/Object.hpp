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

	class PropertyBase;

	template<auto Name, typename T>
	class Property;

	class ObjectBase{
		public:
			virtual ~ObjectBase() = default;

			virtual StrView objName() const noexcept = 0;

			Nat64 numProperties() const noexcept{
				return m_props.size();
			}

			auto propertiesBegin() noexcept{ return m_propMap.begin(); }
			auto propertiesEnd() noexcept{ return m_propMap.end(); }

			auto propertiesBegin() const noexcept{ return m_propMap.begin(); }
			auto propertiesEnd() const noexcept{ return m_propMap.end(); }

			template<auto Name, typename T>
			PropertyBase *createProperty(){
				const auto name = std::string(Name.str, Name.length);

				auto res = m_propMap.find(name);
				if(res != m_propMap.end()) return nullptr;

				auto ptr = makeUnique<Property<Name, T>>();
				auto ret = ptr.get();

				auto emplaceRes = m_propMap.try_emplace(name, ret);
				if(!emplaceRes.second) return nullptr;

				auto emplaceIt = std::upper_bound(m_props.begin(), m_props.end(), ptr);
				m_props.emplace(emplaceIt, std::move(ptr));

				return ret;
			}

			PropertyBase *property(const Str &name){
				auto it = m_propMap.find(std::string(name.c_str(), name.length()));
				return it == m_propMap.end() ? nullptr : it->second;
			}

		private:
			List<UniquePtr<PropertyBase>> m_props;
			HashMap<std::string, PropertyBase*> m_propMap;
	};

	template<auto ObjName, typename ... Props>
	class Object: public virtual ObjectBase{
		public:
			static constexpr auto Name = ObjName;

			Object(){
				(createProperty<Props::Name, Props::Type>, ...);
			}

			virtual ~Object() = default;

			StrView objName() const noexcept override{
				return StrView(ObjName.str, ObjName.length);
			}
	};

	template<auto ObjName>
	class Object<ObjName>: public virtual ObjectBase{
		public:
			Object(){}

			virtual ~Object() = default;

			StrView objName() const noexcept override{
				return StrView(ObjName.str, ObjName.length);
			}
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

	template<meta::CStr Name, typename T>
	class Property<Name, T>: public Object<Name>, public PropertyBase{
		public:
			using Type = T;

			template<typename ... Args>
			explicit Property(Args &&... args)
				: PropertyBase(meta::type<T>), m_val(std::forward<Args>(args)...){}

			StrView name() const noexcept override{ return StrView(Name.str, Name.length); }

			template<typename U>
			void setValue(U &&other){
				m_val = std::forward<U>(other);
			}

			const T &value() const noexcept{ return m_val; }

		protected:
			void *ptr() noexcept override{ return &m_val; }
			const void *ptr() const noexcept override{ return &m_val; }

		private:
			Str m_name;
			T m_val;
	};

	/**
	 * @}
	 */

	/**
	 * @}
	 */
}

#endif // !GPWE_OBJECT_HPP
