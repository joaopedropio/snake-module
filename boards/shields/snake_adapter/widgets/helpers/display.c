#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include "display.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static const struct device *display_dev;
static uint8_t *buf_screen_area;

static uint8_t screen_width;
static uint8_t screen_height;
static size_t buf_screen_size;

static uint16_t splash_logo_color;
static uint16_t splash_created_by_color;
static uint16_t splash_bg_color;

static uint16_t snake_font_color;
static uint16_t snake_num_color;
static uint16_t snake_bg_color;

static uint16_t snake_default_color;
static uint16_t snake_board_color;
static uint16_t snake_board_1_color;
static uint16_t food_color;

static uint16_t snake_color_0;
static uint16_t snake_color_1;
static uint16_t snake_color_2;
static uint16_t snake_color_3;
static uint16_t snake_color_4;
static uint16_t snake_color_5;
static uint16_t snake_color_6;

static uint16_t battery_num_color;
static uint16_t battery_percentage_color;
static uint16_t battery_bg_color;

static uint16_t symbol_selected_color;
static uint16_t symbol_unselected_color;
static uint16_t symbol_bg_color;
static uint16_t bt_num_color;
static uint16_t bt_bg_color;

static uint16_t frame_color;
static uint16_t frame_color_1;

static DefaultScreen default_screen = SNAKE_SCREEN;

#define COLORS_PER_THEME 6

static uint8_t themes_colors_len = 12;
static uint32_t themes_colors[][COLORS_PER_THEME] = {
    // primary  secondary  back1      back2
    {0x222323u, 0xff4adcu, 0x3dff98u, 0xf0f6f0u, 0xf0f6f0u, 0xddddddu}, // C  - custom https://lospec.com/palette-list/b4sement
    {0xd0d058u, 0xa0a840u, 0x708028u, 0x405010u, 0xf0f6f0u, 0xddddddu}, // 01 - https://lospec.com/palette-list/nostalgia
    {0x222323u, 0xff4adcu, 0x3dff98u, 0xf0f6f0u, 0xf0f6f0u, 0xddddddu}, // 02 - https://lospec.com/palette-list/b4sement
    {0xff3796u, 0x302387u, 0x00faacu, 0xfffdafu, 0xf0f6f0u, 0xddddddu}, // 03 - https://lospec.com/palette-list/fuzzyfour
    {0x46878fu, 0x332c50u, 0x94e344u, 0xe2f3e4u, 0xf0f6f0u, 0xddddddu}, // 04 - https://lospec.com/palette-list/kirokaze-gameboy
    {0x203671u, 0x0f052du, 0x36868fu, 0x5fc75du, 0xf0f6f0u, 0xddddddu}, // 05 - https://lospec.com/palette-list/moonlight-gb
    {0xff4d6du, 0xfcdeeau, 0x265935u, 0x012824u, 0xf0f6f0u, 0xddddddu}, // 06 - https://lospec.com/palette-list/cherrymelon
    {0x545c7eu, 0x282328u, 0xc56981u, 0xa3a29au, 0xf0f6f0u, 0xddddddu}, // 07 - https://lospec.com/palette-list/bittersweet
    {0x71969fu, 0xfce4a8u, 0xd71a21u, 0x01334eu, 0xf0f6f0u, 0xddddddu}, // 08 - https://lospec.com/palette-list/americana
    {0xc53a9du, 0x051f39u, 0x4a2480u, 0xff8e80u, 0xf0f6f0u, 0xddddddu}, // 09 - https://lospec.com/palette-list/lava-gb
    {0xfb6634u, 0x2b2132u, 0x7e0d0du, 0xfeb746u, 0xf0f6f0u, 0xddddddu}, // 10 - https://lospec.com/gallery/2bitdream/of-the-earth
    {0xecfffbu, 0x323859u, 0x576373u, 0x0c0421u, 0xf0f6f0u, 0xddddddu}, // 11 - https://lospec.com/gallery/dogmaster/cave
};

