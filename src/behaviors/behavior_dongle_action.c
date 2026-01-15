#define DT_DRV_COMPAT zmk_behavior_dongle_action

// Dependencies
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/events/dongle_action_event.h>

#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_behavior_dongle_action_binding_pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    raise_zmk_dongle_actioned((struct zmk_dongle_actioned){
        .pressed = true,
        .timestamp = k_uptime_get()}
    );
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_behavior_dongle_action_binding_released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    raise_zmk_dongle_actioned((struct zmk_dongle_actioned){
        .pressed = false,
        .timestamp = k_uptime_get()});
    return ZMK_BEHAVIOR_OPAQUE;
}

// API Structure
static const struct behavior_driver_api behavior_dongle_action_driver_api = {
    .binding_pressed = on_behavior_dongle_action_binding_pressed,
    .binding_released = on_behavior_dongle_action_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0,                                                    // Instance Number (Equal to 0 for behaviors that don't require multiple instances,
                                                                              //                  Equal to n for behaviors that do make use of multiple instances)
                        NULL, NULL,                           // Initialization Function, Power Management Device Pointer (Both Optional)
                        NULL, NULL,       // Behavior Data Pointer, Behavior Configuration Pointer (Both Optional)
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,     // Initialization Level, Device Priority
                        &behavior_dongle_action_driver_api);                         // API Structure

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */