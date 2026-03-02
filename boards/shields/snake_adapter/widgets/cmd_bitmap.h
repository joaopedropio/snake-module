/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

#define CMD_WIDTH 240
#define CMD_HEIGHT 240

/**
 * Draw the full compressed bitmap to a 16-bit RGB565 framebuffer.
 */
void draw_cmd_bitmap(uint16_t *framebuffer, int fbWidth, int fbHeight);

/**
 * Decode a horizontal chunk of the compressed bitmap (rows start_row..start_row+num_rows-1)
 * into the provided row_buf (must be at least CMD_WIDTH * num_rows uint16_t entries).
 */
void draw_cmd_bitmap_chunk(uint16_t *row_buf, uint16_t start_row, uint16_t num_rows);
