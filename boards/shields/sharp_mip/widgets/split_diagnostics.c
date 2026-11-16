/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/sensor_event.h>
#include <zmk/split/bluetooth/uuid.h>
#include "split_diagnostics.h"
#include "util.h"

/* Activity timestamps only: no key positions, usages, text, or event history. */
static atomic_t key_tick = ATOMIC_INIT(-1001);
static atomic_t sensor_tick = ATOMIC_INIT(-1001);
static atomic_t security_error;
static char lines[3][9];
static void (*redraw_status)(void);
static const struct bt_gatt_attr *position_attr;

static int activity(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *pos = as_zmk_position_state_changed(eh);
    if (pos && pos->state) {
        atomic_set(&key_tick, (atomic_val_t)k_uptime_get_32());
    }
    if (as_zmk_sensor_event(eh)) {
        atomic_set(&sensor_tick, (atomic_val_t)k_uptime_get_32());
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(sofle_split_diag, activity);
ZMK_SUBSCRIPTION(sofle_split_diag, zmk_position_state_changed);
ZMK_SUBSCRIPTION(sofle_split_diag, zmk_sensor_event);

static void security_changed(struct bt_conn *conn, bt_security_t level,
                             enum bt_security_err err) {
    ARG_UNUSED(conn);
    ARG_UNUSED(level);
    atomic_set(&security_error, err);
}
BT_CONN_CB_DEFINE(sofle_split_diag_conn) = {.security_changed = security_changed};

static uint8_t find_position_attr(const struct bt_gatt_attr *attr, uint16_t handle,
                                  void *user_data) {
    ARG_UNUSED(handle);
    ARG_UNUSED(user_data);
    if (!bt_uuid_cmp(attr->uuid, BT_UUID_DECLARE_128(ZMK_SPLIT_BT_CHAR_POSITION_STATE_UUID))) {
        position_attr = attr;
        return BT_GATT_ITER_STOP;
    }
    return BT_GATT_ITER_CONTINUE;
}

struct link_state { unsigned security; bool notifications; };

static void sample_connection(struct bt_conn *conn, void *user_data) {
    struct link_state *state = user_data;
    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) || info.state != BT_CONN_STATE_CONNECTED) { return; }
    state->security = bt_conn_get_security(conn);
    state->notifications = position_attr &&
        bt_gatt_is_subscribed(conn, position_attr, BT_GATT_CCC_NOTIFY);
}

static void sample(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(sample_work, sample);

static void sample(struct k_work *work) {
    ARG_UNUSED(work);
    struct link_state state = {0};
    bt_conn_foreach(BT_CONN_TYPE_LE, sample_connection, &state);
    uint32_t now = k_uptime_get_32();
    bool key = (uint32_t)(now - (uint32_t)atomic_get(&key_tick)) <= 1000;
    bool sensor = (uint32_t)(now - (uint32_t)atomic_get(&sensor_tick)) <= 1000;
    char next[3][9] = {{0}};
    snprintf(next[0], sizeof(next[0]), "D1 C%uN%u", state.security % 10,
             (unsigned)state.notifications);
    snprintf(next[1], sizeof(next[1]), "KEY %u", (unsigned)key);
    snprintf(next[2], sizeof(next[2]), "E%u ENC%u", (unsigned)atomic_get(&security_error) % 10,
             (unsigned)sensor);
    if (memcmp(lines, next, sizeof(lines))) {
        memcpy(lines, next, sizeof(lines));
        redraw_status();
    }
    /* Diagnostic build only; no repaint when these status values are unchanged. */
    k_work_reschedule_for_queue(zmk_display_work_q(), &sample_work, K_MSEC(250));
}

void sofle_split_diag_draw(lv_obj_t *canvas) {
    lv_draw_label_dsc_t dsc;
    init_label_dsc(&dsc, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_CENTER);
    for (unsigned i = 0; i < 3; i++) {
        lv_canvas_draw_text(canvas, 1, 22 + 13 * i, 66, &dsc, lines[i]);
    }
}

void sofle_split_diag_init(void (*redraw)(void)) {
    redraw_status = redraw;
    bt_gatt_foreach_attr(1, 0xffff, find_position_attr, NULL);
    k_work_schedule_for_queue(zmk_display_work_q(), &sample_work, K_NO_WAIT);
}
