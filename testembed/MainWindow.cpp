#include "MainWindow.hpp"
#include "EmbedWidget.hpp"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle("GPWE Embed Test");
	setContentsMargins(0, 0, 0, 0);
	setCentralWidget(new EmbedWidget(this));
}
