#include "gpwe/sys.hpp"
#include "gpwe/input.hpp"

#include <QApplication>

#include "MainWindow.hpp"

class InputManager: public gpwe::input::Manager{
	public:
		InputManager(){
			createMouse();
			createKeyboard();
		}
};

int main(int argc, char *argv[]){
	InputManager input;
	gpwe::sys::initSys(argc, argv, &input);

	QApplication::setApplicationName("GPWE Embed Test");
	QApplication::setOrganizationName("Hamsmith");
	QApplication::setOrganizationDomain("https://hamsmith.dev");

	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	return app.exec();
}
