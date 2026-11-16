#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "split_repair_plan.h"

int main(void) {
    struct sofle_repair_inventory s = {.central = true, .host_count = 1, .bond_count = 2};
    strcpy(s.peripheral, "1122334455661");
    strcpy(s.hosts[0], "aabbccddeeff0");
    strcpy(s.bonds[0], s.hosts[0]);
    strcpy(s.bonds[1], s.peripheral);
    struct sofle_repair_plan p;
    assert(!sofle_repair_make_plan(&s, &p) && p.count == 4);
    const char *expected[] = {"bt/keys/1122334455661", "bt/ccc/1122334455661",
                             "bt/cf/1122334455661", "bt/sc/1122334455661"};
    for (size_t i = 0; i < p.count; i++) { assert(!strcmp(p.paths[i], expected[i])); }

    /* Refuse to erase a host even when it is incorrectly marked as the split peer. */
    strcpy(s.peripheral, s.hosts[0]);
    assert(sofle_repair_make_plan(&s, &p) == -EPERM && p.count == 0);
    s.peripheral[0] = '\0';
    assert(sofle_repair_make_plan(&s, &p) == -EINVAL && p.count == 0);
    s.central = false;
    assert(sofle_repair_make_plan(&s, &p) == -E2BIG && p.count == 0);
    s.bond_count = 1;
    assert(sofle_repair_make_plan(&s, &p) == -EPERM && p.count == 0);
    strcpy(s.bonds[0], "1122334455661");
    assert(!sofle_repair_make_plan(&s, &p) && p.count == 4);
    s.bond_count = 0;
    assert(!sofle_repair_make_plan(&s, &p) && p.count == 0);
    s.host_count = SOFLE_REPAIR_MAX_PEERS + 1;
    assert(sofle_repair_make_plan(&s, &p) == -E2BIG && p.count == 0);
    const char *invalid[] = {"", "a", "bt/id", "../bt/irk", "1122334455661/0",
        "1122334455669", "0000000000000", "ffffffffffff1", "11223344556z1"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        assert(!sofle_repair_token_valid(invalid[i]));
    }
    char unterminated[SOFLE_REPAIR_TOKEN_SIZE] = "1122334455661";
    unterminated[13] = 'x';
    assert(!sofle_repair_token_valid(unterminated));
    puts("PASS split repair: exact peer-only paths, host protection, ambiguity and malformed records");
}
