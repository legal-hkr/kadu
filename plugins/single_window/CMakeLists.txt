project (single_window)

cmake_minimum_required (VERSION 3.16)

find_package (Kadu REQUIRED CONFIG)

set (SOURCES
	single-window-chat-widget-container-handler.cpp
	single-window-module.cpp
	single-window-plugin-modules-factory.cpp
	single-window-plugin-object.cpp
	single-window.cpp
)

set (CONFIGURATION_FILES
	configuration/single_window.ui
)

kadu_plugin (single_window
	PLUGIN_SOURCES ${SOURCES}
	PLUGIN_CONFIGURATION_FILES ${CONFIGURATION_FILES}
)
