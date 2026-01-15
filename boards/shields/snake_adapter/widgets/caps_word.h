/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
 
#pragma once

#include <zephyr/kernel.h>
#include <lvgl.h>

void zmk_widget_caps_word_init(void);
void print_caps_word();
void start_caps_word_status();
void stop_caps_word_status();