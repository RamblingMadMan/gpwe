set(
	GPWE_TESTGAME_SOURCES
	TestApp.hpp
	TestApp.cpp
)

add_gpwe_app(gpwe-app-test ${GPWE_TESTGAME_SOURCES})

function(gpwe_embed_app_test tgt)
	gpwe_embed_app(${tgt} gpwe-app-test)
endfunction()
