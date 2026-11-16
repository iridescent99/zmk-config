/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT keebart_sofle_gradient
#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/init.h>
#include <zephyr/sys/util.h>
#include "gradient_color.h"
#include "physical_gradient.h"

#define PIXELS DT_INST_PROP(0, chain_length)
#define BACKEND DT_INST_PHANDLE(0, strip)
BUILD_ASSERT(PIXELS == DT_PROP(BACKEND, chain_length), "Gradient/physical strip size mismatch");
BUILD_ASSERT(CONFIG_LED_STRIP_INIT_PRIORITY < 99, "Initialize physical strip before gradient");
static const struct device *const backend = DEVICE_DT_GET(BACKEND);

static int update(const struct device *dev, struct led_rgb *pixels, size_t count) {
    ARG_UNUSED(dev);
    if (count > PIXELS || (!pixels && count)) { return -EINVAL; }
    for (size_t i = 0; i < count; i++) {
#if IS_ENABLED(CONFIG_SOFLE_PHYSICAL_GRADIENT)
        struct gradient_rgb rgb = sofle_physical_gradient(
            (struct gradient_rgb){pixels[i].r, pixels[i].g, pixels[i].b},
            IS_ENABLED(CONFIG_BOARD_SOFLE_CHOC_PRO_RIGHT), i);
#else
        struct gradient_rgb rgb = sofle_gradient_color(
            (struct gradient_rgb){pixels[i].r, pixels[i].g, pixels[i].b}, i, PIXELS);
#endif
        pixels[i].r = rgb.r; pixels[i].g = rgb.g; pixels[i].b = rgb.b;
    }
    return led_strip_update_rgb(backend, pixels, count);
}

static int init(const struct device *dev) {
    ARG_UNUSED(dev);
    return device_is_ready(backend) ? 0 : -ENODEV;
}

static int channels(const struct device *dev, uint8_t *values, size_t count) {
    ARG_UNUSED(dev); ARG_UNUSED(values); ARG_UNUSED(count);
    return -ENOTSUP;
}
static const struct led_strip_driver_api api = {.update_rgb = update, .update_channels = channels};
DEVICE_DT_INST_DEFINE(0, init, NULL, NULL, NULL, POST_KERNEL, 99, &api);
