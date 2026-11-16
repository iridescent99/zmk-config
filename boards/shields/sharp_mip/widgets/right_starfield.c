/* SPDX-License-Identifier: MIT */
#include <zephyr/kernel.h>
#include <zmk/activity.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include "right_starfield.h"
#include "starfield_scene.h"

_Static_assert(LV_COLOR_DEPTH == 1 && sizeof(lv_color_t) == 1,
               "Starfield canvas requires the Sharp monochrome LVGL configuration");
static lv_obj_t *starfield_canvas;
static lv_color_t starfield_buffer[STARFIELD_WIDTH * STARFIELD_HEIGHT];
static bool starfield_active;
static uint32_t starfield_frame;
static void draw_starfield(void) {
    /* The scene is a white-star mask (1=star). The pinned Zephyr MONO01
     * adapter sends LVGL 0 as a set bit, while the Sharp LS0xx panel shows
     * set bits as white. Compensate here, not globally: the status header
     * and working left display retain their existing polarity. */
    sofle_starfield_render((uint8_t *)starfield_buffer, starfield_frame,
                          IS_ENABLED(CONFIG_SHARP_MIP_ROTATE_180));
    for (size_t i = 0; i < STARFIELD_WIDTH * STARFIELD_HEIGHT; i++) {
        starfield_buffer[i].full ^= 1U;
    }
    /* One invalidation per complete frame, on the dedicated display queue. */
    lv_obj_invalidate(starfield_canvas);
}
static void starfield_tick(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(starfield_work, starfield_tick);
static void starfield_tick(struct k_work *work) {
    ARG_UNUSED(work);
    if (!starfield_active) { return; }
    starfield_frame++;
    draw_starfield();
    k_work_reschedule_for_queue(zmk_display_work_q(), &starfield_work,
                                K_MSEC(STARFIELD_FRAME_MS));
}
static bool activity_get(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    return zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE;
}
static void activity_update(bool active) {
    if (active == starfield_active) { return; }
    starfield_active = active;
    if (active) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &starfield_work,
                                    K_MSEC(STARFIELD_FRAME_MS));
    } else {
        k_work_cancel_delayable(&starfield_work);
    }
}
ZMK_DISPLAY_WIDGET_LISTENER(sofle_starfield_activity, bool, activity_update, activity_get)
ZMK_SUBSCRIPTION(sofle_starfield_activity, zmk_activity_state_changed);
void sofle_right_starfield_init(lv_obj_t *parent) {
    starfield_canvas = lv_canvas_create(parent);
    lv_obj_align(starfield_canvas, LV_ALIGN_TOP_LEFT,
                 IS_ENABLED(CONFIG_SHARP_MIP_ROTATE_180) ? 20 : 0, 0);
    lv_canvas_set_buffer(starfield_canvas, starfield_buffer,
                         STARFIELD_WIDTH, STARFIELD_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    draw_starfield();
    sofle_starfield_activity_init();
}
