#include "gpwe/ui.hpp"

using namespace gpwe;
using namespace gpwe::ui;

UniquePtr<Layout> ui::Manager::doCreateLayout(WidgetBase *parent){
	return makeUnique<Layout>(parent);
}

UniquePtr<SolidColor> ui::Manager::doCreateSolidColor(WidgetBase *parent){
	return makeUnique<SolidColor>(parent);
}
