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

#include "splash.h"
#include "helpers/display.h"
#include "snake_image.h"

#if defined(CONFIG_SPLASH_USE_DONGLE)
#include "snake_logo_text.h"
#endif


#ifdef CONFIG_SPLASH_USE_SNAKE_2

static uint16_t *snake_image_buf;
static uint16_t snake_image_x = 24;
static uint16_t snake_image_y = 56;

static uint16_t snake_splash_font_x = 90;
static uint16_t snake_splash_font_y = 210;
static uint16_t snake_splash_font_width = 3;
static uint16_t snake_splash_font_height = 6;
static uint16_t snake_splash_font_scale = 1;
static uint16_t *buf_splash_snake;

static uint16_t snake_logo_start_height = 56;
static uint16_t background_pixel_width = 60;
static uint16_t background_pixel_height = 56;

static uint8_t *buf_background_splash;
static struct display_buffer_descriptor buf_desc_background_splash;
static size_t buf_size_background_splash = 0;

static uint16_t gap_width = 24;
static uint16_t gap_height = 128;
static uint8_t *buf_gap_splash;
static struct display_buffer_descriptor buf_desc_gap_splash;
static size_t buf_size_gap_splash = 0;

static bool initialized_splash = false;

void print_snake_image() {
    for (uint16_t i = 0; i < snake_image_height; i++) {
        render_bitmap(snake_image_buf, snake_image[i], snake_image_x, snake_image_y + i, snake_image_width, 1, 1, get_splash_logo_color(), get_splash_bg_color());
    }
}

void render_background_splash_pixel(uint8_t x, uint8_t y, uint8_t offset_x, uint8_t offset_y) {
    uint16_t initial_y = (y * background_pixel_height) + offset_y;
    uint16_t initial_x = (x * background_pixel_width) + offset_x;
	display_write_wrapper(initial_x, initial_y, &buf_desc_background_splash, buf_background_splash);
}

void print_background(void) {
    clear_screen();
}

void print_splash(void) {
    if (initialized_splash) {
        return;
    }
    
	for (uint8_t x = 0; x < 4; x++) {
        render_background_splash_pixel(x, 0, 0, 0);
    }
	for (uint8_t x = 0; x < 4; x++) {
        render_background_splash_pixel(x, 0, 0, 184);
    }

    display_write_wrapper(0, snake_logo_start_height, &buf_desc_gap_splash, buf_gap_splash);
    display_write_wrapper(216, snake_logo_start_height, &buf_desc_gap_splash, buf_gap_splash);
    print_snake_image();

    Character created_chars[] = {
        CHAR_C,
        CHAR_R,
        CHAR_E,
        CHAR_A,
        CHAR_T,
        CHAR_E,
        CHAR_D,
    };
    Character by_chars[] = {
        CHAR_B,
        CHAR_Y,
        CHAR_COLON,
        CHAR_P,
        CHAR_I,
        CHAR_O,
    };
    uint16_t char_gap_pixels = 1;
    print_string(buf_splash_snake, created_chars, snake_splash_font_x, snake_splash_font_y, snake_splash_font_scale, get_splash_created_by_color(), get_splash_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 7);
    print_string(buf_splash_snake, by_chars, snake_splash_font_x + 30, snake_splash_font_y, snake_splash_font_scale, get_splash_created_by_color(), get_splash_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 6);
    
    initialized_splash = true;
}

// ############## Display setup ################

void buffer_gap_splash_init() {
	buf_size_gap_splash = gap_width * gap_height * 2u;
	buf_gap_splash = k_malloc(buf_size_gap_splash);
	buf_desc_gap_splash.pitch = gap_width;
	buf_desc_gap_splash.width = gap_width;
	buf_desc_gap_splash.height = gap_height;
	fill_buffer_color(buf_gap_splash, buf_size_gap_splash, get_splash_bg_color());
}

