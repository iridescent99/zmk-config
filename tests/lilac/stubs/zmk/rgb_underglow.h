#pragma once
#include <stdint.h>
struct zmk_led_hsb { uint16_t h; uint8_t s; uint8_t b; };
int zmk_rgb_underglow_set_hsb(struct zmk_led_hsb color);
int zmk_rgb_underglow_select_effect(int effect);
int zmk_rgb_underglow_on(void);
