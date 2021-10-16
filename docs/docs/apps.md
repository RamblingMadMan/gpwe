# Apps

Apps are simply plugins that contain application logic.

## Creating an App

Apps are created using the cmake function `add_gpwe_app`.

Example `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.14)

project(my-app VERSION 0.0.0 LANGUAGES CXX)

add_subdirectory(gpwe)

set(
	MY_APP_SOURCES
	MyApp.hpp
	MyApp.cpp
)

add_gpwe_app(my-app ${MY_APP_SOURCES})
```
