/* Minimal display device boundary for compiling the pinned Zephyr monochrome
 * adapter. Colour conversion/packing/rounding/flush are the upstream C code. */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <lvgl.h>
#define BIT(n) (1U << (n))
#define PIXEL_FORMAT_MONO01 1
#define PIXEL_FORMAT_MONO10 2
#define SCREEN_INFO_EPD BIT(0)
#define SCREEN_INFO_DOUBLE_BUFFER BIT(1)
#define SCREEN_INFO_MONO_VTILED BIT(2)
#define SCREEN_INFO_MONO_MSB_FIRST BIT(3)
#define SCREEN_INFO_X_ALIGNMENT_WIDTH BIT(4)
struct device { int unused; };
struct display_buffer_descriptor { uint32_t buf_size; uint16_t width, pitch, height; };
struct display_capabilities {
    uint16_t x_resolution, y_resolution;
    unsigned current_pixel_format, screen_info;
};
struct lvgl_disp_data {
    const struct device *display_dev;
    struct display_capabilities cap;
    bool blanking_on;
};
int display_write(const struct device *, uint16_t, uint16_t,
                  const struct display_buffer_descriptor *, const void *);
static inline int display_blanking_on(const struct device *d) { (void)d; return 0; }
static inline int display_blanking_off(const struct device *d) { (void)d; return 0; }
void lvgl_flush_cb_mono(lv_disp_drv_t *, const lv_area_t *, lv_color_t *);
void lvgl_set_px_cb_mono(lv_disp_drv_t *, uint8_t *, lv_coord_t, lv_coord_t,
                         lv_coord_t, lv_color_t, lv_opa_t);
void lvgl_rounder_cb_mono(lv_disp_drv_t *, lv_area_t *);
