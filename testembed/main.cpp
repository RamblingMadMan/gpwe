#include "gpwe/sys.hpp"
#include "gpwe/input.hpp"
#include "gpwe/app.hpp"

#include <QApplication>

#include "MainWindow.hpp"

class TestApp: public gpwe::app::Manager{
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
	gpwe::sys::setAppManager(gpwe::makeUnique<TestApp>());
	gpwe::sys::setInputManager(gpwe::makeUnique<InputManager>());

	gpwe::sys::initSys(argc, argv);
	gpwe::sys::initInput();

	QApplication::setApplicationName("GPWE Embed Test");
	QApplication::setOrganizationName("Hamsmith");
	QApplication::setOrganizationDomain("https://hamsmith.dev");

	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	return app.exec();
}
