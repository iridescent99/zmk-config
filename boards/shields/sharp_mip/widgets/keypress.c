/* SPDX-License-Identifier: MIT */
#include <zephyr/kernel.h>
#include <stdio.h>
#include <string.h>
#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>
#include "status.h"
#include "keypress_model.h"

#define FRAME_MS 25

static struct zmk_widget_status *display_widget;
static struct k_spinlock model_lock;
static struct keypress_model model;
static bool ready;
/* Only the current rendered label is cached, to avoid repeated identical draws. */
static char shown_label[KEYPRESS_LABEL_SIZE];
static uint8_t shown_modifiers;

static void text(lv_obj_t *canvas, int y, const lv_font_t *font, const char *label) {
    lv_draw_label_dsc_t dsc;
    init_label_dsc(&dsc, LVGL_FOREGROUND, font, LV_TEXT_ALIGN_CENTER);
    lv_canvas_draw_text(canvas, 1, y, 66, &dsc, label);
}

static const char *layer_label(const struct status_state *s) {
    const char *name = s->layer_label;
    if (!name || !*name) { return NULL; }
    /* Display-only aliases for the stock Sofle map; Studio/custom names survive. */
    if (!strcmp(name, "default")) { return "QWERTY"; }
    if (!strcmp(name, "lower")) { return "NUM/SYM"; }
    if (!strcmp(name, "raise")) { return "NAV"; }
    if (!strcmp(name, "adjust")) { return "ADJUST"; }
    return name;
}

static void draw_status(void) {
    struct zmk_widget_status *w = display_widget;
    struct status_state *s = &w->state;
    lv_obj_t *canvas = lv_obj_get_child(w->obj, 0);
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    draw_battery(canvas, s);
    char battery[6];
    snprintf(battery, sizeof(battery), "%u%%", s->battery);
    lv_draw_label_dsc_t dsc;
    init_label_dsc(&dsc, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT);
    lv_canvas_draw_text(canvas, 34, 4, 34, &dsc, battery);
    char connection[20];
    if (s->selected_endpoint.transport == ZMK_TRANSPORT_USB) {
        snprintf(connection, sizeof(connection), "USB%s", zmk_usb_is_hid_ready() ? "" : " -");
    } else {
        snprintf(connection, sizeof(connection), "BT%d %s", s->active_profile_index + 1,
                 s->active_profile_connected ? "OK" : s->active_profile_bonded ? "OFF" : "NEW");
    }
    text(canvas, 25, &lv_font_unscii_8, connection);
    char fallback[10];
    snprintf(fallback, sizeof(fallback), "LAYER %u", s->layer_index);
    const char *name = layer_label(s);
    /* Bound custom Studio names to the available width without scrolling. */
    char short_name[9];
    snprintf(short_name, sizeof(short_name), "%.8s", name ? name : fallback);
    text(canvas, 49, &lv_font_unscii_8, short_name);
    rotate_canvas(canvas, w->cbuf);
}

static void draw_key(const char *label) {
    struct zmk_widget_status *w = display_widget;
    lv_obj_t *canvas = lv_obj_get_child(w->obj, 1);
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    const lv_font_t *font = &lv_font_montserrat_48;
    if (lv_txt_get_width(label, strlen(label), font, 0, LV_TEXT_FLAG_NONE) > 64) {
        font = &lv_font_montserrat_26;
    }
    if (lv_txt_get_width(label, strlen(label), font, 0, LV_TEXT_FLAG_NONE) > 64) {
        font = &lv_font_montserrat_16;
    }
    text(canvas, (68 - font->line_height) / 2, font, label);
    rotate_canvas(canvas, w->cbuf2);
}

static void draw_modifiers(uint8_t mods) {
    struct zmk_widget_status *w = display_widget;
    lv_obj_t *canvas = lv_obj_get_child(w->obj, 2);
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    char line[9];
    snprintf(line, sizeof(line), "%s %s", mods & 0x11 ? "CTRL" : "    ",
             mods & 0x44 ? "ALT" : "   ");
    text(canvas, 2, &lv_font_unscii_8, line);
    snprintf(line, sizeof(line), "%s %s", mods & 0x88 ? "GUI" : "   ",
             mods & 0x22 ? "SHFT" : "    ");
    text(canvas, 13, &lv_font_unscii_8, line);
    rotate_canvas(canvas, w->cbuf3);
}

