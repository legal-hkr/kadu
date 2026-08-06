project (idle)

cmake_minimum_required (VERSION 3.16)

find_package (Kadu REQUIRED CONFIG)

kadu_api_directories (plugins/idle
	.
)

set (SOURCES
	idle-module.cpp
	idle-plugin-modules-factory.cpp
	idle-plugin-object.cpp
	idle.cpp
)

if (UNIX AND NOT APPLE)
	# KIdleTime speaks the compositor's idle protocol. Without it there is no way for a Wayland
	# client to learn how long the user has been away, so the plugin still builds and answers
	# that the idle time is unknown rather than making one up.
	find_package (KF6IdleTime QUIET)
	if (KF6IdleTime_FOUND)
		message (STATUS "Found KF6IdleTime: idle detection enabled")
		set (LIBRARIES KF6::IdleTime)
		list (APPEND SOURCES
			idle-kidletime.cpp
		)
	else ()
		message (STATUS "KF6IdleTime not found: idle time will be reported as unknown")
		list (APPEND SOURCES
			idle-unknown.cpp
		)
	endif ()
endif ()

if (WIN32)
	list (APPEND SOURCES
		idle-win.cpp
	)
endif ()

add_definitions (-Didle_EXPORTS)

kadu_plugin (idle
	PLUGIN_SOURCES ${SOURCES}
	PLUGIN_LIBRARIES ${LIBRARIES}
)
