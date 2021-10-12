#include <QVBoxLayout>

#include "ContainerWidget.hpp"
#include "EmbedWidget.hpp"

ContainerWidget::ContainerWidget(QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	lay->addWidget(new EmbedWidget(this));

	setLayout(lay);
}
