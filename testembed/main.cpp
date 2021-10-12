#include "gpwe/sys.hpp"
#include "gpwe/input.hpp"
#include "gpwe/App.hpp"

#include <QApplication>

#include "MainWindow.hpp"

class TestApp: public gpwe::App{
	public:
		TestApp(){}
		~TestApp(){}

		void update(float dt) override{}
};

GPWE_APP(TestApp, "Test App", "RamblingMad", 0, 0, 0)

class InputManager: public gpwe::input::Manager{
	public:
		InputManager(){
			createMouse();
			createKeyboard();
		}
};

int main(int argc, char *argv[]){
	InputManager input;

	gpwe::sys::setCreateAppFn(gpweCreateApp);
	gpwe::sys::initSys(argc, argv, &input);

	QApplication::setApplicationName("GPWE Embed Test");
	QApplication::setOrganizationName("Hamsmith");
	QApplication::setOrganizationDomain("https://hamsmith.dev");

	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	return app.exec();
}