static const uint16_t empty_bitmap_5x7[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};
static const uint16_t none_bitmap_5x7[] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 1, 0, 0,
};
static const uint16_t b_bitmap_5x7[] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
};
static const uint16_t e_bitmap_5x7[] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
};
static const uint16_t h_bitmap_5x7[] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
};
static const uint16_t l_bitmap_5x7[] = {
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
};
static const uint16_t m_bitmap_5x7[] = {
    0, 1, 0, 1, 0,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 0, 0, 1,
};
static const uint16_t n_bitmap_5x7[] = {
    1, 0, 0, 0, 1,
    1, 1, 0, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 0, 1, 1,
    1, 0, 0, 1, 1,
    1, 0, 0, 0, 1,
};
static const uint16_t p_bitmap_5x7[] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
};
static const uint16_t s_bitmap_5x7[] = {
    0, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
};
static const uint16_t t_bitmap_5x7[] = {
    1, 1, 1, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
};
static const uint16_t w_bitmap_5x7[] = {
    1, 0, 0, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    0, 1, 0, 1, 0,
};
static const uint16_t colon_bitmap_5x7[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 1, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 1, 0, 0, 0,
    0, 0, 0, 0, 0,
};
static const uint16_t num_bitmaps_5x7[10][35] = {
    {// zero
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// one
        0, 0, 1, 0, 0,
        0, 1, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 1, 1, 0
    },
    {// two
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0,
        0, 0, 1, 0, 0,
        0, 1, 0, 0, 0,
        1, 1, 1, 1, 1
    },
    {// three
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        0, 0, 0, 0, 1,
        0, 0, 1, 1, 0,
        0, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// four
        0, 0, 0, 1, 0,
        0, 0, 1, 1, 0,
        0, 1, 0, 1, 0,
        1, 0, 0, 1, 0,
        1, 1, 1, 1, 1,
        0, 0, 0, 1, 0,
        0, 0, 0, 1, 0
    },
    {// five
        1, 1, 1, 1, 1,
        1, 0, 0, 0, 0,
        1, 1, 1, 1, 0,
        0, 0, 0, 0, 1,
        0, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// six
        0, 0, 1, 1, 0,
        0, 1, 0, 0, 0,
        1, 0, 0, 0, 0,
        1, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// seven
        1, 1, 1, 1, 1,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0,
        0, 0, 0, 1, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 0, 0, 0,
    },
    {// eight
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// nine
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 1,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0,
        1, 1, 1, 0, 0
    },
};

static const uint16_t none_bitmap_5x8[] = {
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
};
static const uint16_t dash_bitmap_5x8[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};
static const uint16_t f_bitmap_5x8[] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
};
static const uint16_t u_bitmap_5x8[] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
};
static const uint16_t l_bitmap_5x8[] = {
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
};
static const uint16_t percentage_bitmap_5x8[] = {
    1, 1, 0, 0, 1,
    1, 1, 0, 1, 0,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    0, 1, 0, 1, 1,
    1, 0, 0, 1, 1,
};

static const uint16_t num_bitmaps_3x5[10][15] = {
    {
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        0, 1, 0,
        1, 1, 0,
        0, 1, 0,
        0, 1, 0,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
    },
    {
         1, 1, 1,
         0, 0, 1,
         1, 1, 1,
         0, 0, 1,
         1, 1, 1,
    },
    {
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
    },
    {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    },
    {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
    },
};

static const uint16_t num_bitmaps_3x6[10][18] = {
    {
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        0, 1, 0,
        1, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        1, 0, 0,
        1, 0, 0,
        1, 1, 1,
    },
    {
         1, 1, 1,
         0, 0, 1,
         1, 1, 1,
         0, 0, 1,
         0, 0, 1,
         1, 1, 1,
    },
    {
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    },
    {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    },
    {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
    },
    {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        1, 1, 1,
    },
};

