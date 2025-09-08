#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_configuration, LOG_LEVEL_DBG);

#include <stdlib.h>
#include "helpers/display.h"
#include "layer_switch.h"

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>

static const struct pwm_dt_spec pwm_backlight = PWM_DT_SPEC_GET(DT_CHOSEN(zephyr_backlight));

void board_size() {
    const char *size_str = CONFIG_SNAKE_BOARD_SIZE;
    if (strcmp(size_str, "XXXL") == 0) {
        set_snake_board_width(48);
        set_snake_board_height(48);
        set_snake_pixel_size(5);
    } else if (strcmp(size_str, "XXL") == 0) {
        set_snake_board_width(24);
        set_snake_board_height(24);
        set_snake_pixel_size(10);
    } else  if (strcmp(size_str, "XL") == 0) {
        set_snake_board_width(20);
        set_snake_board_height(20);
        set_snake_pixel_size(12);
    } else if (strcmp(size_str, "L") == 0) {
        set_snake_board_width(16);
        set_snake_board_height(16);
        set_snake_pixel_size(15);
    } else if (strcmp(size_str, "M") == 0) {
        set_snake_board_width(12);
        set_snake_board_height(12);
        set_snake_pixel_size(20);
    } else if (strcmp(size_str, "S") == 0) {
        set_snake_board_width(10);
        set_snake_board_height(10);
        set_snake_pixel_size(24);
    } else if (strcmp(size_str, "XS") == 0) {
        set_snake_board_width(8);
        set_snake_board_height(8);
        set_snake_pixel_size(30);
    } else {
        set_snake_board_width(24);
        set_snake_board_height(24);
        set_snake_pixel_size(10);
    }
}

void default_screen() {
    const char *default_screen_str = CONFIG_DEFAULT_SCREEN;
    if (strcmp(default_screen_str, "snake") == 0) {
        set_default_screen(SNAKE_SCREEN);
    } else if (strcmp(default_screen_str, "status") == 0) {
        set_default_screen(STATUS_SCREEN);
    } else {
        set_default_screen(SNAKE_SCREEN);
    }
}

void custom_theme() {
    uint32_t color1 = hex_string_to_uint(CONFIG_THEME_COLOR_1);
    uint32_t color2 = hex_string_to_uint(CONFIG_THEME_COLOR_2);
    uint32_t color3 = hex_string_to_uint(CONFIG_THEME_COLOR_3);
    uint32_t color4 = hex_string_to_uint(CONFIG_THEME_COLOR_4);
    uint32_t color5 = hex_string_to_uint(CONFIG_THEME_COLOR_5);
    uint32_t color6 = hex_string_to_uint(CONFIG_THEME_COLOR_6);
    if (color1 == HEX_PARSE_ERROR ||
        color2 == HEX_PARSE_ERROR ||
        color3 == HEX_PARSE_ERROR ||
        color4 == HEX_PARSE_ERROR ||
        color5 == HEX_PARSE_ERROR ||
        color6 == HEX_PARSE_ERROR) {
        set_custom_theme_colors(0x4aedffu, 0xff8acdu, 0xb03e80u, 0xFFFFFFu, 0x000000u, 0xddddddu);
    } else {
        set_custom_theme_colors(color1, color2, color3, color4, color5, color6);
    }
}

void set_display_brightness() {
    uint8_t level;
    if (CONFIG_DISPLAY_BRIGHTNESS < 0 || CONFIG_DISPLAY_BRIGHTNESS > 100) {
        level = 100;
    } else {
        level = CONFIG_DISPLAY_BRIGHTNESS;
    }
    uint32_t period = 1000;
    uint32_t duty_cycle = (period * level) / 100;
    pwm_set_dt(&pwm_backlight, PWM_HZ(period), PWM_HZ(duty_cycle));
}

void action_button() {
    set_theme_threshold(300);
    set_mute_threshold(600);
    if (CONFIG_THEME_THRESHOLD <= 0) {
        return;
    }
    if (CONFIG_THEME_THRESHOLD >= CONFIG_MUTE_THRESHOLD) {
        return;
    }
    set_theme_threshold(CONFIG_THEME_THRESHOLD);
    set_mute_threshold(CONFIG_MUTE_THRESHOLD);
}

void configure(void) {
    set_display_brightness();
    if (!CONFIG_USE_COMPLETE_CUSTOM_THEME) {
        custom_theme();
    }
    board_size();
    default_screen();
    action_button();
}
