if(CONFIG_DHARA)

zephyr_interface_library_named(DHARA)

zephyr_library_sources(
	dhara/error.c
	dhara/journal.c
	dhara/map.c
	dhara_interface.c
)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

zephyr_library_link_libraries(DHARA)
target_link_libraries(DHARA INTERFACE zephyr_interface)

endif()
