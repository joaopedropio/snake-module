#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_rotate_init, CONFIG_DISPLAY_LOG_LEVEL);

/*
 * The ST7789V panel is physically mounted at 270° relative to the
 * desired "normal" viewing orientation.  This SYS_INIT corrects for
 * that by writing MADCTL once at boot.  User-selectable rotation
 * (CONFIG_ROTATE_DISPLAY) is handled independently by the software
 * coordinate-transform layer in display.c / configuration.c.
 */
int disp_set_orientation(void)
{
	const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display)) {
		LOG_ERR("Display device not ready");
		return -EIO;
	}

	int ret = display_set_orientation(display, DISPLAY_ORIENTATION_ROTATED_270);
	if (ret < 0) {
		LOG_ERR("Failed to set base display orientation (%d)", ret);
		return ret;
	}

	LOG_INF("Display base orientation set to 270° (panel mount correction)");
	return 0;
}

SYS_INIT(disp_set_orientation, APPLICATION, 60);