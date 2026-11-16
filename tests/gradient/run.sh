#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
test_binary=$(mktemp)
trap 'rm -f "$test_binary"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$repo/boards/shields/sofle_gradient" \
    "$repo/boards/shields/sofle_gradient/gradient_color.c" \
    "$repo/tests/gradient/color_test.c" -o "$test_binary"
"$test_binary"

${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"$repo/boards/shields/sofle_gradient" \
    "$repo/boards/shields/sofle_gradient/physical_gradient.c" \
    "$repo/tests/gradient/physical_test.c" -o "$test_binary"
"$test_binary"
