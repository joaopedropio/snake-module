zephyr_include_directories(include)
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library()

if(CONFIG_SHIELD_SNAKE_ADAPTER)

        add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/drivers/display)

endif()

target_sources(app PRIVATE src/behaviors/behavior_dongle_action.c)
target_sources(app PRIVATE src/events/dongle_action_event.c)