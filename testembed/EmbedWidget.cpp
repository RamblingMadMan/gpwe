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
	gpwe::sys::setRendererArg(reinterpret_cast<void*>(getGLProc));

	gpwe::sys::initRenderer(width(), height());
	gpwe::sys::initApp();

	m_ticker.reset();
}

void EmbedWidget::paintGL(){
	gpwe::sys::tick(m_ticker.tick());
}
