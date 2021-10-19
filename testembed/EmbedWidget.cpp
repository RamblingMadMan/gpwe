#include "gpwe/sys.hpp"

#include <QOpenGLContext>

#include "EmbedWidget.hpp"

EmbedWidget::EmbedWidget(QWidget *parent): QOpenGLWidget(parent){

}

QOpenGLContext *gpweCtx = nullptr;

using GLProc = void(*)();

GLProc getGLProc(const char *name){ return gpweCtx->getProcAddress(name); }

void EmbedWidget::initializeGL(){
	gpweCtx = context();

	auto manager = gpwe::sys::manager();

	manager->setRenderArg((void*)getGLProc);
	manager->setRenderSize(width(), height());
	manager->init();

	m_ticker.reset();
}

void EmbedWidget::paintGL(){
	gpwe::sys::update(m_ticker.tick());
}
