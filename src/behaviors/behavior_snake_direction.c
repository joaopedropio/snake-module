#define DT_DRV_COMPAT zmk_behavior_snake_direction

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk_dongle_events/snake_direction_event.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_binding_pressed(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    uint8_t dir = binding->param1;  /* 0=UP, 1=RIGHT, 2=DOWN, 3=LEFT */
    raise_zmk_snake_direction(
        (struct zmk_snake_direction){ .direction = dir });
    return ZMK_BEHAVIOR_OPAQUE;     /* swallow — no HID report to host */
}

static int on_binding_released(struct zmk_behavior_binding *binding,
                               struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;     /* nothing on release */
}

static const struct behavior_driver_api behavior_snake_direction_driver_api = {
    .binding_pressed  = on_binding_pressed,
    .binding_released = on_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0,
                        NULL, NULL,
                        NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_snake_direction_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
