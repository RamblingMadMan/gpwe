#include "gpwe/sys.hpp"
#include "gpwe/input.hpp"
#include "gpwe/app.hpp"

#include <QApplication>

#include "MainWindow.hpp"

class InputManager: public gpwe::input::Manager{
	public:
		using Mouse = gpwe::input::Mouse;
		using Keyboard = gpwe::input::Keyboard;

		InputManager(){
			create<Mouse>(0);
			create<Keyboard>(0);
		}
};

int main(int argc, char *argv[]){
	auto manager = gpwe::makeUnique<gpwe::sys::Manager>();

	manager->setArgs(argc, argv);
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
