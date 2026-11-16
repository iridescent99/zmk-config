/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#define STARFIELD_WIDTH 140
#define STARFIELD_HEIGHT 68
#define STARFIELD_FRAME_MS 125
/* Physical landscape buffer, portrait scene. One byte per monochrome pixel. */
void sofle_starfield_render(uint8_t *pixels,
                           uint32_t frame, bool rotated);
