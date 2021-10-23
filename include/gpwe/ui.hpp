#ifndef GPWE_UI_HPP
#define GPWE_UI_HPP 1

#include "util/math.hpp"
#include "Manager.hpp"

namespace gpwe::ui{
	class WidgetBase;
	class SolidColor;
	class Layout;

	class Manager:
		public gpwe::Manager<Manager, ManagerKind::ui, Layout>
	{
		public:

		protected:
			UniquePtr<Layout> doCreateLayout(WidgetBase *parent);
			UniquePtr<SolidColor> doCreateSolidColor(WidgetBase *parent);

			friend class Layout;
			friend class SolidColor;
	};

	class WidgetBase: public virtual gpwe::ObjectBase{
		public:
			virtual ~WidgetBase() = default;
	};

	template<typename Derived, auto CreateFn, typename ... Props>
	class Widget:
		public Managed<
			Derived, CreateFn,
			Property<"layout"_cs, Layout*>,
			Props...
		>
	{
		public:
			using ManagedType = Managed<
				Derived, CreateFn,
				Property<"layout"_cs, Layout*>,
				Props...
			>;

			explicit Widget(WidgetBase *parent_ = nullptr)
				: ManagedType(static_cast<ObjectBase*>(parent_)){}

			virtual ~Widget() = default;
	};

	class SolidColor:
		public Widget<
			SolidColor, &Manager::doCreateSolidColor,
			Property<"color"_cs, Vec4>
		>
	{
		public:
			using WidgetType = Widget<
				SolidColor, &Manager::doCreateSolidColor,
				Property<"color"_cs, Vec4>
			>;

			using WidgetType::WidgetType;

	};

	class Layout:
		public Widget<Layout, &Manager::doCreateLayout>
	{
		public:
			using WidgetType = Widget<Layout, &Manager::doCreateLayout>;

			using WidgetType::WidgetType;

			bool addWidget(WidgetBase *widget){
				if(std::find(m_widgets.begin(), m_widgets.end(), widget) != m_widgets.end()){
					return false;
				}

				auto layout = widget->findProperty("layout")->as<Layout*>();
				*layout = this;

				m_widgets.emplace_back(widget);
			}

			bool removeWidget(WidgetBase *widget){
				auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
				if(it == m_widgets.end()) return false;

				m_widgets.erase(it);
				return true;
			}

		private:
			Vector<WidgetBase*> m_widgets;
	};
}

#endif // !GPWE_UI_HPP
