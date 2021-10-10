set(
	GPWE_TESTGAME_SOURCES
	TestApp.hpp
	TestApp.cpp
)

add_library(gpwe-app-test SHARED ${GPWE_TESTGAME_SOURCES})

set_target_properties(
	gpwe-app-test PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_link_libraries(gpwe-app-test gpwe-base)

add_custom_command(TARGET gpwe-app-test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gpwe-app-test> ${CMAKE_BINARY_DIR}/client/
)

add_custom_command(TARGET gpwe-app-test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/Assets ${CMAKE_BINARY_DIR}/client/Assets
)