static const uint16_t num_bitmaps_5x8[10][40] = {
    {// zero
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// one
        0, 0, 1, 0, 0,
        0, 1, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 1, 1, 0
    },
    {// two
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0,
        0, 0, 1, 0, 0,
        0, 1, 0, 0, 0,
        1, 0, 0, 0, 0,
        1, 1, 1, 1, 1
    },
    {// three
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        0, 0, 0, 0, 1,
        0, 0, 1, 1, 0,
        0, 0, 0, 0, 1,
        0, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// four
        0, 0, 0, 1, 0,
        0, 0, 1, 1, 0,
        0, 1, 0, 1, 0,
        1, 0, 0, 1, 0,
        1, 1, 1, 1, 1,
        0, 0, 0, 1, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 1, 0
    },
    {// five
        1, 1, 1, 1, 1,
        1, 0, 0, 0, 0,
        1, 0, 0, 0, 0,
        1, 1, 1, 1, 0,
        0, 0, 0, 0, 1,
        0, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// six
        0, 0, 1, 1, 0,
        0, 1, 0, 0, 0,
        1, 0, 0, 0, 0,
        1, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// seven
        1, 1, 1, 1, 1,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0,
        0, 0, 0, 1, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 0, 0, 0,
        0, 1, 0, 0, 0
    },
    {// eight
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 0
    },
    {// nine
        0, 1, 1, 1, 0,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        0, 1, 1, 1, 1,
        0, 0, 0, 0, 1,
        0, 0, 0, 1, 0,
        1, 1, 1, 0, 0
    },
};

const uint16_t none_letter_3x5[] = {
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
};
const uint16_t colon_letter_3x5[] = {
    0, 0, 0,
    1, 0, 0,
    0, 0, 0,
    1, 0, 0,
    0, 0, 0,
};
const uint16_t o_letter_3x5[] = {
    1, 1, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
    1, 1, 1,
};
const uint16_t p_letter_3x5[] = {
    1, 1, 1,
    1, 0, 1,
    1, 1, 1,
    1, 0, 0,
    1, 0, 0,
};
const uint16_t y_letter_3x5[] = {
    1, 0, 1,
    1, 0, 1,
    1, 1, 1,
    0, 0, 1,
    1, 1, 1,
};
const uint16_t b_letter_3x5[] = {
    1, 1, 0,
    1, 0, 1,
    1, 1, 0,
    1, 0, 1,
    1, 1, 1,
};
const uint16_t d_letter_3x5[] = {
    1, 1, 0,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
    1, 1, 0,
};
const uint16_t t_letter_3x5[] = {
    1, 1, 1,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
};
const uint16_t r_letter_3x5[] = {
    1, 1, 1,
    1, 0, 1,
    1, 1, 0,
    1, 0, 1,
    1, 0, 1,
};
const uint16_t c_letter_3x5[] = {
    1, 1, 1,
    1, 0, 0,
    1, 0, 0,
    1, 0, 0,
    1, 1, 1,
};
const uint16_t s_letter_3x5[] = {
    1, 1, 1,
    1, 0, 0,
    1, 1, 1,
    0, 0, 1,
    1, 1, 1,
};
const uint16_t n_letter_3x5[] = {
    1, 1, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
};
const uint16_t a_letter_3x5[] = {
    1, 1, 1,
    1, 0, 1,
    1, 1, 1,
    1, 0, 1,
    1, 0, 1,
};
const uint16_t k_letter_3x5[] = {
    1, 0, 1,
    1, 0, 1,
    1, 1, 0,
    1, 0, 1,
    1, 0, 1,
};
const uint16_t e_letter_3x5[] = {
    1, 1, 1,
    1, 0, 0,
    1, 1, 1,
    1, 0, 0,
    1, 1, 1,
};
const uint16_t i_letter_3x5[] = {
    1, 1, 1,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    1, 1, 1,
};

const uint16_t none_letter_3x6[] = {
    1, 1, 1,
    1, 1, 1,
    1, 1, 1,
    1, 1, 1,
    1, 1, 1,
    1, 1, 1,
};
const uint16_t s_letter_3x6[] = {
    1, 1, 1,
    1, 0, 0,
    1, 1, 1,
    0, 0, 1,
    0, 0, 1,
    1, 1, 1,
};
const uint16_t n_letter_3x6[] = {
    1, 1, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
};
const uint16_t a_letter_3x6[] = {
    1, 1, 1,
    1, 0, 1,
    1, 1, 1,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
};
const uint16_t k_letter_3x6[] = {
    1, 0, 1,
    1, 0, 1,
    1, 1, 0,
    1, 0, 1,
    1, 0, 1,
    1, 0, 1,
};
const uint16_t e_letter_3x6[] = {
    1, 1, 1,
    1, 0, 0,
    1, 1, 0,
    1, 0, 0,
    1, 0, 0,
    1, 1, 1,
};
const uint16_t i_letter_3x6[] = {
    1, 1, 1,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    1, 1, 1,
};

