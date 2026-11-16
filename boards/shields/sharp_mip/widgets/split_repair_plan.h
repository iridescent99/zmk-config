/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stddef.h>

#define SOFLE_REPAIR_TOKEN_SIZE 14
#define SOFLE_REPAIR_MAX_PEERS 6
#define SOFLE_REPAIR_PATH_SIZE 32
#define SOFLE_REPAIR_PATH_COUNT 4

struct sofle_repair_inventory {
    int read_error;
    bool central;
    char peripheral[SOFLE_REPAIR_TOKEN_SIZE];
    size_t host_count;
    char hosts[SOFLE_REPAIR_MAX_PEERS][SOFLE_REPAIR_TOKEN_SIZE];
    size_t bond_count;
    char bonds[SOFLE_REPAIR_MAX_PEERS][SOFLE_REPAIR_TOKEN_SIZE];
};
struct sofle_repair_plan {
    size_t count;
    char paths[SOFLE_REPAIR_PATH_COUNT][SOFLE_REPAIR_PATH_SIZE];
};

bool sofle_repair_token_valid(const char *token);
int sofle_repair_make_plan(const struct sofle_repair_inventory *inventory,
                           struct sofle_repair_plan *plan);
