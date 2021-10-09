set(
	GPWE_TESTGAME_SOURCES
	TestApp.hpp
	TestApp.cpp
)

add_library(gpwe-game-test SHARED ${GPWE_TESTGAME_SOURCES})

set_target_properties(
	gpwe-game-test PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_link_libraries(gpwe-game-test gpwe-base)

add_custom_command(TARGET gpwe-game-test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gpwe-game-test> ${CMAKE_BINARY_DIR}/client/
)
