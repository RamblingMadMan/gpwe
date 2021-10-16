find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(
	GPWE_TESTEMBED_SOURCES
	EmbedWidget.hpp
	EmbedWidget.cpp
	ContainerWidget.hpp
	ContainerWidget.cpp
	MainWindow.hpp
	MainWindow.cpp
	main.cpp
)

add_executable(gpwe-test-embed ${GPWE_TESTEMBED_SOURCES})

set_target_properties(
	gpwe-test-embed PROPERTIES
	CXX_STANDARD 17
	STANDARD_REQUIRED ON
	AUTOMOC ON
)

target_link_libraries(
	gpwe-test-embed
	PRIVATE
	GPWE::Base
	Qt::Core Qt::Widgets
)

gpwe_embed_plugins(gpwe-test-embed renderer-gl43)
