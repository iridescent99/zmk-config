/* SPDX-License-Identifier: MIT */
#include <zephyr/kernel.h>
#include <zmk/activity.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include "right_animation.h"
#include "util.h"

#define ORBIT_FRAME_MS 250
#define ORBIT_FRAMES 32
static lv_obj_t *orbit_canvas;
static lv_color_t orbit_buffer[CANVAS_SIZE * CANVAS_SIZE];
static bool orbit_active;
static uint8_t orbit_frame;
/* Eight-second orbit; integer coordinates avoid runtime trigonometry. */
static const lv_point_t moon_positions[ORBIT_FRAMES] = {
    {61,34},{60,39},{59,44},{56,49},{53,53},{49,56},{44,59},{39,60},
    {34,61},{29,60},{24,59},{19,56},{15,53},{12,49},{9,44},{8,39},
    {7,34},{8,29},{9,24},{12,19},{15,15},{19,12},{24,9},{29,8},
    {34,7},{39,8},{44,9},{49,12},{53,15},{56,19},{59,24},{60,29}
};

static void circle(int x, int y, int radius, lv_color_t color) {
    lv_draw_rect_dsc_t dsc;
    init_rect_dsc(&dsc, color);
    dsc.radius = LV_RADIUS_CIRCLE;
    lv_canvas_draw_rect(orbit_canvas, x-radius, y-radius, radius*2+1, radius*2+1, &dsc);
}

static void draw_orbit(void) {
    lv_canvas_fill_bg(orbit_canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    /* Fixed stars plus a slow twinkle, independent of typing. */
    circle(9, 9, 1, LVGL_FOREGROUND);
    circle(57, 57, 1, LVGL_FOREGROUND);
    circle(8, 58, 0, LVGL_FOREGROUND);
    circle(58, 9, (orbit_frame / 8) & 1, LVGL_FOREGROUND);

    circle(34, 34, 13, LVGL_FOREGROUND);
    circle(34, 34, 11, LVGL_BACKGROUND);
    /* A diagonal planetary ring. */
    const lv_point_t ring[] = {{16,43},{13,42},{14,39},{20,34},{29,29},
                               {39,25},{49,24},{54,26},{53,29},{47,34},
                               {38,39},{28,43},{20,44},{16,43}};
    lv_draw_line_dsc_t line;
    init_line_dsc(&line, LVGL_FOREGROUND, 1);
    lv_canvas_draw_line(orbit_canvas, ring, sizeof(ring)/sizeof(*ring), &line);
    circle(31, 30, 1, LVGL_FOREGROUND);
    circle(38, 37, 2, LVGL_FOREGROUND);

    lv_point_t moon = moon_positions[orbit_frame];
    circle(moon.x, moon.y, 4, LVGL_BACKGROUND);
    circle(moon.x, moon.y, 3, LVGL_FOREGROUND);
    circle(moon.x+1, moon.y-1, 1, LVGL_BACKGROUND);
    rotate_canvas(orbit_canvas, orbit_buffer);
}

static void orbit_tick(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(orbit_work, orbit_tick);
static void orbit_tick(struct k_work *work) {
    ARG_UNUSED(work);
    if (!orbit_active) { return; }
    orbit_frame = (orbit_frame + 1) % ORBIT_FRAMES;
    draw_orbit();
    k_work_reschedule_for_queue(zmk_display_work_q(), &orbit_work, K_MSEC(ORBIT_FRAME_MS));
}

static bool activity_get(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    return zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE;
}
static void activity_update(bool active) {
    if (active == orbit_active) { return; }
    orbit_active = active;
    if (active) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &orbit_work, K_MSEC(ORBIT_FRAME_MS));
    } else {
        k_work_cancel_delayable(&orbit_work);
    }
}
ZMK_DISPLAY_WIDGET_LISTENER(sofle_orbit_activity, bool, activity_update, activity_get)
ZMK_SUBSCRIPTION(sofle_orbit_activity, zmk_activity_state_changed);

void sofle_right_animation_init(lv_obj_t *parent) {
    orbit_canvas = lv_canvas_create(parent);
#if IS_ENABLED(CONFIG_SHARP_MIP_ROTATE_180)
    lv_obj_align(orbit_canvas, LV_ALIGN_TOP_LEFT, 68, 0);
#else
    lv_obj_align(orbit_canvas, LV_ALIGN_TOP_LEFT, 24, 0);
#endif
    lv_canvas_set_buffer(orbit_canvas, orbit_buffer, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);
    draw_orbit();
    sofle_orbit_activity_init();
}
