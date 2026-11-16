/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define KEYPRESS_HOLD_MS 1000
#define KEYPRESS_LABEL_SIZE 8

/* One latest usage, eight modifier reference counts; never a typing history. */
struct keypress_model {
    uint32_t keycode;
    uint16_t page;
    uint8_t modifier_refs[8];
    uint8_t pressed_modifiers;
    bool visible;
    bool down;
    int64_t expires_at;
};

void keypress_event(struct keypress_model *s, uint16_t page, uint32_t code,
                    uint8_t implicit, uint8_t explicit, bool pressed, int64_t now);
bool keypress_expire(struct keypress_model *s, int64_t now);
uint8_t keypress_modifiers(const struct keypress_model *s);
void keypress_label(const struct keypress_model *s, char out[KEYPRESS_LABEL_SIZE]);