// ###############################################################

uint8_t get_themes_colors_len () {
    return themes_colors_len;
}

void set_custom_theme_colors(uint32_t color1, uint32_t color2, uint32_t color3, uint32_t color4, uint32_t color5, uint32_t color6) {
    themes_colors[0][0] = color1;
    themes_colors[0][1] = color2;
    themes_colors[0][2] = color3;
    themes_colors[0][3] = color4;
    themes_colors[0][4] = color5;
    themes_colors[0][5] = color6;
}

void apply_current_theme(uint8_t current_theme) {
    set_colors(
        themes_colors[current_theme][0],
        themes_colors[current_theme][1],
        themes_colors[current_theme][2],
        themes_colors[current_theme][3],
        themes_colors[current_theme][4],
        themes_colors[current_theme][5]
    );
}

uint16_t rgb888_to_rgb565(uint32_t color) {
    uint16_t red = (((color & 0xff0000) / 0x10000) * 31 / 255);
    uint16_t green = (((color & 0x00ff00) / 0x100) * 63 / 255);
    uint16_t blue = (((color & 0x0000ff) / 0x1) * 31 / 255);
    
    // Shift the red value to the left by 11 bits.
    uint16_t red_shifted = red << 11;
    // Shift the green value to the left by 5 bits.
    uint16_t green_shifted = green << 5;

    // Combine the red, green, and blue values.
    return red_shifted | green_shifted | blue;
}

void set_default_screen(DefaultScreen screen) {
    default_screen = screen;
}

void set_splash_logo_color(uint32_t color) {
    splash_logo_color = rgb888_to_rgb565(color);
}

void set_splash_created_by_color(uint32_t color) {
    splash_created_by_color = rgb888_to_rgb565(color);
}

void set_splash_bg_color(uint32_t color) {
    splash_bg_color = rgb888_to_rgb565(color);
}

void set_snake_font_color(uint32_t color) {
    snake_font_color = rgb888_to_rgb565(color);
}

void set_snake_num_color(uint32_t color) {
    snake_num_color = rgb888_to_rgb565(color);
}

void set_snake_bg_color(uint32_t color) {
    snake_bg_color = rgb888_to_rgb565(color);
}

void set_snake_default_color(uint32_t color) {
    snake_default_color = rgb888_to_rgb565(color);
}

void set_snake_board_color(uint32_t color) {
    snake_board_color = rgb888_to_rgb565(color);
}

void set_snake_board_1_color(uint32_t color) {
    snake_board_1_color = rgb888_to_rgb565(color);
}

void set_food_color(uint32_t color) {
    food_color = rgb888_to_rgb565(color);
}

void set_snake_color_0(uint32_t color) {
    snake_color_0 = rgb888_to_rgb565(color);
}

void set_snake_color_1(uint32_t color) {
    snake_color_1 = rgb888_to_rgb565(color);
}

void set_snake_color_2(uint32_t color) {
    snake_color_2 = rgb888_to_rgb565(color);
}

void set_snake_color_3(uint32_t color) {
    snake_color_3 = rgb888_to_rgb565(color);
}

void set_snake_color_4(uint32_t color) {
    snake_color_4 = rgb888_to_rgb565(color);
}

void set_snake_color_5(uint32_t color) {
    snake_color_5 = rgb888_to_rgb565(color);
}

void set_snake_color_6(uint32_t color) {
    snake_color_6 = rgb888_to_rgb565(color);
}

void set_battery_num_color(uint32_t color) {
    battery_num_color = rgb888_to_rgb565(color);
}

void set_battery_percentage_color(uint32_t color) {
    battery_percentage_color = rgb888_to_rgb565(color);
}

void set_battery_bg_color(uint32_t color) {
    battery_bg_color = rgb888_to_rgb565(color);
}

void set_frame_color(uint32_t color) {
    frame_color = rgb888_to_rgb565(color);
}

