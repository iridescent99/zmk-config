#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_binary=$(mktemp)
trap 'rm -f "$test_binary"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -include stdbool.h -I"$repo/tests/lilac/stubs" \
    "$repo/tests/lilac/lighting_test.c" -o "$test_binary"
"$test_binary"
