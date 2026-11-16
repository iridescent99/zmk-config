/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "gradient_color.h"
#define SOFLE_HALF_LEDS 30
#define SOFLE_WIDTH 224
uint8_t sofle_led_x(bool right, unsigned led);
struct gradient_rgb sofle_physical_gradient(struct gradient_rgb input, bool right, unsigned led);
