/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
struct gradient_rgb { uint8_t r, g, b; };
struct gradient_rgb sofle_gradient_color(struct gradient_rgb in, uint16_t position,
                                         uint16_t count);
