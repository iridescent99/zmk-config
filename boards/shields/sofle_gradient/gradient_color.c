/* SPDX-License-Identifier: MIT */
#include "gradient_color.h"

/* Rotate hue by 0..40 degrees along each physical LED chain. Preserve the
 * incoming minimum/maximum channels, so ZMK's brightness cap, off, saturation,
 * hue controls and effects still work. No floating point, state or allocation.
 * Default H280 lilac becomes H320 pink at the far end of each chain. */
struct gradient_rgb sofle_gradient_color(struct gradient_rgb in, uint16_t position,
                                         uint16_t count) {
    int r = in.r, g = in.g, b = in.b;
    int hi = r > g ? r : g;
    if (b > hi) { hi = b; }
    int lo = r < g ? r : g;
    if (b < lo) { lo = b; }
    int delta = hi - lo;
    if (!delta || count < 2 || !position) { return in; }
    if (position >= count) { position = count - 1; }
    int hue;
    if (hi == r) { hue = 256 * (g - b) / delta; }
    else if (hi == g) { hue = 512 + 256 * (b - r) / delta; }
    else { hue = 1024 + 256 * (r - g) / delta; }
    hue = (hue + 1536 + (40U * 256U * position) / (60U * (count - 1))) % 1536;
    int sector = hue / 256;
    int fraction = hue % 256;
    int middle = lo + delta * ((sector & 1) ? 256 - fraction : fraction) / 256;
    switch (sector) {
    case 0: return (struct gradient_rgb){hi, middle, lo};
    case 1: return (struct gradient_rgb){middle, hi, lo};
    case 2: return (struct gradient_rgb){lo, hi, middle};
    case 3: return (struct gradient_rgb){lo, middle, hi};
    case 4: return (struct gradient_rgb){middle, lo, hi};
    default: return (struct gradient_rgb){hi, lo, middle};
    }
}
