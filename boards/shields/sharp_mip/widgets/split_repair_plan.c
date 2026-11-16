/* SPDX-License-Identifier: MIT */
#include "split_repair_plan.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

bool sofle_repair_token_valid(const char *token) {
    if (!token) { return false; }
    for (size_t i = 0; i < 12; i++) {
        if (!((token[i] >= '0' && token[i] <= '9') ||
              (token[i] >= 'a' && token[i] <= 'f'))) { return false; }
    }
    return (token[12] == '0' || token[12] == '1') && token[13] == '\0' &&
        strncmp(token, "000000000000", 12) && strncmp(token, "ffffffffffff", 12);
}

int sofle_repair_make_plan(const struct sofle_repair_inventory *s,
                           struct sofle_repair_plan *plan) {
    memset(plan, 0, sizeof(*plan));
    if (s->read_error) { return s->read_error; }
    if (s->host_count > SOFLE_REPAIR_MAX_PEERS || s->bond_count > SOFLE_REPAIR_MAX_PEERS) {
        return -E2BIG;
    }
    for (size_t i = 0; i < s->host_count; i++) {
        if (!sofle_repair_token_valid(s->hosts[i])) { return -EINVAL; }
    }
    for (size_t i = 0; i < s->bond_count; i++) {
        if (!sofle_repair_token_valid(s->bonds[i])) { return -EINVAL; }
    }
    const char *target;
    if (s->central) {
        /* Never guess a central's split peer from its list of host bonds. */
        target = s->peripheral;
    } else {
        if (s->bond_count == 0) { return 0; }
        if (s->bond_count != 1) { return -E2BIG; }
        target = s->bonds[0];
    }
    if (!sofle_repair_token_valid(target)) { return -EINVAL; }
    for (size_t i = 0; i < s->host_count; i++) {
        if (!strcmp(target, s->hosts[i])) { return -EPERM; }
    }
    /* Zephyr 3.5 BT_ID_DEFAULT peer records only. Identity/IRK, host profiles,
     * keymap and unrelated settings are never deletion candidates. */
    static const char *const namespaces[] = {"keys", "ccc", "cf", "sc"};
    for (size_t i = 0; i < SOFLE_REPAIR_PATH_COUNT; i++) {
        snprintf(plan->paths[i], sizeof(plan->paths[i]), "bt/%s/%s", namespaces[i], target);
    }
    plan->count = SOFLE_REPAIR_PATH_COUNT;
    return 0;
}
