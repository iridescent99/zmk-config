#include <assert.h>
#include <stdio.h>
#include <zmk/rgb_underglow.h>
static int calls, fail_at;
static struct zmk_led_hsb colour;
static int effect = 3;
static bool powered;
#include "../../boards/shields/sharp_mip/widgets/lilac_lighting.c"
int zmk_rgb_underglow_set_hsb(struct zmk_led_hsb value) {
    calls++;
    if (fail_at == 1) { return -1; }
    colour = value;
    return 0;
}
int zmk_rgb_underglow_select_effect(int value) {
    calls++;
    if (fail_at == 2) { return -2; }
    effect = value;
    return 0;
}
int zmk_rgb_underglow_on(void) {
    calls++;
    if (fail_at == 3) { return -3; }
    powered = true;
    return 0;
}
int main(void) {
    /* Starting with saved OFF and an animation, including API failure paths. */
    for (fail_at = 1; fail_at <= 3; fail_at++) {
        calls = 0;
        assert(registered_commit() == -fail_at);
        assert(calls == fail_at && !powered);
    }
    fail_at = 0; calls = 0;
    assert(registered_commit() == 0);
    assert(calls == 3 && powered && effect == 0);
    assert(colour.h == 280 && colour.s == 35 && colour.b == 70);
    /* Subsequent settings commits must not override runtime RGB controls. */
    powered = false;
    assert(registered_commit() == 0);
    assert(calls == 3 && !powered);
    puts("PASS: saved-off to lilac, power-on, solid effect, errors, no repeated override");
}
