add_subdirectory (kadu-core)
add_subdirectory (varia)
add_subdirectory (translations)

# Qt6 port, stage 1: build the core only. Plugins, kthxbye and tests come back
# once kadu-core compiles and runs.
if (KADU_PORT_BUILD_EVERYTHING)
	include (Plugins.cmake)
	add_subdirectory (plugins)
	add_subdirectory (kthxbye)
	add_subdirectory (tests)
endif ()
