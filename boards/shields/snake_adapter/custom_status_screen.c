/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "custom_status_screen.h"
#include "widgets/battery_status.h"
#include "widgets/output_status.h"
#include "widgets/splash.h"
#include "widgets/snake.h"
#include "widgets/helpers/display.h"
#include "widgets/action_button.h"
#include "widgets/logo.h"
#include "widgets/configuration.h"
#include "widgets/wpm.h"
#include "widgets/modifier.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "widgets/helpers/buzzer.h"
#include "widgets/helpers/settings.h"

static const uint8_t SPLASH_DURATION = 50;
static const uint8_t SPLASH_FINAL_COUNT = 50;
static uint8_t splash_count = 0;
static bool splash_finished = false;

void timer_splash(lv_timer_t * timer) {
    if (splash_finished) {
        return;
    }
    if (splash_count >= SPLASH_FINAL_COUNT) {
        print_background();
        initialize_snake_game();
        initialize_battery_status();
        DefaultScreen screen = get_default_screen();
        bool menu_on;
        if (screen == STATUS_SCREEN) {
            print_menu();
            menu_on = true;
        } else {
            start_snake();
            menu_on = false;
        }
        start_action_button(menu_on);

        lv_timer_pause(timer);
        splash_finished = true;
        return;
    }
    print_splash();
    splash_count++;
}

lv_obj_t* zmk_display_status_screen() {
    configure();
    init_display();
    theme_init();
    logo_animation_init();

    #ifdef CONFIG_USE_BUZZER
    app_buzzer_init();
        #ifdef CONFIG_USE_SPLASH_SOUND
            play_snake_game_intro();
        #endif
    #endif
    
    zmk_widget_peripheral_status_init();
    zmk_widget_splash_init();
    zmk_widget_snake_init();
    zmk_widget_output_status_init();
    zmk_widget_peripheral_battery_status_init();
    zmk_widget_layer_init();
    zmk_widget_action_button_init();
    zmk_widget_wpm_init();
    zmk_widget_modifier_init();

    lv_timer_create(timer_splash, SPLASH_DURATION, NULL);
    lv_timer_create(logo_animation_timer, CONFIG_LOGO_WALK_INTERVAL, NULL);

    return lv_obj_create(NULL);
}