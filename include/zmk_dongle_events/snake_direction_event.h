/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

/* Direction values — intentionally match the Direction enum in snake.c */
#define SNAKE_DIR_UP    0
#define SNAKE_DIR_RIGHT 1
#define SNAKE_DIR_DOWN  2
#define SNAKE_DIR_LEFT  3

struct zmk_snake_direction {
    uint8_t direction;   /* SNAKE_DIR_UP … SNAKE_DIR_LEFT */
};

ZMK_EVENT_DECLARE(zmk_snake_direction);
