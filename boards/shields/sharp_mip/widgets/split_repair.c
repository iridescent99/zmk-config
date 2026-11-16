/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/addr.h>
#include <zmk/ble/profile.h>
#include "split_repair.h"
#include "split_repair_plan.h"

/* ZMK v0.3 loads settings and enables split transport in main(), after SYS_INIT.
 * Work here before keys/CCC are loaded or advertising/scanning can start. */
BUILD_ASSERT(CONFIG_BT_ID_MAX == 1, "Repair supports BT_ID_DEFAULT only");
BUILD_ASSERT(CONFIG_BT_MAX_PAIRED <= SOFLE_REPAIR_MAX_PEERS, "Unexpected bond capacity");
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
BUILD_ASSERT(CONFIG_ZMK_SPLIT_BLE_CENTRAL_PERIPHERALS == 1, "Expected one Sofle peripheral");
#else
BUILD_ASSERT(CONFIG_BT_MAX_PAIRED == 1, "Expected a single peripheral bond");
#endif

#define ATTEMPT_KEY "sofle_split_repair/attempt_v1"
static const char *result = "FIX ERR";

const char *sofle_split_repair_status(void) { return result; }

static int address_token(const bt_addr_le_t *addr, char out[SOFLE_REPAIR_TOKEN_SIZE]) {
    if (addr->type > 1) { return -EINVAL; }
    snprintf(out, SOFLE_REPAIR_TOKEN_SIZE, "%02x%02x%02x%02x%02x%02x%c",
             addr->a.val[5], addr->a.val[4], addr->a.val[3],
             addr->a.val[2], addr->a.val[1], addr->a.val[0], '0' + addr->type);
    return sofle_repair_token_valid(out) ? 0 : -EINVAL;
}

/* Zephyr 3.5 settings_load_subtree_direct discards backend callback errors.
 * Keep an explicit sticky error so a partial inventory can never authorize writes. */
static int record_error(struct sofle_repair_inventory *s, int err) {
    if (err) { s->read_error = err; }
    return err;
}

static int attempt_read(const char *key, size_t len, settings_read_cb read_cb,
                        void *cb_arg, void *param) {
    ARG_UNUSED(read_cb);
    ARG_UNUSED(cb_arg);
    if ((!key || !*key) && len) { *(bool *)param = true; }
    return 0;
}

static int ble_read(const char *key, size_t len, settings_read_cb read_cb,
                    void *cb_arg, void *param) {
    struct sofle_repair_inventory *s = param;
    const char *next;
    if (!len || !key) { return 0; }
    if (settings_name_steq(key, "profiles", &next) && next) {
        struct zmk_ble_profile profile;
        if (len != sizeof(profile) ||
            read_cb(cb_arg, &profile, sizeof(profile)) != (ssize_t)sizeof(profile)) {
            return record_error(s, -EINVAL);
        }
        if (!bt_addr_le_cmp(&profile.peer, BT_ADDR_LE_ANY) ||
            !bt_addr_le_cmp(&profile.peer, BT_ADDR_LE_NONE)) { return 0; }
        if (s->host_count == SOFLE_REPAIR_MAX_PEERS) { return record_error(s, -E2BIG); }
        return record_error(s, address_token(&profile.peer, s->hosts[s->host_count++]));
    }
    if (s->central && settings_name_steq(key, "peripheral_addresses", &next) && next) {
        bt_addr_le_t addr;
        if (strcmp(next, "0") || len != sizeof(addr) ||
            read_cb(cb_arg, &addr, sizeof(addr)) != (ssize_t)sizeof(addr)) {
            return record_error(s, -EINVAL);
        }
        return record_error(s, address_token(&addr, s->peripheral));
    }
    return 0;
}

static int bonds_read(const char *key, size_t len, settings_read_cb read_cb,
                      void *cb_arg, void *param) {
    ARG_UNUSED(read_cb);
    ARG_UNUSED(cb_arg);
    struct sofle_repair_inventory *s = param;
    if (!len) { return 0; }
    /* Refuse unfamiliar formats/identity suffixes; never read key material. */
    if (!sofle_repair_token_valid(key)) { return record_error(s, -EINVAL); }
    if (s->bond_count == SOFLE_REPAIR_MAX_PEERS) { return record_error(s, -E2BIG); }
    memcpy(s->bonds[s->bond_count++], key, SOFLE_REPAIR_TOKEN_SIZE);
    return 0;
}

static int repair_init(void) {
    if (settings_subsys_init()) { return 0; }
    bool attempted = false;
    if (settings_load_subtree_direct(ATTEMPT_KEY, attempt_read, &attempted)) { return 0; }
    if (attempted) { result = "FIX USED"; return 0; }

    struct sofle_repair_inventory inventory = {
        .central = IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL),
    };
    struct sofle_repair_plan plan;
    if (settings_load_subtree_direct("ble", ble_read, &inventory) ||
        settings_load_subtree_direct("bt/keys", bonds_read, &inventory) ||
        sofle_repair_make_plan(&inventory, &plan)) {
        result = "FIX STOP";
        return 0;
    }

    /* Commit the guard BEFORE deleting anything. An interrupted/failed attempt
     * must never clear a newly established bond on a later reboot. */
    const uint8_t marker = 1;
    if (settings_save_one(ATTEMPT_KEY, &marker, sizeof(marker))) { return 0; }
    for (size_t i = 0; i < plan.count; i++) {
        int err = settings_delete(plan.paths[i]);
        if (err && err != -ENOENT) { return 0; }
    }
    result = plan.count ? "FIX DONE" : "FIX NONE";
    return 0;
}

SYS_INIT(repair_init, APPLICATION, 60);
