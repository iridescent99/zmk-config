/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "keypress_model.h"

static void expect_label(struct keypress_model *s, const char *expected) {
    char actual[KEYPRESS_LABEL_SIZE];
    keypress_label(s, actual);
    assert(!strcmp(actual, expected));
}

static void key(struct keypress_model *s, uint32_t code, bool down, int64_t t) {
    keypress_event(s, 7, code, 0, 0, down, t);
}

int main(void) {
    struct keypress_model s = {0};
    expect_label(&s, "");
    for (unsigned i = 0; i < 26; i++) {
        char expected[] = {(char)('A' + i), 0};
        key(&s, 4 + i, true, i); expect_label(&s, expected);
    }
    const char *digits = "1234567890", *symbols = "!@#$%^&*()";
    for (unsigned i = 0; i < 10; i++) {
        char expected[] = {digits[i], 0};
        key(&s, 0x1e + i, true, i); expect_label(&s, expected);
        expected[0] = symbols[i];
        keypress_event(&s, 7, 0x1e + i, 2, 0, true, i); expect_label(&s, expected);
    }
    const char *plain = "-=[]\\#;'`,./", *shifted = "_+{}|~:\"~<>?";
    for (unsigned i = 0; i < 12; i++) {
        char expected[] = {plain[i], 0};
        key(&s, 0x2d + i, true, i); expect_label(&s, expected);
        expected[0] = shifted[i];
        keypress_event(&s, 7, 0x2d + i, 2, 0, true, i); expect_label(&s, expected);
    }
    const char *special[] = {"ENTER", "ESC", "BSPC", "TAB", "SPACE"};
    for (unsigned i = 0; i < 5; i++) {
        key(&s, 0x28 + i, true, 0); expect_label(&s, special[i]);
    }
    const char *arrows[] = {"RIGHT", "LEFT", "DOWN", "UP"};
    for (unsigned i = 0; i < 4; i++) {
        key(&s, 0x4f + i, true, 0); expect_label(&s, arrows[i]);
    }

    s = (struct keypress_model){0};
    key(&s, 4, true, 0);
    assert(!keypress_expire(&s, 10000)); /* Held key does not time out. */
    key(&s, 4, false, 10001);
    assert(!keypress_expire(&s, 11000));
    assert(keypress_expire(&s, 11001));
    expect_label(&s, "");
    assert(s.page == 0 && s.keycode == 0 && s.expires_at == 0);

    key(&s, 4, true, 0); key(&s, 5, true, 10); key(&s, 4, false, 20);
    assert(s.down); expect_label(&s, "B");
    key(&s, 5, false, 30);
    key(&s, 6, true, 40);
    assert(!keypress_expire(&s, 1030)); expect_label(&s, "C");
    key(&s, 6, false, 1040);
    assert(!keypress_expire(&s, 2039));
    assert(keypress_expire(&s, 2040));

    s = (struct keypress_model){0};
    key(&s, 0xe1, true, 0); expect_label(&s, "SHIFT");
    key(&s, 0xe5, true, 1); key(&s, 0xe1, false, 2);
    assert(keypress_modifiers(&s) == 0x20);
    key(&s, 0x1e, true, 3); expect_label(&s, "!");
    key(&s, 0xe5, false, 4);
    assert(keypress_modifiers(&s) == 0);
    expect_label(&s, "!"); assert(s.down); /* Modifier release cannot erase key. */
    key(&s, 0x1e, false, 5);
    assert(keypress_expire(&s, 1005));

    keypress_event(&s, 7, 0xe0, 0, 2, true, 0); /* LS(LCTRL) */
    key(&s, 0xe1, true, 1);
    keypress_event(&s, 7, 0xe0, 0, 2, false, 2);
    assert(keypress_modifiers(&s) == 2);
    key(&s, 0xe1, false, 3); assert(keypress_modifiers(&s) == 0);

    const char *mods[] = {"CTRL", "SHIFT", "ALT", "GUI"};
    s = (struct keypress_model){0};
    for (unsigned i = 0; i < 8; i++) {
        key(&s, 0xe0 + i, true, i); expect_label(&s, mods[i % 4]);
        key(&s, 0xe0 + i, false, i);
    }
    assert(keypress_modifiers(&s) == 0);
    keypress_event(&s, 0x0c, 0xe9, 0, 0, true, 0); expect_label(&s, "VOL+");
    keypress_event(&s, 0x0c, 0xb6, 0, 0, true, 0); expect_label(&s, "PREV");
    keypress_event(&s, 0xffff, UINT32_MAX, 0, 0, true, 0); expect_label(&s, "?");
    keypress_event(&s, 7, UINT32_MAX, 0, 0, true, 0); expect_label(&s, "?");

    /* 10,000 rapid taps: only the latest usage survives, bounded memory, and
     * duplicate/unrelated releases must not extend the expiry deadline. */
    s = (struct keypress_model){0};
    for (int i = 0; i < 10000; i++) {
        key(&s, 4 + i % 26, true, i * 2);
        key(&s, 4 + i % 26, false, i * 2 + 1);
    }
    int64_t deadline = s.expires_at;
    key(&s, s.keycode, false, deadline - 1);
    key(&s, 0xe1, false, deadline - 1);
    assert(s.expires_at == deadline);
    assert(keypress_expire(&s, deadline));
    expect_label(&s, "");
    puts("PASS: labels, modifiers, overlap, expiry, unsupported HID, 10000 rapid taps");
}
