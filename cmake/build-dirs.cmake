add_subdirectory (kadu-core)
add_subdirectory (varia)
add_subdirectory (translations)

# Qt6 port: the core was brought up first and plugins are being ported one at a
# time, so they are off by default. Enable with KADU_PORT_BUILD_PLUGINS and pick
# the subset through the existing COMPILE_PLUGINS variable, e.g.
#   cmake -DKADU_PORT_BUILD_PLUGINS=ON -DCOMPILE_PLUGINS=gadu_protocol
if (KADU_PORT_BUILD_PLUGINS OR KADU_PORT_BUILD_EVERYTHING)
	include (Plugins.cmake)
	add_subdirectory (plugins)
endif ()

if (KADU_PORT_BUILD_EVERYTHING)
	add_subdirectory (kthxbye)
	add_subdirectory (tests)
endif ()
