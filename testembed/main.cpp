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

class InputManager: public gpwe::input::Manager{
	public:
		InputManager(){
			createMouse();
			createKeyboard();
		}
};

int main(int argc, char *argv[]){
	auto manager = gpwe::makeUnique<gpwe::sys::Manager>();

	manager->setArgs(argc, argv);
	manager->setAppManager(gpwe::makeUnique<TestApp>());
	manager->setInputManager(gpwe::makeUnique<InputManager>());

	gpwe::sys::setManager(manager.get());

	QApplication::setApplicationName("GPWE Embed Test");
	QApplication::setOrganizationName("Hamsmith");
	QApplication::setOrganizationDomain("https://hamsmith.dev");

	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	return app.exec();
}
