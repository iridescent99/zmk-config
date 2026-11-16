#include <assert.h>
#include <stdio.h>
#include "physical_gradient.h"
#include "keebart_coordinates.h"
static unsigned peak(struct gradient_rgb c) { int p=c.r>c.g?c.r:c.g; return p>c.b?p:c.b; }
int main(void) {
    unsigned seen[60]={0};
    for (unsigned i=0;i<60;i++) {
        unsigned key=ks_led_positions[i];
        assert(key<60 && !seen[key]++);
        assert(sofle_led_x(i>=30,i%30)==xy[key][0]);
        for (unsigned v=0;v<256;v++) {
            struct gradient_rgb in={v,v/2,v/3};
            struct gradient_rgb out=sofle_physical_gradient(in,i>=30,i%30);
            assert(peak(out)==v && out.b==v);
            assert(out.b>=out.r && out.b>=out.g);
            if (!v) { assert(!out.r && !out.g && !out.b); }
            for (unsigned j=0;j<60;j++) if (xy[ks_led_positions[j]][0]==xy[key][0]) {
                struct gradient_rgb same=sofle_physical_gradient(in,j>=30,j%30);
                assert(same.r==out.r && same.g==out.g && same.b==out.b);
            }
        }
    }
    struct gradient_rgb in={255,170,230};
    struct gradient_rgb left=sofle_physical_gradient(in,false,26);
    struct gradient_rgb right=sofle_physical_gradient(in,true,26);
    assert(left.r==96 && left.g==192 && left.b==255);
    assert(right.r==160 && right.g==112 && right.b==255);
    assert(left.b>left.g && left.g>left.r); /* Blue, not pink. */
    assert(right.b>right.r && right.r>right.g); /* Blue-biased lilac. */
    for (unsigned i=0;i<60;i++) for (unsigned j=0;j<60;j++) {
        if (sofle_led_x(i>=30,i%30)>=sofle_led_x(j>=30,j%30)) { continue; }
        struct gradient_rgb a=sofle_physical_gradient(in,i>=30,i%30);
        struct gradient_rgb b=sofle_physical_gradient(in,j>=30,j%30);
        assert(a.r<=b.r && a.g>=b.g && a.b==b.b);
    }
    /* Across the split, inner thumb LEDs continue the palette smoothly. */
    struct gradient_rgb a=sofle_physical_gradient(in,false,1);
    struct gradient_rgb b=sofle_physical_gradient(in,true,1);
    assert(b.r>=a.r && b.r-a.r<4 && a.g>=b.g && a.g-b.g<4);
    assert(!peak(sofle_physical_gradient(in,true,30)));
    puts("PASS: all 60 LEDs match BT geometry, same-column colours, continuous blue-to-lilac progression, blue-dominant endpoints, brightness/off at 15360 levels");
}