void buffer_snake_image_init() {
    snake_image_buf = k_malloc(snake_image_width * 2 * sizeof(uint16_t));
}

void buffer_splash_snake_init() {
	buf_splash_snake = k_malloc((snake_splash_font_width * snake_splash_font_scale) * (snake_splash_font_height * snake_splash_font_scale) * 2u);
}

void buffer_background_splash_init() {
	buf_size_background_splash = background_pixel_width * background_pixel_height * 2u;
	buf_background_splash = k_malloc(buf_size_background_splash);
	buf_desc_background_splash.pitch = background_pixel_width;
	buf_desc_background_splash.width = background_pixel_width;
	buf_desc_background_splash.height = background_pixel_height;
	fill_buffer_color(buf_background_splash, buf_size_background_splash, get_splash_bg_color());
}

void zmk_widget_splash_init() {
	buffer_splash_snake_init();
	buffer_background_splash_init();
    buffer_gap_splash_init();
    buffer_snake_image_init();
}

void clean_up_splash() {
    k_free(buf_background_splash);
    k_free(buf_splash_snake);
    k_free(buf_gap_splash);
    k_free(snake_image_buf);
}

#elif defined(CONFIG_SPLASH_USE_DONGLE)

// ======================== Snake Dongle Splash ========================
// Layout: "SNAKE" logo bitmap | "DONGLE" multicolor text | cobra image | credit

static uint16_t *snake_logo_text_buf_splash;
static uint16_t *snake_image_buf_splash;
static uint16_t *buf_dongle_text;
static uint16_t *buf_credit_text;

static bool initialized_splash = false;

// "SNAKE" logo text bitmap (150x58) centered at x=45
static uint16_t logo_text_x = 45;
static uint16_t logo_text_y = 2;

// "DONGLE" multicolor text: FONT_SIZE_10x13 scale 2 → 20x26 per char
// 6 chars × 20 + 5 gaps × 4 = 140px. Centered: x = (240-140)/2 = 50
static uint16_t dongle_font_width = 10;
static uint16_t dongle_font_height = 13;
static uint16_t dongle_scale = 2;
static uint16_t dongle_text_y = 62;

// Snake cobra image (192x128) centered at x=24
static uint16_t snake_img_x = 24;
static uint16_t snake_img_y = 92;

// Credit text: FONT_SIZE_3x5 scale 1
static uint16_t credit_font_width = 3;
static uint16_t credit_font_height = 5;
static uint16_t credit_scale = 1;
static uint16_t credit_text_y = 224;

void print_snake_logo_text_splash() {
    for (uint16_t i = 0; i < snake_logo_text_height; i++) {
        render_bitmap(snake_logo_text_buf_splash, snake_logo_text[i],
                      logo_text_x, logo_text_y + i,
                      snake_logo_text_width, 1, 1,
                      get_splash_logo_color(), get_splash_bg_color());
    }
}

void print_snake_image_splash() {
    for (uint16_t i = 0; i < snake_image_height; i++) {
        render_bitmap(snake_image_buf_splash, snake_image[i],
                      snake_img_x, snake_img_y + i,
                      snake_image_width, 1, 1,
                      get_splash_logo_color(), get_splash_bg_color());
    }
}

void print_background(void) {
    clear_screen();
}

