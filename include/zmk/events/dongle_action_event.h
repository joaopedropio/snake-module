/*
- Copyright (c) 2021 The ZMK Contributors
-
- SPDX-License-Identifier: MIT
*/

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

struct zmk_dongle_actioned {
    bool pressed;
    int64_t timestamp;
};

ZMK_EVENT_DECLARE(zmk_dongle_actioned);