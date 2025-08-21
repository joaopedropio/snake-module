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

// ############## DISPLAY STATICS ##############

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
        render_bitmap(snake_image_buf, snake_image[i], snake_image_x, snake_image_y + i, snake_image_width, 1, 1, get_symbol_selected_color(), get_symbol_bg_color());
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
    print_string(buf_splash_snake, created_chars, snake_splash_font_x, snake_splash_font_y, snake_splash_font_scale, get_symbol_unselected_color(), get_symbol_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 7);
    print_string(buf_splash_snake, by_chars, snake_splash_font_x + 30, snake_splash_font_y, snake_splash_font_scale, get_symbol_unselected_color(), get_symbol_bg_color(), FONT_SIZE_3x5, char_gap_pixels, 6);
    
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