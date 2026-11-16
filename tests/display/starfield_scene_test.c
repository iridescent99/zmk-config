#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "starfield_scene.h"
int main(void) {
    struct { uint8_t before[16], pixels[140*68], after[16]; } guarded;
    memset(&guarded,0xa5,sizeof(guarded));
    uint8_t reverse[140*68], previous[140*68]={0};
    unsigned changed=0;
    for (uint32_t f=0;f<2000;f++) {
        sofle_starfield_render(guarded.pixels,f,true);
        sofle_starfield_render(reverse,f,false);
        unsigned lit=0;
        for (unsigned i=0;i<sizeof(reverse);i++) {
            assert(guarded.pixels[i]<=1);
            assert(guarded.pixels[i]==reverse[sizeof(reverse)-1-i]);
            lit+=guarded.pixels[i];
        }
        assert(lit>180 && lit<500);
        changed+=memcmp(previous,guarded.pixels,sizeof(previous))!=0;
        memcpy(previous,guarded.pixels,sizeof(previous));
        for (unsigned i=0;i<16;i++) { assert(guarded.before[i]==0xa5 && guarded.after[i]==0xa5); }
    }
    assert(changed==2000);
    sofle_starfield_render(guarded.pixels,UINT32_MAX,true);
    puts("PASS: 2000 changing starfield frames, both orientations, pixel bounds, density and frame overflow");
}
