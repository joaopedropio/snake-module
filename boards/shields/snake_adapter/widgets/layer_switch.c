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

#include "layer_switch.h"
#include "snake.h"
#include "output_status.h"
#include "battery_status.h"
#include "helpers/display.h"
#include "helpers/buzzer.h"
#include "helpers/settings.h"
#include "theme.h"
//#include "snake_image.h"
#include "logo.h"
#include <stdint.h>

static uint8_t *buf_frame;
static uint16_t menu_threshold = 0;
static uint16_t theme_threshold = 300;
static uint16_t mute_threshold = 600;
static int64_t pressed_timestamp = 0;
static int64_t released_timestamp = 0;

static bool layer_switch_initialized = false;
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

void print_container(uint8_t *buf_frame, uint16_t start_x, uint16_t end_x, uint16_t start_y, uint16_t end_y, uint16_t scale) {
    print_rectangle(buf_frame, start_x, end_x - scale, start_y, end_y - scale, get_frame_color(), scale);
    print_rectangle(buf_frame, start_x + scale, end_x - (scale * 2), start_y + scale, end_y - (scale * 2), get_frame_color_1(), scale);
}

void print_frames() {
    uint16_t thickness = 1;
    // logo frame
    print_container(buf_frame, 0, 240, 0, 113, thickness);

    // status frames
    print_container(buf_frame, 0, 120, 112, 161, thickness);

    // theme frames
    print_container(buf_frame, 120, 240, 112, 161, thickness);

    // battery frames 
    print_container(buf_frame, 0, 120, 160, 240, thickness);
    print_container(buf_frame, 120, 240, 160, 240, thickness);
}

void print_menu() {
    clear_screen();
    start_animation();
    print_frames();
    start_battery_status();
    start_output_status();
    set_status_symbol();
    set_battery_symbol();
    print_themes();
}

void toggle_menu() {
    #ifdef CONFIG_USE_BUZZER
        #ifdef CONFIG_USE_MENU_SOUND
        play_notification_song();
        #endif
    #endif
    if (menu_on) {
        stop_output_status();
        stop_battery_status();
        stop_animation();
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

// uint16_t compare_integers(const void *a, const void *b) {
//     return (*(uint16_t *)a - *(uint16_t *)b);
// }

// uint16_t find_closest_below_elapsed(const uint16_t *array, size_t size, int64_t elapsed_time) {
//     qsort(array, size, sizeof(uint16_t), compare_integers);

//     for (uint8_t i = 0; i < size; i++) {
//         if (elapsed_time < array[i]) {
//             if (i == 0) {
//                 return array[0];
//             }
//             return array[i - 1];
//         }
//     }
//     return array[size-1];
// }


// uint8_t get_action(int64_t elapsed_time) {
//     uint16_t thresholds[] = {menu_threshold, theme_threshold, mute_threshold};
//     uint16_t threshold = find_closest_below_elapsed(thresholds, 3, elapsed_time);
//     if (threshold == menu_threshold) {
//         return menu_layer;
//     }
//     if (threshold == theme_threshold) {
//         return theme_layer;
//     }
//     if (threshold == mute_threshold) {
//         return mute_layer;
//     }
//     return menu_layer;
// }

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
        if (layer_switch_initialized) {
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


void zmk_widget_layer_switch_init() {
    dongle_action_init();

    buf_frame = (uint8_t*)k_malloc(320 * 2 * sizeof(uint8_t));
}

void start_layer_switch(bool is_menu_on) {
    menu_on = is_menu_on;
    layer_switch_initialized = true;
}