static void render(struct k_work *work) {
    ARG_UNUSED(work);
    struct keypress_model snapshot;
    k_spinlock_key_t lock = k_spin_lock(&model_lock);
    snapshot = model;
    k_spin_unlock(&model_lock, lock);
    char label[KEYPRESS_LABEL_SIZE];
    keypress_label(&snapshot, label);
    if (memcmp(label, shown_label, sizeof(label))) {
        draw_key(label);
        memcpy(shown_label, label, sizeof(label));
    }
    uint8_t mods = keypress_modifiers(&snapshot);
    if (mods != shown_modifiers) {
        draw_modifiers(mods);
        shown_modifiers = mods;
    }
}
K_WORK_DELAYABLE_DEFINE(render_work, render);

static void expire(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(expire_work, expire);

static void expire(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t now = k_uptime_get();
    k_spinlock_key_t lock = k_spin_lock(&model_lock);
    bool changed = keypress_expire(&model, now);
    int64_t remaining = model.visible && !model.down ? model.expires_at - now : 0;
    k_spin_unlock(&model_lock, lock);
    if (changed) { k_work_schedule_for_queue(zmk_display_work_q(), &render_work, K_NO_WAIT); }
    /* A stale timeout cannot clear a newer press/release. */
    if (remaining > 0) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &expire_work, K_MSEC(remaining));
    }
}

static int key_event(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (!ev) { return ZMK_EV_EVENT_BUBBLE; }
    int64_t now = k_uptime_get();
    k_spinlock_key_t lock = k_spin_lock(&model_lock);
    if (!ready) {
        k_spin_unlock(&model_lock, lock);
        return ZMK_EV_EVENT_BUBBLE;
    }
    keypress_event(&model, ev->usage_page, ev->keycode, ev->implicit_modifiers,
                   ev->explicit_modifiers, ev->state, now);
    int64_t remaining = model.visible && !model.down ? model.expires_at - now : 0;
    k_spin_unlock(&model_lock, lock);
    /* schedule (not reschedule) ensures a stream of events cannot postpone the
     * pending frame forever. No LVGL, allocation, logging, I/O or waits here. */
    k_work_schedule_for_queue(zmk_display_work_q(), &render_work, K_MSEC(FRAME_MS));
    if (remaining > 0) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &expire_work, K_MSEC(remaining));
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(sofle_keypress, key_event);
ZMK_SUBSCRIPTION(sofle_keypress, zmk_keycode_state_changed);

/* Existing ZMK widget helpers serialize status changes onto the same display
 * queue as rendering and LVGL's tick handler. */
static struct status_state status_get(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    zmk_keymap_layer_index_t layer = zmk_keymap_highest_layer_active();
    return (struct status_state){
        .battery = zmk_battery_state_of_charge(),
        .charging = zmk_usb_is_powered(),
        .selected_endpoint = zmk_endpoints_selected(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
        .layer_index = layer,
        .layer_label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(layer)),
    };
}
static void status_update(struct status_state state) {
    display_widget->state = state;
    draw_status();
}
ZMK_DISPLAY_WIDGET_LISTENER(sofle_keypress_status, struct status_state, status_update, status_get)
ZMK_SUBSCRIPTION(sofle_keypress_status, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(sofle_keypress_status, zmk_usb_conn_state_changed);
ZMK_SUBSCRIPTION(sofle_keypress_status, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(sofle_keypress_status, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(sofle_keypress_status, zmk_layer_state_changed);

int zmk_widget_status_init(struct zmk_widget_status *w, lv_obj_t *parent) {
    display_widget = w;
    w->obj = lv_obj_create(parent);
    lv_obj_remove_style_all(w->obj);
    lv_obj_set_size(w->obj, 160, 68);
    lv_obj_clear_flag(w->obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_color_t *buffers[] = {w->cbuf, w->cbuf2, w->cbuf3};
#if IS_ENABLED(CONFIG_SHARP_MIP_ROTATE_180)
    const int positions[] = {0, 68, 136};
#else
    const int positions[] = {92, 24, -44};
#endif
    for (unsigned i = 0; i < 3; i++) {
        lv_obj_t *canvas = lv_canvas_create(w->obj);
        lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, positions[i], 0);
        lv_canvas_set_buffer(canvas, buffers[i], CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);
    }
    sofle_keypress_status_init();
    draw_key("");
    draw_modifiers(0);
    k_spinlock_key_t lock = k_spin_lock(&model_lock);
    ready = true;
    k_spin_unlock(&model_lock, lock);
    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *w) { return w->obj; }
