/* SPDX-License-Identifier: MIT */
#include "physical_gradient.h"
/* LED-chain -> physical X, from Keebart's Bluetooth board geometry:
 * Keebart/zmk-config@19d7f37d8339bbd9305ddfb1ddfb49b16ec7f8cd
 * studio/coordinates_sofle_bt.h: xy[ks_led_positions[led]][0].
 * Upstream calls the chain provisional; confirm orientation on hardware.
 * First half is left, second half right, viewed in normal typing orientation.
 */
static const uint8_t led_x[2][SOFLE_HALF_LEDS] = {
    {95,115,95,95,95,95,95,76,76,76,76,76,57,57,57,57,57,38,38,38,
     38,38,19,19,19,19,0,0,0,0},
    {129,119,129,129,129,129,129,148,148,148,148,148,167,167,167,167,
     167,186,186,186,186,186,209,209,209,209,224,224,224,224}
};
uint8_t sofle_led_x(bool right, unsigned led) {
    return led < SOFLE_HALF_LEDS ? led_x[right ? 1 : 0][led] : 0;
}
struct gradient_rgb sofle_physical_gradient(struct gradient_rgb input, bool right, unsigned led) {
    if (led >= SOFLE_HALF_LEDS) { return (struct gradient_rgb){0,0,0}; }
    unsigned peak = input.r > input.g ? input.r : input.g;
    if (input.b > peak) { peak = input.b; }
    unsigned x = sofle_led_x(right, led);
    /* Reference palette: lavender #c4a8f0 -> lilac #dd79e8 -> pink #e34eb8.
     * Fixed spatial palette, scaled only by ZMK brightness (including off).
     * Interpolation never exceeds the incoming peak or the board power cap.
     */
    static const uint8_t palette[3][3] = {{196,168,240},{221,121,232},{227,78,184}};
    unsigned segment = x >= SOFLE_WIDTH/2;
    unsigned t = x - segment * (SOFLE_WIDTH/2);
    uint8_t out[3];
    for (unsigned c=0;c<3;c++) {
        unsigned value = (palette[segment][c] * (SOFLE_WIDTH/2-t) +
                          palette[segment+1][c] * t + SOFLE_WIDTH/4) / (SOFLE_WIDTH/2);
        out[c] = (uint8_t)((value * peak + 127) / 255);
    }
    return (struct gradient_rgb){out[0],out[1],out[2]};
}
