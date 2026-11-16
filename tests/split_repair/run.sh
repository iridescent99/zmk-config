#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_binary=$(mktemp)
trap 'rm -f "$test_binary"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$repo/boards/shields/sharp_mip/widgets" \
    "$repo/boards/shields/sharp_mip/widgets/split_repair_plan.c" \
    "$repo/tests/split_repair/plan_test.c" -o "$test_binary"
"$test_binary"
for central in 0 1; do
    ${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
        -fsanitize=address,undefined -fno-omit-frame-pointer \
        -DCONFIG_ZMK_SPLIT_ROLE_CENTRAL=$central \
        -I"$repo/tests/split_repair/stubs" -I"$repo/tests/split_repair" \
        -I"$repo/boards/shields/sharp_mip/widgets" \
        "$repo/boards/shields/sharp_mip/widgets/split_repair_plan.c" \
        "$repo/tests/split_repair/adapter_test.c" -o "$test_binary"
    "$test_binary"
done
