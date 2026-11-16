/* SPDX-License-Identifier: MIT */
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/settings/settings.h>
#include <zmk/rgb_underglow.h>

/* Run after every settings setter, so persisted OFF/black/animation preferences
 * cannot override the requested startup colour. No input listener or polling.
 * The public RGB API enables the board's external LED power and saves only the
 * RGB/power preferences. The board's existing 30% hardware brightness cap stays.
 */
static int lilac_settings_commit(void) {
    static bool applied;
    if (applied) { return 0; }

    /* Low saturation makes this lilac rather than saturated violet. ZMK scales
     * brightness 70 by the existing 30% board cap, giving 21% maximum duty. */
    int rc = zmk_rgb_underglow_set_hsb((struct zmk_led_hsb){.h = 280, .s = 35, .b = 70});
    if (rc < 0) { return rc; }
    rc = zmk_rgb_underglow_select_effect(0); /* Solid, same on both halves. */
    if (rc < 0) { return rc; }
    rc = zmk_rgb_underglow_on();
    if (rc < 0) { return rc; }
    applied = true;
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(sofle_lilac, "sofle_lilac", NULL, NULL,
                               lilac_settings_commit, NULL);
