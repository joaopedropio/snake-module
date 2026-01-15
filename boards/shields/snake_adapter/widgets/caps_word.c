#include "caps_word.h"

#include <zephyr/kernel.h>
#include <zmk/display.h>
#include <zmk/events/caps_word_state_changed.h>
#include <zmk/event_manager.h>
#include "helpers/display.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);


static bool caps_word_widget_running = false;
static bool caps_word_widget_initialized = false;

static uint16_t caps_word_font_scale = 2;
static uint16_t caps_word_font_width = 15;
static uint16_t caps_word_font_height = 15;
static uint16_t *scaled_bitmap_caps_word_font;

Slot caps_word_slot;
static uint16_t caps_word_x = 8;
static uint16_t caps_word_y = 20;

static struct caps_word_indicator_state caps_word_state;

struct caps_word_indicator_state {
    bool active;
};

static const uint16_t caps_word_bitmap[] = {
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
};

void print_caps_word() {
    if (caps_word_state.active) {
        render_bitmap(scaled_bitmap_caps_word_font, caps_word_bitmap, caps_word_x + 40, caps_word_y, caps_word_font_width, caps_word_font_height, caps_word_font_scale, get_caps_word_selected_color(), get_caps_word_bg_color());
    } else {
        render_bitmap(scaled_bitmap_caps_word_font, caps_word_bitmap, caps_word_x + 40, caps_word_y, caps_word_font_width, caps_word_font_height, caps_word_font_scale, get_caps_word_unselected_color(), get_caps_word_bg_color());
    }
}

void caps_word_indicator_update_cb(struct caps_word_indicator_state state) {
    caps_word_state = state;
    if (caps_word_widget_initialized && caps_word_widget_running) {
        print_caps_word();
    }
}

static struct caps_word_indicator_state caps_word_indicator_get_state(const zmk_event_t *eh) {
    const struct zmk_caps_word_state_changed *ev = as_zmk_caps_word_state_changed(eh);
    return (struct caps_word_indicator_state){
        .active = ev->active,
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_caps_word_indicator, struct caps_word_indicator_state,
                            caps_word_indicator_update_cb, caps_word_indicator_get_state)
ZMK_SUBSCRIPTION(widget_caps_word_indicator, zmk_caps_word_state_changed);

void zmk_widget_caps_word_init() {
    uint16_t caps_word_font_size = (caps_word_font_width * caps_word_font_scale) * (caps_word_font_height * caps_word_font_scale);
    scaled_bitmap_caps_word_font = k_malloc(caps_word_font_size * 2 * sizeof(uint16_t));

    caps_word_slot = get_slot_by_name(SLOT_NAME_CAPS_WORD);
    caps_word_x += caps_word_slot.x;
    caps_word_y += caps_word_slot.y;

    widget_caps_word_indicator_init();
    
    caps_word_widget_initialized = true;
}

void start_caps_word_status() {
    caps_word_widget_running = true;
}

void stop_caps_word_status() {
    caps_word_widget_running = false;
}