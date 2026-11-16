#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#define CONFIG_BT_ID_MAX 1
#define CONFIG_ZMK_SPLIT_BLE_CENTRAL_PERIPHERALS 1
#if CONFIG_ZMK_SPLIT_ROLE_CENTRAL
#define CONFIG_BT_MAX_PAIRED 6
#else
#define CONFIG_BT_MAX_PAIRED 1
#endif
#define IS_ENABLED(x) (x)
#define BUILD_ASSERT(test, message) _Static_assert(test, message)
#define ARG_UNUSED(x) ((void)(x))
#define SYS_INIT(fn, level, priority) _Static_assert(priority == 60, "Pre-main initialization")
typedef struct { uint8_t val[6]; } bt_addr_t;
typedef struct { uint8_t type; bt_addr_t a; } bt_addr_le_t;
static const bt_addr_le_t any_addr = {0};
static const bt_addr_le_t none_addr = {.type = 0xff, .a = {.val = {255,255,255,255,255,255}}};
#define BT_ADDR_LE_ANY (&any_addr)
#define BT_ADDR_LE_NONE (&none_addr)
static inline int bt_addr_le_cmp(const bt_addr_le_t *a, const bt_addr_le_t *b) {
    return memcmp(a, b, sizeof(*a));
}
struct zmk_ble_profile { char name[15]; bt_addr_le_t peer; };
typedef ssize_t (*settings_read_cb)(void *, void *, size_t);
typedef int (*settings_load_direct_cb)(const char *, size_t, settings_read_cb, void *, void *);
int settings_subsys_init(void);
int settings_load_subtree_direct(const char *, settings_load_direct_cb, void *);
int settings_save_one(const char *, const void *, size_t);
int settings_delete(const char *);
bool settings_name_steq(const char *, const char *, const char **);
