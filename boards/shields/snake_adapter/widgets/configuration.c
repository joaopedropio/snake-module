#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_configuration, LOG_LEVEL_DBG);

#include <stdlib.h>
#include "helpers/display.h"
#include "action_button.h"

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>

static const struct pwm_dt_spec pwm_backlight = PWM_DT_SPEC_GET(DT_CHOSEN(zephyr_backlight));

SlotName get_slot_name_from_var(char *slot_name) {
    if (strcmp(slot_name, "modifiers") == 0) {
        return SLOT_NAME_MODIFIERS;
    }
    if (strcmp(slot_name, "connectivity") == 0) {
        return SLOT_NAME_CONNECTIVITY;
    }
    if (strcmp(slot_name, "layer") == 0) {
        return SLOT_NAME_LAYER;
    }
    if (strcmp(slot_name, "theme") == 0) {
        return SLOT_NAME_THEME;
    }
    if (strcmp(slot_name, "wpm") == 0) {
        return SLOT_NAME_WPM;
    }
    return SLOT_NAME_NONE;
}

SlotMode get_slot_mode_from_var(char *slot_mode) {
    if (strcmp(slot_mode, "2-slot") == 0) {
        return SLOT_MODE_2;
    }
    if (strcmp(slot_mode, "4-slot") == 0) {
        return SLOT_MODE_4;
    }
    if (strcmp(slot_mode, "6-slot") == 0) {
        return SLOT_MODE_6;
    }
    return SLOT_MODE_2;
}

void reset_slots() {
    set_slot_1(SLOT_NAME_NONE);
    set_slot_2(SLOT_NAME_NONE);
    set_slot_3(SLOT_NAME_NONE);
    set_slot_4(SLOT_NAME_NONE);
    set_slot_5(SLOT_NAME_NONE);
    set_slot_6(SLOT_NAME_NONE);
}

void info_slots() {
    reset_slots();
    set_slot_mode(get_slot_mode_from_var(CONFIG_INFO_SLOT_MODE));
    set_slot_1(get_slot_name_from_var(CONFIG_INFO_SLOT_1));
    set_slot_2(get_slot_name_from_var(CONFIG_INFO_SLOT_2));
    set_slot_3(get_slot_name_from_var(CONFIG_INFO_SLOT_3));
    set_slot_4(get_slot_name_from_var(CONFIG_INFO_SLOT_4));
    set_slot_5(get_slot_name_from_var(CONFIG_INFO_SLOT_5));
    set_slot_6(get_slot_name_from_var(CONFIG_INFO_SLOT_6));
}

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
    uint32_t color1 = hex_string_to_uint(CONFIG_THEME_PRIMARY_COLOR);
    uint32_t color2 = hex_string_to_uint(CONFIG_THEME_SECONDARY_COLOR);
    uint32_t color3 = hex_string_to_uint(CONFIG_THEME_BG_COLOR);
    uint32_t color4 = hex_string_to_uint(CONFIG_THEME_BG_DARKER_COLOR);
    if (color1 == HEX_PARSE_ERROR ||
        color2 == HEX_PARSE_ERROR ||
        color3 == HEX_PARSE_ERROR ||
        color4 == HEX_PARSE_ERROR) {
        // https://lospec.com/palette-list/b4sement
        set_custom_theme_colors(0x3dff98u, 0xff4adcu, 0x222323u, 0x121313u, 0, 0);
    } else {
        set_custom_theme_colors(color1, color2, color3, color4, 0x000000u, 0x000000u);
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

void rotate_display() {
    if (CONFIG_ROTATE_DISPLAY == 270) {
        set_display_orientation(DISPLAY_ORIENTATION_270);
        return;
    }
    if (CONFIG_ROTATE_DISPLAY == 180) {
        set_display_orientation(DISPLAY_ORIENTATION_180);
        return;
    }
    if (CONFIG_ROTATE_DISPLAY == 90) {
        set_display_orientation(DISPLAY_ORIENTATION_90);
        return;
    }
    set_display_orientation(DISPLAY_ORIENTATION_0);
}

void configure(void) {
    rotate_display();
    info_slots();
    set_display_brightness();
    custom_theme();
    board_size();
    default_screen();
    action_button();
}