void set_frame_color_1(uint32_t color) {
    frame_color_1 = rgb888_to_rgb565(color);
}

void set_symbol_selected_color(uint32_t color) {
    symbol_selected_color = rgb888_to_rgb565(color);
}

void set_symbol_unselected_color(uint32_t color) {
    symbol_unselected_color = rgb888_to_rgb565(color);
}

void set_symbol_bg_color(uint32_t color) {
    symbol_bg_color = rgb888_to_rgb565(color);
}

void set_bt_num_color(uint32_t color) {
    bt_num_color = rgb888_to_rgb565(color);
}

void set_bt_bg_color(uint32_t color) {
    bt_bg_color = rgb888_to_rgb565(color);
}

DefaultScreen get_default_screen() {
    return default_screen;
}

uint16_t get_splash_created_by_color() {
    return splash_created_by_color;
}

uint16_t get_splash_logo_color() {
    return splash_logo_color;
}

uint16_t get_splash_bg_color() {
    return splash_bg_color;
}

uint16_t get_snake_font_color() {
    return snake_font_color;
}

uint16_t get_snake_num_color() {
    return snake_num_color;
}

uint16_t get_snake_bg_color() {
    return snake_bg_color;
}

uint16_t get_snake_default_color() {
    return snake_default_color;
}

uint16_t get_snake_board_color() {
    return snake_board_color;
}

uint16_t get_snake_board_1_color() {
    return snake_board_1_color;
}

uint16_t get_food_color() {
    return food_color;
}

uint16_t get_snake_color_0() {
    return snake_color_0;
}

uint16_t get_snake_color_1() {
    return snake_color_1;
}

uint16_t get_snake_color_2() {
    return snake_color_2;
}

uint16_t get_snake_color_3() {
    return snake_color_3;
}

uint16_t get_snake_color_4() {
    return snake_color_4;
}

uint16_t get_snake_color_5() {
    return snake_color_5;
}

uint16_t get_snake_color_6() {
    return snake_color_6;
}

uint16_t get_battery_num_color() {
    return battery_num_color;
}

uint16_t get_battery_percentage_color() {
    return battery_percentage_color;
}

uint16_t get_battery_bg_color() {
    return battery_bg_color;
}

uint16_t get_symbol_selected_color() {
    return symbol_selected_color;
}

uint16_t get_symbol_unselected_color() {
    return symbol_unselected_color;
}

uint16_t get_symbol_bg_color() {
    return symbol_bg_color;
}

uint16_t get_bt_num_color() {
    return bt_num_color;
}

uint16_t get_bt_bg_color() {
    return bt_bg_color;
}

uint16_t get_frame_color() {
    return frame_color;
}

uint16_t get_frame_color_1() {
    return frame_color_1;
}

// Clamp function to ensure values stay within 0-255
int clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

// Function to darken RGB color
uint32_t darkenColor(uint32_t rgb, float percentage) {
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;

    // Extract red, green, and blue components
    uint32_t r = (rgb >> 16) & 0xFF;
    uint32_t g = (rgb >> 8)  & 0xFF;
    uint32_t b = rgb & 0xFF;

    // Darken each component
    r = clamp((uint32_t)(r * (1.0f - percentage)));
    g = clamp((uint32_t)(g * (1.0f - percentage)));
    b = clamp((uint32_t)(b * (1.0f - percentage)));

    // Recombine into a single int
    return (r << 16) | (g << 8) | b;
}

void display_write_wrapper(uint16_t x, uint16_t y, struct display_buffer_descriptor *buf_desc, uint8_t *buf) {
    display_write(display_dev, x, y, buf_desc, buf);
}

void init_display(void) {
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device %s not found. Aborting sample.", display_dev->name);
		return;
	}

    screen_width = 20;
    screen_height = 20;
    buf_screen_size = screen_width * screen_height * 2u;
	buf_screen_area = k_malloc(buf_screen_size);
    
	//fill_buffer_color(buf_screen_area, buf_screen_size, get_splash_bg_color());
}

