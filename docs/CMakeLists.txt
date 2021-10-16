set(
	GPWE_DOCS_SOURCES
	mkdocs.yml
	docs/index.md
	docs/getting-started.md
	docs/apps.md
	docs/plugins.md
	docs/style/main.css
)

add_custom_target(
	gpwe-docs

	COMMAND
		mkdocs build -d ${CMAKE_CURRENT_BINARY_DIR}

	WORKING_DIRECTORY
		${CMAKE_CURRENT_LIST_DIR}

	SOURCES
		${GPWE_DOCS_SOURCES}

	VERBATIM
)
