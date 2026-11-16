/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "starfield_scene.h"
/* No random generator state, input history, floating point, or heap allocation. */
static uint32_t mix(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15;
    x *= 0x846ca68bU; return x ^ (x >> 16);
}
static void point(uint8_t *pixels, bool rotated, int x, int y) {
    if (x < 0 || x >= STARFIELD_HEIGHT || y < 0 || y >= STARFIELD_WIDTH) { return; }
    unsigned px = rotated ? (unsigned)y : STARFIELD_WIDTH-1U-(unsigned)y;
    unsigned py = rotated ? STARFIELD_HEIGHT-1U-(unsigned)x : (unsigned)x;
    pixels[py*STARFIELD_WIDTH+px] = 1;
}
void sofle_starfield_render(uint8_t *pixels, uint32_t frame, bool rotated) {
    memset(pixels, 0, STARFIELD_WIDTH * STARFIELD_HEIGHT);
    /* Fine-grained distant galactic ribbon, drifting much more slowly than stars. */
    for (unsigned i=0;i<85;i++) {
        uint32_t seed=mix(i+700U);
        int y=(int)((seed % 140U + (frame / 32U) % 140U) % 140U);
        int x=12 + y/4 + (int)((seed >> 16) % 15U) - 7;
        point(pixels,rotated,x,y);
    }
    /* Three parallax depths: distant pinpricks, intermediate stars, near diamonds.
     * Each has its own twinkle phase; no screen-wide brightness pulsing. */
    for (unsigned i=0;i<72;i++) {
        uint32_t seed=mix(i+1U);
        unsigned depth=i % 3U;
        unsigned period=depth==0 ? 16U : depth==1 ? 4U : 1U;
        int x=(int)((seed >> 16) % 68U);
        int y=(int)((seed % 140U + (frame / period) % 140U) % 140U);
        unsigned phase=(frame/4U + seed%41U)%41U;
        if (depth==0 && phase==0) { continue; }
        point(pixels,rotated,x,y);
        if (depth==2 || (depth==1 && phase<3)) {
            point(pixels,rotated,x-1,y); point(pixels,rotated,x+1,y);
            point(pixels,rotated,x,y-1); point(pixels,rotated,x,y+1);
        }
        if (depth==2 && phase<4) {
            point(pixels,rotated,x-2,y); point(pixels,rotated,x+2,y);
            point(pixels,rotated,x,y-2); point(pixels,rotated,x,y+2);
        }
    }
    /* Two staggered meteors per 23 seconds, with tapered, broken trailing dust. */
    unsigned cycle=frame%184U;
    for (unsigned m=0;m<2;m++) {
        unsigned start=m ? 126U : 42U;
        if (cycle<start || cycle>=start+19U) { continue; }
        int age=(int)(cycle-start);
        int head_x=-8+age*4;
        int head_y=(m ? 62 : 15)+age*2;
        for (int tail=0;tail<19;tail++) {
            if (tail>8 && (tail+age)%3==0) { continue; }
            point(pixels,rotated,head_x-tail,head_y-tail/2);
            if (tail<3) { point(pixels,rotated,head_x-tail,head_y-tail/2+1); }
        }
    }
}
