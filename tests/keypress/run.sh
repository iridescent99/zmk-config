#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_binary=$(mktemp)
trap 'rm -f "$test_binary"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$repo/boards/shields/sharp_mip/widgets" \
    "$repo/boards/shields/sharp_mip/widgets/keypress_model.c" \
    "$repo/tests/keypress/model_test.c" -o "$test_binary"
"$test_binary"
