# Tests
find_package (Qt6 6.2 REQUIRED COMPONENTS Test)

function (kadu_add_plugin_test plugin_name name_)
	set (test_file "${name_}.test")
	string (REPLACE "/" "-" test_name ${test_file})
	list (APPEND UNIT_TEST_TARGETS ${test_name} PARENT_SCOPE)

	add_executable (${test_name} "${test_file}.cpp")
	kadu_set_flags (${test_name})
	if (NOT MSVC)
		set_property (TARGET ${test_name} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-error")
	endif ()
	add_test (
		NAME ${test_name}
		COMMAND ${test_name})

	target_link_libraries (${test_name} LINK_PRIVATE Qt6::Core Qt6::Core5Compat Qt6::StateMachine Qt6::Gui Qt6::Network Qt6::Qml Qt6::Quick Qt6::QuickWidgets Qt6::Test Qt6::Widgets Qt6::Xml)

	# Add libkadu after the plugin so that --as-needed won't drop anything
	target_link_libraries (${test_name} LINK_PRIVATE libkadu ${plugin_name} ${ADDITIONAL_LIBRARIES})
endfunction ()

function (kadu_add_test name_)
	kadu_add_plugin_test ("" "${name_}")
endfunction ()

enable_testing ()
