/* SPDX-License-Identifier: MIT */
#include "keypress_model.h"
#include <stdio.h>
#include <string.h>

/* USB HID keyboard/keypad (0x07) and consumer (0x0c) usage pages. Keeping this
 * small model independent of Zephyr allows the actual state machine to be tested. */
static bool is_modifier(uint16_t page, uint32_t code) {
    return page == 0x07 && code >= 0xe0 && code <= 0xe7;
}

uint8_t keypress_modifiers(const struct keypress_model *s) {
    uint8_t mods = 0;
    for (unsigned i = 0; i < 8; i++) {
        if (s->modifier_refs[i]) { mods |= 1u << i; }
    }
    return mods;
}

void keypress_event(struct keypress_model *s, uint16_t page, uint32_t code,
                    uint8_t implicit, uint8_t explicit, bool pressed, int64_t now) {
    /* Explicit modifier wrappers are additional references, just as in ZMK HID.
     * Left and right modifiers remain independent when one of them is released. */
    for (unsigned i = 0; i < 8; i++) {
        unsigned refs = !!(explicit & (1u << i));
        refs += is_modifier(page, code) && code == 0xe0 + i;
        while (refs--) {
            if (pressed && s->modifier_refs[i] < UINT8_MAX) { s->modifier_refs[i]++; }
            if (!pressed && s->modifier_refs[i]) { s->modifier_refs[i]--; }
        }
    }
    if (pressed) {
        s->page = page;
        s->keycode = code;
        s->pressed_modifiers = keypress_modifiers(s) | implicit;
        s->visible = true;
        s->down = true;
        s->expires_at = 0;
    } else if (s->visible && s->down && s->page == page && s->keycode == code) {
        s->down = false;
        s->expires_at = now + KEYPRESS_HOLD_MS;
    }
}

bool keypress_expire(struct keypress_model *s, int64_t now) {
    if (!s->visible || s->down || now < s->expires_at) { return false; }
    s->visible = false;
    s->page = 0;
    s->keycode = 0;
    s->pressed_modifiers = 0;
    s->expires_at = 0;
    return true;
}

void keypress_label(const struct keypress_model *s, char out[KEYPRESS_LABEL_SIZE]) {
    memset(out, 0, KEYPRESS_LABEL_SIZE);
    if (!s->visible) { return; }
    uint32_t k = s->keycode;
    const char *name = "?";
    if (s->page == 0x07) {
        bool shift = (s->pressed_modifiers & 0x22) != 0;
        if (k >= 0x04 && k <= 0x1d) {
            out[0] = 'A' + k - 0x04; /* Key legend, not host capitalization. */
            return;
        }
        if (k >= 0x1e && k <= 0x27) {
            out[0] = (shift ? "!@#$%^&*()" : "1234567890")[k - 0x1e];
            return;
        }
        if (k >= 0x2d && k <= 0x38) {
            out[0] = (shift ? "_+{}|~:\"~<>?" : "-=[]\\#;'`,./")[k - 0x2d];
            return;
        }
        if (k >= 0x3a && k <= 0x45) {
            snprintf(out, KEYPRESS_LABEL_SIZE, "F%u", (unsigned)(k - 0x39));
            return;
        }
        switch (k) {
        case 0x28: name = "ENTER"; break;
        case 0x29: name = "ESC"; break;
        case 0x2a: name = "BSPC"; break;
        case 0x2b: name = "TAB"; break;
        case 0x2c: name = "SPACE"; break;
        case 0x39: name = "CAPS"; break;
        case 0x46: name = "PRINT"; break;
        case 0x47: name = "SCROLL"; break;
        case 0x48: name = "PAUSE"; break;
        case 0x49: name = "INS"; break;
        case 0x4a: name = "HOME"; break;
        case 0x4b: name = "PGUP"; break;
        case 0x4c: name = "DEL"; break;
        case 0x4d: name = "END"; break;
        case 0x4e: name = "PGDN"; break;
        case 0x4f: name = "RIGHT"; break;
        case 0x50: name = "LEFT"; break;
        case 0x51: name = "DOWN"; break;
        case 0x52: name = "UP"; break;
        case 0x64: name = shift ? "|" : "\\"; break;
        case 0x65: name = "MENU"; break;
        case 0x7a: name = "UNDO"; break;
        case 0x7b: name = "CUT"; break;
        case 0x7c: name = "COPY"; break;
        case 0x7d: name = "PASTE"; break;
        case 0xe0: case 0xe4: name = "CTRL"; break;
        case 0xe1: case 0xe5: name = "SHIFT"; break;
        case 0xe2: case 0xe6: name = "ALT"; break;
        case 0xe3: case 0xe7: name = "GUI"; break;
        }
    } else if (s->page == 0x0c) {
        switch (k) {
        case 0xb0: name = "PLAY"; break;
        case 0xb5: name = "NEXT"; break;
        case 0xb6: name = "PREV"; break;
        case 0xb7: name = "STOP"; break;
        case 0xcd: name = "PLAY"; break;
        case 0xe2: name = "MUTE"; break;
        case 0xe9: name = "VOL+"; break;
        case 0xea: name = "VOL-"; break;
        }
    }
    snprintf(out, KEYPRESS_LABEL_SIZE, "%s", name);
}
