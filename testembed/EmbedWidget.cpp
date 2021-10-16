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

	auto manager = gpwe::sys::sysManager();

	manager->setRenderArg((void*)getGLProc);
	manager->setRenderSize(width(), height());

	gpwe::sys::sysManager()->init();

	m_ticker.reset();
}

void EmbedWidget::paintGL(){
	gpwe::sys::tick(m_ticker.tick());
}