void print_splash(void) {
    if (initialized_splash) {
        return;
    }

    print_background();

    // 1) Render "SNAKE" logo text bitmap at top
    print_snake_logo_text_splash();

    // 2) Render "DONGLE" in multicolor below the logo
    uint16_t colors[4] = {
        get_splash_logo_multicolor_0(),
        get_splash_logo_multicolor_1(),
        get_splash_logo_multicolor_2(),
        get_splash_logo_multicolor_3(),
    };
    Character dongle_chars[] = { CHAR_D, CHAR_O, CHAR_N, CHAR_G, CHAR_L, CHAR_E };
    uint16_t dongle_x_positions[] = { 50, 74, 98, 122, 146, 170 };
    for (uint8_t i = 0; i < 6; i++) {
        print_bitmap_multicolor(buf_dongle_text, dongle_chars[i],
                                dongle_x_positions[i], dongle_text_y,
                                dongle_scale, colors, FONT_SIZE_10x13);
    }

    // 3) Render snake cobra image
    print_snake_image_splash();

    // 4) Render credit text "VAIBHAV RAJPUT"
    // "VAIBHAV" = 7 chars × 3px + 6 × 1px gap = 27px
    // gap 4px
    // "RAJPUT"  = 6 chars × 3px + 5 × 1px gap = 23px
    // Total = 54px. Centered: x = (240 - 54) / 2 = 93
    Character vaibhav_chars[] = {
        CHAR_V, CHAR_A, CHAR_I, CHAR_B, CHAR_H, CHAR_A, CHAR_V
    };
    Character rajput_chars[] = {
        CHAR_R, CHAR_A, CHAR_J, CHAR_P, CHAR_U, CHAR_T
    };
    uint16_t char_gap_pixels = 1;
    print_string(buf_credit_text, vaibhav_chars, 93, credit_text_y,
                 credit_scale, get_splash_created_by_color(),
                 get_splash_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 7);
    print_string(buf_credit_text, rajput_chars, 93 + 27 + 4, credit_text_y,
                 credit_scale, get_splash_created_by_color(),
                 get_splash_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 6);

    initialized_splash = true;
}

// ############## Display setup ################

void zmk_widget_splash_init() {
    snake_logo_text_buf_splash = k_malloc(snake_logo_text_width * 2 * sizeof(uint16_t));
    snake_image_buf_splash = k_malloc(snake_image_width * 2 * sizeof(uint16_t));
    buf_dongle_text = k_malloc((dongle_font_width * dongle_scale) * (dongle_font_height * dongle_scale) * 2u);
    buf_credit_text = k_malloc((credit_font_width * credit_scale) * (credit_font_height * credit_scale) * 2u);
}

void clean_up_splash() {
    k_free(snake_logo_text_buf_splash);
    k_free(snake_image_buf_splash);
    k_free(buf_dongle_text);
    k_free(buf_credit_text);
}

#else

static bool initialized_splash = false;

static uint16_t snake_splash_font_x = 90;
static uint16_t snake_splash_font_y = 210;
static uint16_t snake_splash_font_width = 10;
static uint16_t snake_splash_font_height = 13;
static uint16_t snake_splash_font_scale = 1;

static uint16_t snake_font_width = 10;
static uint16_t snake_font_height = 13;
static uint16_t snake_scale = 5;
static uint16_t *buf_splash_snake;

static uint16_t snake_logo_start_height = 84;
static uint16_t background_pixel_width = 60;
static uint16_t background_pixel_height = 42;

static uint8_t *buf_background_splash;
static struct display_buffer_descriptor buf_desc_background_splash;
static size_t buf_size_background_splash = 0;

static uint16_t gap_width = 20;
static uint16_t gap_height = 72;
static uint8_t *buf_gap_splash;
static struct display_buffer_descriptor buf_desc_gap_splash;
static size_t buf_size_gap_splash = 0;

static uint16_t gap1_width = 4;
static uint16_t gap1_height = 72;
static uint8_t *buf_gap1_splash;
static struct display_buffer_descriptor buf_desc_gap1_splash;
static size_t buf_size_gap1_splash = 0;

void render_background_splash_pixel(uint8_t x, uint8_t y, uint8_t offset_x, uint8_t offset_y) {
    uint16_t initial_y = (y * background_pixel_height) + offset_y;
    uint16_t initial_x = (x * background_pixel_width) + offset_x;
	display_write_wrapper(initial_x, initial_y, &buf_desc_background_splash, buf_background_splash);
}

void print_background(void) {
    clear_screen();
}

