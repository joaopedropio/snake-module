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

static bool initialized_splash = false;

void print_snake_image() {
    for (uint16_t i = 0; i < snake_image_height; i++) {
        render_bitmap(snake_image_buf, snake_image[i], snake_image_x, snake_image_y + i, snake_image_width, 1, 1, get_splash_logo_color(), get_splash_bg_color());
    }
}

void print_splash(void) {
    if (initialized_splash) {
        return;
    }

    clear_screen(get_splash_bg_color());
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

void buffer_snake_image_init() {
    snake_image_buf = k_malloc(snake_image_width * 2 * sizeof(uint16_t));
}

void buffer_splash_snake_init() {
	buf_splash_snake = k_malloc((snake_splash_font_width * snake_splash_font_scale) * (snake_splash_font_height * snake_splash_font_scale) * 2u);
}

void zmk_widget_splash_init() {
	buffer_splash_snake_init();
    buffer_snake_image_init();
}

void clean_up_splash() {
    k_free(buf_splash_snake);
    k_free(snake_image_buf);
}

#else

static bool initialized_splash = false;

static uint16_t snake_splash_font_x = 90;
static uint16_t snake_splash_font_y = 210;
static uint16_t snake_splash_font_scale = 1;

static uint16_t snake_font_width = 10;
static uint16_t snake_font_height = 13;
static uint16_t snake_scale = 5;
static uint16_t *buf_splash_snake;

static uint16_t snake_logo_start_height = 84;

void print_splash(void) {
    if (initialized_splash) {
        return;
    }
    
    clear_screen(get_splash_bg_color());

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

void buffer_splash_snake_init() {
	buf_splash_snake = k_malloc((snake_font_width * snake_scale) * (snake_font_height * snake_scale) * 2u);
}

void zmk_widget_splash_init() {
	buffer_splash_snake_init();
}

void clean_up_splash() {
    k_free(buf_splash_snake);
}

#endif