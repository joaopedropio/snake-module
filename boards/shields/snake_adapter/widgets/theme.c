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
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zmk/display.h>
#include "helpers/display.h"
#include "helpers/settings.h"

#define COLORS_PER_THEME 6

static uint8_t themes_colors_len = 11;
static uint32_t themes_colors[][COLORS_PER_THEME] = {
    // back      primary    secondary
    {0x405010u, 0xd0d058u, 0x708028u, 0xa0a840u, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/nostalgia
    {0x222323u, 0xff4adcu, 0x3dff98u, 0xf0f6f0u, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/b4sement
    {0x302387u, 0xff3796u, 0x00faacu, 0xfffdafu, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/fuzzyfour
    {0x332c50u, 0x46878fu, 0x94e344u, 0xe2f3e4u, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/kirokaze-gameboy
    {0x0f052du, 0x203671u, 0x36868fu, 0x5fc75du, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/moonlight-gb
    {0xfcdeeau, 0xff4d6du, 0x265935u, 0x012824u, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/cherrymelon
    {0x282328u, 0x545c7eu, 0xc56981u, 0xa3a29au, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/bittersweet
    {0xfce4a8u, 0x71969fu, 0xd71a21u, 0x01334eu, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/americana
    {0x051f39u, 0xc53a9du, 0x4a2480u, 0xff8e80u, 0x000000u, 0xddddddu}, // https://lospec.com/palette-list/lava-gb
    {0x2b2132u, 0xfb6634u, 0x7e0d0du, 0xfeb746u, 0x000000u, 0xddddddu}, // https://lospec.com/gallery/2bitdream/of-the-earth
    {0x323859u, 0xecfffbu, 0x576373u, 0x0c0421u, 0x000000u, 0xddddddu}, // https://lospec.com/gallery/dogmaster/cave
};

static uint8_t current_theme = 0;

static uint16_t theme_font_scale = 4;
static uint16_t theme_font_width = 3;
static uint16_t theme_font_height = 6;
static uint16_t *scaled_bitmap_theme_font;

static uint16_t theme_x = 136;
static uint16_t theme_y = 126;

static Character int_to_num_char(uint8_t i) {
    switch (i) {
        case 0: return CHAR_0;
        case 1: return CHAR_1;
        case 2: return CHAR_2;
        case 3: return CHAR_3;
        case 4: return CHAR_4;
        case 5: return CHAR_5;
        case 6: return CHAR_6;
        case 7: return CHAR_7;
        case 8: return CHAR_8;
        case 9: return CHAR_9;
    }
    return CHAR_NONE;
}

void print_themes() {
    Character theme_template[] = {
        CHAR_S,
        CHAR_K,
        CHAR_I,
        CHAR_N,
    };

    uint8_t num = current_theme + 1;
    uint16_t first_num = current_theme / 10;
    uint16_t second_num = current_theme % 10;

    uint16_t char_gap_pixels = 2;
    print_string(scaled_bitmap_theme_font, theme_template, theme_x, theme_y, theme_font_scale, get_symbol_unselected_color(), get_symbol_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 4);
    print_bitmap(scaled_bitmap_theme_font, int_to_num_char(first_num), theme_x + 62, theme_y, theme_font_scale, get_symbol_unselected_color(), get_symbol_bg_color(), FONT_SIZE_3x5);
    print_bitmap(scaled_bitmap_theme_font, int_to_num_char(second_num), theme_x + 76, theme_y, theme_font_scale, get_symbol_unselected_color(), get_symbol_bg_color(), FONT_SIZE_3x5);
}

void set_next_theme_number() {
    current_theme++;
    if (current_theme >= themes_colors_len) {
        current_theme = 0;
    }
}

void set_previous_theme_number() {
    current_theme--;
    if (current_theme < 0) {
        current_theme = themes_colors_len - 1;
    }
}

void set_next_theme() {
    set_next_theme_number();
    int rc = snake_settings_save_current_theme(current_theme);
    if (rc) {
        set_previous_theme_number();
        return;
    }
    set_current_theme();
}

void set_current_theme() {
    set_colors(
        themes_colors[current_theme][0],
        themes_colors[current_theme][1],
        themes_colors[current_theme][2],
        themes_colors[current_theme][3],
        themes_colors[current_theme][4],
        themes_colors[current_theme][5]
    );
}

void theme_init() {
    uint16_t theme_font_size = (theme_font_width * theme_font_scale) * (theme_font_height * theme_font_scale);

    scaled_bitmap_theme_font = k_malloc(theme_font_size * 2 * sizeof(uint16_t));

    current_theme = snake_settings_get_current_theme();
    set_current_theme();
}