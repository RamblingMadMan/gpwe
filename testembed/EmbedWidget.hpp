#ifndef EMBEDWIDGET_HPP
#define EMBEDWIDGET_HPP

#include "gpwe/Ticker.hpp"

#include <QOpenGLWidget>

class EmbedWidget: public QOpenGLWidget{
	Q_OBJECT

	public:
		explicit EmbedWidget(QWidget *parent = nullptr);

	protected:
		void initializeGL() override;
		void paintGL() override;

	private:
		gpwe::Ticker<> m_ticker;
};

#endif // EMBEDWIDGET_HPP
