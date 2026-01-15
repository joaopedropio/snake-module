zephyr_include_directories(include)
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library()
target_sources(app PRIVATE src/behaviors/behavior_dongle_action.c)
target_sources(app PRIVATE src/events/dongle_action_event.c)
# target_sources(app PRIVATE src/behaviors/behavior_caps_word.c)
# target_sources(app PRIVATE src/events/caps_word_state_changed.c)

if(CONFIG_DT_HAS_ZMK_BEHAVIOR_CAPS_WORD_ENABLED)
        zephyr_library_sources(src/events/caps_word_state_changed.c)
        set_source_files_properties(
                ${APPLICATION_SOURCE_DIR}/src/behaviors/behavior_caps_word.c
                TARGET_DIRECTORY app
                PROPERTIES HEADER_FILE_ONLY ON)
        target_sources(app PRIVATE src/behaviors/behavior_caps_word.c)
endif()