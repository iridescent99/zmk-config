#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <lvgl.h>
#define CONFIG_ZMK_SPLIT 1
#define CONFIG_ZMK_SPLIT_ROLE_CENTRAL 1
#define CONFIG_SHARP_MIP_WIDGET_INVERTED 0
#define CONFIG_SHARP_MIP_ROTATE_180 1
#define CONFIG_SOFLE_SPLIT_REPAIR 0
#define IS_ENABLED(x) (x)
#define ARG_UNUSED(x) ((void)(x))
typedef struct { int unused; } sys_snode_t;
struct k_spinlock { int unused; };
typedef int k_spinlock_key_t;
static inline int k_spin_lock(struct k_spinlock *lock) { (void)lock; return 0; }
static inline void k_spin_unlock(struct k_spinlock *lock, int key) { (void)lock; (void)key; }
struct k_work { void (*fn)(struct k_work *); };
struct k_work_delayable { struct k_work work; int64_t due; bool pending; };
struct k_work_q { int unused; };
#define K_WORK_DELAYABLE_DEFINE(name, fn) \
    struct k_work_delayable name = {.work = {fn}}
#define K_NO_WAIT 0
#define K_MSEC(ms) (ms)
int64_t k_uptime_get(void);
struct k_work_q *zmk_display_work_q(void);
int k_work_schedule_for_queue(struct k_work_q *, struct k_work_delayable *, int64_t);
int k_work_reschedule_for_queue(struct k_work_q *, struct k_work_delayable *, int64_t);
typedef struct {
    uint16_t usage_page;
    uint32_t keycode;
    uint8_t implicit_modifiers, explicit_modifiers;
    bool state;
} zmk_event_t;
#define zmk_keycode_state_changed key_event_struct
struct key_event_struct {
    uint16_t usage_page;
    uint32_t keycode;
    uint8_t implicit_modifiers, explicit_modifiers;
    bool state;
};
static inline const struct key_event_struct *as_zmk_keycode_state_changed(const zmk_event_t *ev) {
    return (const struct key_event_struct *)ev;
}
#define ZMK_EV_EVENT_BUBBLE 0
#define ZMK_LISTENER(name, cb) _Static_assert(1, "Listener")
#define ZMK_SUBSCRIPTION(name, event) _Static_assert(1, "Subscription")
#define ZMK_DISPLAY_WIDGET_LISTENER(name, type, cb, get) \
    static void name##_init(void) { cb(get(NULL)); }
#define ZMK_TRANSPORT_USB 0
#define ZMK_TRANSPORT_BLE 1
struct zmk_endpoint_instance { int transport; };
static inline struct zmk_endpoint_instance zmk_endpoints_selected(void) {
    return (struct zmk_endpoint_instance){ZMK_TRANSPORT_USB};
}
static inline int zmk_battery_state_of_charge(void) { return 80; }
static inline bool zmk_usb_is_powered(void) { return false; }
static inline bool zmk_usb_is_hid_ready(void) { return true; }
static inline int zmk_ble_active_profile_index(void) { return 0; }
static inline bool zmk_ble_active_profile_is_connected(void) { return false; }
static inline bool zmk_ble_active_profile_is_open(void) { return false; }
typedef int zmk_keymap_layer_index_t;
extern int test_layer_index;
extern const char *test_layer_name;
static inline int zmk_keymap_highest_layer_active(void) { return test_layer_index; }
static inline int zmk_keymap_layer_index_to_id(int i) { return i; }
static inline const char *zmk_keymap_layer_name(int i) { (void)i; return test_layer_name; }
int k_work_cancel_delayable(struct k_work_delayable *);
enum zmk_activity_state { ZMK_ACTIVITY_ACTIVE, ZMK_ACTIVITY_IDLE, ZMK_ACTIVITY_SLEEP };
enum zmk_activity_state zmk_activity_get_state(void);