void fill_buffer_color(uint8_t *buf, size_t buf_size, uint32_t color) {
	for (size_t idx = 0; idx < buf_size; idx += 2) {
		*(buf + idx + 0) = (color >> 8) & 0xFFu;
		*(buf + idx + 1) = (color >> 0) & 0xFFu;
	}
}

uint16_t swap_16_bit_color(uint16_t color) {
    return (color >> 8) | (color << 8);
}

void render_bitmap(uint16_t *scaled_bitmap, uint16_t bitmap[], uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t scale, uint16_t num_color, uint16_t bg_color) {
	struct display_buffer_descriptor buf_font_desc;

    uint16_t color;
    uint16_t pixel;
    uint16_t font_width_scaled = width * scale;
    uint16_t font_height_scaled = height * scale;
    uint16_t font_buf_size_scaled = font_width_scaled * font_height_scaled;
    uint16_t index = 0;
    for (uint16_t line = 0; line < height; line++) {
        for (uint16_t i = 0; i < scale; i++) {
            for (uint16_t column = 0; column < width; column++) {
                for (uint16_t j = 0; j < scale; j++) {
                    pixel = bitmap[(line*width) + column];
                    if (pixel == 1) {
                        color = num_color;
                    } else {
                        color = bg_color;
                    }
                    *(scaled_bitmap + index) = swap_16_bit_color(color);
                    index++;
                }
            }
        }
    }
	buf_font_desc.buf_size = font_buf_size_scaled;
	buf_font_desc.pitch = font_width_scaled;
	buf_font_desc.width = font_width_scaled;
	buf_font_desc.height = font_height_scaled;
    display_write(display_dev, x, y, &buf_font_desc, scaled_bitmap);
}

