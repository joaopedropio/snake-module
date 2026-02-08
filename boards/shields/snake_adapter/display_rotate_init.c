#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_rotate_init, CONFIG_DISPLAY_LOG_LEVEL);

int disp_set_orientation(void)
{
	const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display)) {
		LOG_ERR("Display device not ready");
		return -EIO;
	}

	enum display_orientation orient;

	switch (CONFIG_ROTATE_DISPLAY) {
	case 90:
		orient = DISPLAY_ORIENTATION_ROTATED_90;
		break;
	case 180:
		orient = DISPLAY_ORIENTATION_ROTATED_180;
		break;
	case 270:
		orient = DISPLAY_ORIENTATION_ROTATED_270;
		break;
	default:
		/* 0 or any other value → keep the panel's native orientation */
		return 0;
	}

	int ret = display_set_orientation(display, orient);
	if (ret < 0) {
		LOG_ERR("Failed to set display orientation to %d° (%d)",
			CONFIG_ROTATE_DISPLAY, ret);
		return ret;
	}

	LOG_INF("Display hardware orientation set to %d°", CONFIG_ROTATE_DISPLAY);
	return 0;
}

SYS_INIT(disp_set_orientation, APPLICATION, 60);