void print_splash(void) {
    if (initialized_splash) {
        return;
    }
    
    print_background();

    uint16_t colors[4] = {
        get_splash_logo_multicolor_0(),
        get_splash_logo_multicolor_1(),
        get_splash_logo_multicolor_2(),
        get_splash_logo_multicolor_3(),
    };

    print_bitmap_multicolor(buf_splash_snake, CHAR_S, 5, snake_logo_start_height, snake_scale, colors, FONT_SIZE_10x13);
	print_bitmap_multicolor(buf_splash_snake, CHAR_N, 50, snake_logo_start_height, snake_scale, colors, FONT_SIZE_10x13);
	print_bitmap_multicolor(buf_splash_snake, CHAR_A, 95, snake_logo_start_height, snake_scale, colors, FONT_SIZE_10x13);
	print_bitmap_multicolor(buf_splash_snake, CHAR_K, 140, snake_logo_start_height, snake_scale, colors, FONT_SIZE_10x13);
	print_bitmap_multicolor(buf_splash_snake, CHAR_E, 185, snake_logo_start_height, snake_scale, colors, FONT_SIZE_10x13);

    Character created_chars[] = {
        CHAR_C,
        CHAR_R,
        CHAR_E,
        CHAR_A,
        CHAR_T,
        CHAR_E,
        CHAR_D,
    };
    Character by_chars[] = {
        CHAR_B,
        CHAR_Y,
        CHAR_COLON,
        CHAR_P,
        CHAR_I,
        CHAR_O,
    };
    uint16_t char_gap_pixels = 1;
    print_string(buf_splash_snake, created_chars, snake_splash_font_x, snake_splash_font_y, snake_splash_font_scale, get_splash_created_by_color(), get_splash_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 7);
    print_string(buf_splash_snake, by_chars, snake_splash_font_x + 30, snake_splash_font_y, snake_splash_font_scale, get_splash_created_by_color(), get_splash_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 6);
    
    initialized_splash = true;
}

// ############## Display setup ################

void buffer_gap_splash_init() {
	buf_size_gap_splash = gap_width * gap_height * 2u;
	buf_gap_splash = k_malloc(buf_size_gap_splash);
	buf_desc_gap_splash.pitch = gap_width;
	buf_desc_gap_splash.width = gap_width;
	buf_desc_gap_splash.height = gap_height;
	fill_buffer_color(buf_gap_splash, buf_size_gap_splash, get_splash_bg_color());
}

void buffer_gap1_splash_init() {
	buf_size_gap1_splash = gap1_width * gap1_height * 2u;
	buf_gap1_splash = k_malloc(buf_size_gap1_splash);
	buf_desc_gap1_splash.pitch = gap1_width;
	buf_desc_gap1_splash.width = gap1_width;
	buf_desc_gap1_splash.height = gap1_height;
	fill_buffer_color(buf_gap1_splash, buf_size_gap1_splash, get_splash_bg_color());
}

void buffer_splash_snake_init() {
	buf_splash_snake = k_malloc((snake_font_width * snake_scale) * (snake_font_height * snake_scale) * 2u);
}

void buffer_background_splash_init() {
	buf_size_background_splash = background_pixel_width * background_pixel_height * 2u;
	buf_background_splash = k_malloc(buf_size_background_splash);
	buf_desc_background_splash.pitch = background_pixel_width;
	buf_desc_background_splash.width = background_pixel_width;
	buf_desc_background_splash.height = background_pixel_height;
	fill_buffer_color(buf_background_splash, buf_size_background_splash, get_splash_bg_color());
}

void zmk_widget_splash_init() {
	buffer_splash_snake_init();
	buffer_background_splash_init();
    buffer_gap_splash_init();
    buffer_gap1_splash_init();
}

void clean_up_splash() {
    k_free(buf_background_splash);
    k_free(buf_splash_snake);
    k_free(buf_gap_splash);
    k_free(buf_gap1_splash);
}

#endif