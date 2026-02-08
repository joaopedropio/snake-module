/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zmk/display.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk_dongle_events/dongle_action_event.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/display.h>

#include "action_button.h"
#include "snake.h"
#include "output_status.h"
#include "battery_status.h"
#include "layer_status.h"
#include "helpers/display.h"
#include "helpers/buzzer.h"
#include "helpers/settings.h"
#include "theme.h"
#include "wpm.h"
#include "logo.h"
#include <stdint.h>

static uint8_t *buf_frame;
static uint16_t menu_threshold = 0;
static uint16_t theme_threshold = 300;
static uint16_t mute_threshold = 600;
static int64_t pressed_timestamp = 0;
static int64_t released_timestamp = 0;

static bool action_button_initialized = false;
static struct layer_status_state ls_state;

static bool menu_on = false;
static bool dongle_lock = false;

static const uint8_t base_layer = 0;
static const uint8_t menu_layer = 4;
static const uint8_t theme_layer = 5;
static const uint8_t mute_layer = 6;

struct layer_status_state {
    uint8_t index;
    const char *label;
};

void set_theme_threshold(uint16_t term_ms) {
    theme_threshold = term_ms;
}

void set_mute_threshold(uint16_t term_ms) {
    mute_threshold = term_ms;
}

void print_frames() {
    uint16_t thickness = 1;

    SlotMode slot_mode = get_slot_mode();

    if (slot_mode == SLOT_MODE_2) {
        print_container(buf_frame, 0, 240, 0, 113, thickness); // snake slot
        
        print_container(buf_frame, 0, 120, 112, 161, thickness);   // slot 5
        print_container(buf_frame, 120, 240, 112, 161, thickness); // slot 6
    }

    if (slot_mode == SLOT_MODE_4) {
        print_container(buf_frame, 0, 240, 0, 73, thickness); // snake slot

        print_container(buf_frame, 0, 120, 72, 117, thickness);   // slot 3
        print_container(buf_frame, 120, 240, 72, 117, thickness); // slot 4
        
        print_container(buf_frame, 0, 120, 116, 161, thickness);   // slot 5
        print_container(buf_frame, 120, 240, 116, 161, thickness); // slot 6
    }

    if (slot_mode == SLOT_MODE_6) {
        print_container(buf_frame, 0, 120, 0, 55, thickness);     // slot 1
        print_container(buf_frame, 120, 240, 0, 55, thickness);   // slot 2
        print_container(buf_frame, 0, 120, 54, 108, thickness);   // slot 3
        print_container(buf_frame, 120, 240, 54, 108, thickness); // slot 4
        print_container(buf_frame, 0, 120, 107, 161, thickness);  // slot 5
        print_container(buf_frame, 120, 240, 107, 161, thickness);// slot 6
    }

    // battery frames 
    #ifdef CONFIG_SHOW_SINGLE_BATTERY
    print_container(buf_frame, 0, 240, 160, 240, thickness);
    #else
    print_container(buf_frame, 0, 120, 160, 240, thickness);
    print_container(buf_frame, 120, 240, 160, 240, thickness);
    #endif
}

void print_menu() {
    clear_screen();
    start_animation();
    print_frames();
    start_battery_status();
    start_output_status();
    start_wpm_status();
    start_modifier_status();
    start_layer_status();
    set_status_symbol();
    set_battery_symbol();
    print_layer();
    print_themes();
    print_wpm();
    print_modifiers();
}

void toggle_menu() {
    #ifdef CONFIG_USE_BUZZER
        #ifdef CONFIG_USE_MENU_SOUND
        play_notification_song();
        #endif
    #endif
    if (menu_on) {
        stop_wpm_status();
        stop_modifier_status();
        stop_output_status();
        stop_battery_status();
        stop_animation();
        stop_layer_status();
        start_snake();
        menu_on = false;
    } else {
        stop_snake();
        print_menu();
        menu_on = true;
    }
}

void change_theme() {
    set_next_theme();
    #ifdef CONFIG_USE_BUZZER
        #ifdef CONFIG_USE_THEME_SOUND
        play_startup_song();
        #endif
    #endif
    if (menu_on) {
        print_menu();
        apply_theme_snake();
    } else {
        stop_snake();
        apply_theme_snake();
        start_snake();
    }
}

void set_layer_symbol() {
    if (dongle_lock) {
        return;
    }
    dongle_lock = true;
    if (ls_state.index == menu_layer) {
        toggle_menu();
    }
    if (ls_state.index == theme_layer) {
        change_theme();
    }
    if (ls_state.index == mute_layer) {
        #ifdef CONFIG_USE_BUZZER
        snake_settings_toggle_mute();
        if (!snake_settings_get_mute()) {
            play_once(coin);
        }
        #endif
    }
    dongle_lock = false;
}

void dongle_action_update_cb(struct zmk_dongle_actioned state) {
    if (state.timestamp == 0) {
        return;
    }
    if (state.pressed) {
        pressed_timestamp = state.timestamp;
        return;
    }
    if (!state.pressed) {
        uint8_t index;
        int64_t elapsed_time = state.timestamp - pressed_timestamp;
        if (elapsed_time > menu_threshold) {
            index = menu_layer;
        }
        if (elapsed_time > theme_threshold) {
            index = theme_layer;
        }
        if (elapsed_time > mute_threshold) {
            index = mute_layer;
        }
        ls_state = (struct layer_status_state) {
            .index = index
        };
        if (action_button_initialized) {
            set_layer_symbol();
        }
        pressed_timestamp = 0;
    }
}

static struct zmk_dongle_actioned dongle_action_get_state(const zmk_event_t *eh) {
    const struct zmk_dongle_actioned *ev = as_zmk_dongle_actioned(eh);

    return (struct zmk_dongle_actioned){
        .pressed = (ev != NULL) ? ev->pressed : false,
        .timestamp = (ev != NULL) ? ev->timestamp : 0,
    };
}


ZMK_DISPLAY_WIDGET_LISTENER(dongle_action, struct zmk_dongle_actioned, dongle_action_update_cb, dongle_action_get_state)
ZMK_SUBSCRIPTION(dongle_action, zmk_dongle_actioned);


void zmk_widget_action_button_init() {
    dongle_action_init();

    buf_frame = (uint8_t*)k_malloc(320 * 2 * sizeof(uint8_t));
}

void start_action_button(bool is_menu_on) {
    menu_on = is_menu_on;
    action_button_initialized = true;
}