void print_bitmap_5x8(uint16_t *scaled_bitmap, Character c, uint16_t x, uint16_t y, uint16_t scale, uint16_t color, uint16_t bg_color) {
    uint8_t font_width = 5;
    uint8_t font_height = 8;
    
    if (c >= 0 && c < 10) {
        render_bitmap(scaled_bitmap, num_bitmaps_5x8[c], x, y, font_width, font_height, scale, color, bg_color);
        return;
    }
    switch (c) {
    case CHAR_F: render_bitmap(scaled_bitmap, f_bitmap_5x8, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_U: render_bitmap(scaled_bitmap, u_bitmap_5x8, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_L: render_bitmap(scaled_bitmap, l_bitmap_5x8, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_DASH: render_bitmap(scaled_bitmap, dash_bitmap_5x8, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_PERCENTAGE: render_bitmap(scaled_bitmap, percentage_bitmap_5x8, x, y, font_width, font_height, scale, color, bg_color); break;
    default: render_bitmap(scaled_bitmap, none_bitmap_5x8, x, y, font_width, font_height, scale, color, bg_color);
    }
}

void print_bitmap_5x7(uint16_t *scaled_bitmap, Character c, uint16_t x, uint16_t y, uint16_t scale, uint16_t color, uint16_t bg_color) {
    uint8_t font_width = 5;
    uint8_t font_height = 7;

    if (c >= 0 && c < 10) {
        render_bitmap(scaled_bitmap, num_bitmaps_5x7[c], x, y, font_width, font_height, scale, color, bg_color);
        return;
    }
    switch (c) {
    case CHAR_B: render_bitmap(scaled_bitmap, b_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_E: render_bitmap(scaled_bitmap, e_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_H: render_bitmap(scaled_bitmap, h_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_L: render_bitmap(scaled_bitmap, l_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_M: render_bitmap(scaled_bitmap, m_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_N: render_bitmap(scaled_bitmap, n_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_P: render_bitmap(scaled_bitmap, p_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_S: render_bitmap(scaled_bitmap, s_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_T: render_bitmap(scaled_bitmap, t_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_W: render_bitmap(scaled_bitmap, w_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_COLON: render_bitmap(scaled_bitmap, colon_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    case CHAR_EMPTY: render_bitmap(scaled_bitmap, empty_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color); break;
    default: render_bitmap(scaled_bitmap, none_bitmap_5x7, x, y, font_width, font_height, scale, color, bg_color);
    }
}

void print_bitmap_3x5(uint16_t *scaled_bitmap, Character c, uint16_t x, uint16_t y, uint16_t scale, uint16_t color, uint16_t bg_color) {
    if (c >= 0 && c < 10) {
        render_bitmap(scaled_bitmap, num_bitmaps_3x5[c], x, y, 3, 5, scale, color, bg_color);
        return;
    }
    switch (c) {
    case CHAR_P: render_bitmap(scaled_bitmap, p_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_O: render_bitmap(scaled_bitmap, o_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_C: render_bitmap(scaled_bitmap, c_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_B: render_bitmap(scaled_bitmap, b_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_Y: render_bitmap(scaled_bitmap, y_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_R: render_bitmap(scaled_bitmap, r_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_T: render_bitmap(scaled_bitmap, t_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_D: render_bitmap(scaled_bitmap, d_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_S: render_bitmap(scaled_bitmap, s_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_N: render_bitmap(scaled_bitmap, n_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_A: render_bitmap(scaled_bitmap, a_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_K: render_bitmap(scaled_bitmap, k_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_E: render_bitmap(scaled_bitmap, e_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_I: render_bitmap(scaled_bitmap, i_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    case CHAR_COLON: render_bitmap(scaled_bitmap, colon_letter_3x5, x, y, 3, 5, scale, color, bg_color); break;
    default: render_bitmap(scaled_bitmap, none_letter_3x5, x, y, 3, 5, scale, color, bg_color);
    }
}

void print_bitmap_3x6(uint16_t *scaled_bitmap, Character c, uint16_t x, uint16_t y, uint16_t scale, uint16_t color, uint16_t bg_color) {
    if (c >= 0 && c < 10) {
        render_bitmap(scaled_bitmap, num_bitmaps_3x6[c], x, y, 3, 6, scale, color, bg_color);
        return;
    }
    switch (c) {
    case CHAR_S: render_bitmap(scaled_bitmap, s_letter_3x6, x, y, 3, 6, scale, color, bg_color); break;
    case CHAR_N: render_bitmap(scaled_bitmap, n_letter_3x6, x, y, 3, 6, scale, color, bg_color); break;
    case CHAR_A: render_bitmap(scaled_bitmap, a_letter_3x6, x, y, 3, 6, scale, color, bg_color); break;
    case CHAR_K: render_bitmap(scaled_bitmap, k_letter_3x6, x, y, 3, 6, scale, color, bg_color); break;
    case CHAR_E: render_bitmap(scaled_bitmap, e_letter_3x6, x, y, 3, 6, scale, color, bg_color); break;
    case CHAR_I: render_bitmap(scaled_bitmap, i_letter_3x6, x, y, 3, 6, scale, color, bg_color); break;
    default: render_bitmap(scaled_bitmap, none_letter_3x6, x, y, 3, 6, scale, color, bg_color);
    }
}

void print_bitmap(uint16_t *scaled_bitmap, Character c, uint16_t x, uint16_t y, uint16_t scale, uint16_t color, uint16_t bg_color, FontSize font_size) {
    switch (font_size) {
        case FONT_SIZE_3x6: print_bitmap_3x6(scaled_bitmap, c, x, y, scale, color, bg_color); break;
        case FONT_SIZE_3x5: print_bitmap_3x5(scaled_bitmap, c, x, y, scale, color, bg_color); break;
        case FONT_SIZE_5x8: print_bitmap_5x8(scaled_bitmap, c, x, y, scale, color, bg_color); break;
        case FONT_SIZE_5x7: print_bitmap_5x7(scaled_bitmap, c, x, y, scale, color, bg_color); break;
    }
}

void print_line_horizontal(uint8_t *buf_frame, uint16_t start_x, uint16_t end_x, uint16_t start_y, uint16_t end_y, uint16_t scale, uint16_t color) {
    struct display_buffer_descriptor horizontal_line_desc;

    uint16_t horizontal_line_len = end_x - start_x + scale;

    horizontal_line_desc.buf_size = horizontal_line_len * scale;
	horizontal_line_desc.pitch = horizontal_line_len;
	horizontal_line_desc.width = horizontal_line_len;
	horizontal_line_desc.height = scale;

    fill_buffer_color(buf_frame, horizontal_line_desc.buf_size * 2u, color);
    display_write(display_dev, start_x, start_y, &horizontal_line_desc, buf_frame);
    display_write(display_dev, start_x, end_y, &horizontal_line_desc, buf_frame);
}

void print_line_vertical(uint8_t *buf_frame, uint16_t start_x, uint16_t end_x, uint16_t start_y, uint16_t end_y, uint16_t scale, uint16_t color) {
    struct display_buffer_descriptor vertical_line_desc;

    uint16_t vertical_line_len = end_y - start_y + scale;

    vertical_line_desc.buf_size = vertical_line_len * scale;
	vertical_line_desc.pitch = scale;
	vertical_line_desc.width = scale;
	vertical_line_desc.height = vertical_line_len;

    fill_buffer_color(buf_frame, vertical_line_desc.buf_size * 2u, color);
    display_write(display_dev, start_x, start_y, &vertical_line_desc, buf_frame);
    display_write(display_dev, end_x, start_y, &vertical_line_desc, buf_frame);
}

void print_rectangle(uint8_t *buf_frame, uint16_t start_x, uint16_t end_x, uint16_t start_y, uint16_t end_y, uint16_t color, uint16_t scale) {
    print_line_horizontal(buf_frame, start_x, end_x, start_y, end_y, scale, color);
    print_line_vertical(buf_frame, start_x, end_x, start_y, end_y, scale, color);
}

void render_filled_rectangle(uint8_t *buf_area, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    struct display_buffer_descriptor buf_desc_area;
    buf_desc_area.pitch = width;
	buf_desc_area.width = width;
	buf_desc_area.height = height;
	display_write_wrapper(x, y, &buf_desc_area, buf_area);
}

void clear_screen() {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            render_filled_rectangle(buf_screen_area, i * screen_width, j * screen_height, screen_width, screen_height);
        }
    }
}

void set_colors(uint32_t primary, uint32_t secondary, uint32_t background1, uint32_t background2, uint32_t color5, uint32_t color6) {
    set_splash_logo_color(primary);
    set_splash_created_by_color(background1);
    set_splash_bg_color(background2);
    
    // old snake version
    // set_snake_font_color(color2);
    // set_snake_num_color(color3);
    // set_snake_bg_color(color1);

    set_snake_default_color(primary);
    set_snake_board_color(background2);
    #ifdef CONFIG_CHECKERED_BOARD
        set_snake_board_1_color(background1);
    #else
        set_snake_board_1_color(background2);
    #endif

    set_food_color(secondary);
    set_snake_color_0(primary);
    set_snake_color_1(secondary);
    set_snake_color_2(background1);
    set_snake_color_3(secondary);
    set_snake_color_4(primary);
    set_snake_color_5(secondary);
    set_snake_color_6(background1);

    set_battery_num_color(primary);
    set_battery_percentage_color(background1);
    set_battery_bg_color(background2);

    set_symbol_selected_color(primary);
    set_symbol_unselected_color(background1);
    set_symbol_bg_color(background2);

    set_bt_num_color(background1);
    set_bt_bg_color(background2);

    set_frame_color(background1);
    set_frame_color_1(background1);

    fill_buffer_color(buf_screen_area, buf_screen_size, get_splash_bg_color());
}

void print_string(uint16_t *scaled_bitmap, Character str[], uint16_t x, uint16_t y, uint16_t scale, uint16_t color, uint16_t bg_color, FontSize font_size, uint16_t gap_pixels, uint8_t strLen) {
    uint16_t string_font_width_scaled = 0;
    if (font_size == FONT_SIZE_3x6 || font_size == FONT_SIZE_3x5) {
        string_font_width_scaled = 3 * scale;
    }
    if (font_size == FONT_SIZE_5x7 || font_size == FONT_SIZE_5x8) {
        string_font_width_scaled = 5 * scale;
    }
    if (string_font_width_scaled == 0) {
        return ;
    }

    for (uint8_t i = 0; i < strLen; i++) {
        Character c = str[i];
        uint16_t actual_x = x + (string_font_width_scaled * i) + (gap_pixels * i);
        print_bitmap(scaled_bitmap, c, actual_x, y, scale, color, bg_color, font_size);
    }
}