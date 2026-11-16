#include <assert.h>
#include <stdio.h>
#include "gradient_color.h"
static int high(struct gradient_rgb p) { int x=p.r>p.g?p.r:p.g; return x>p.b?x:p.b; }
static int low(struct gradient_rgb p) { int x=p.r<p.g?p.r:p.g; return x<p.b?x:p.b; }
int main(void) {
    /* Black stays off at every LED; gray stays gray. Colour conversion cannot
     * increase peak brightness or alter saturation, including hue-wrap sectors. */
    for (unsigned r=0;r<256;r+=17) for (unsigned g=0;g<256;g+=17) for (unsigned b=0;b<256;b+=17) {
        struct gradient_rgb in={r,g,b};
        for (unsigned i=0;i<30;i++) {
            struct gradient_rgb out=sofle_gradient_color(in,i,30);
            assert(high(out)==high(in) && low(out)==low(in));
            if (r==g && g==b) { assert(out.r==r && out.g==g && out.b==b); }
        }
    }
    struct gradient_rgb lilac={47,35,53};
    struct gradient_rgb start=sofle_gradient_color(lilac,0,30);
    struct gradient_rgb end=sofle_gradient_color(lilac,29,30);
    assert(start.r==47 && start.g==35 && start.b==53);
    assert(end.r==53 && end.g==35 && end.b<53 && end.b>35);
    unsigned changes=0;
    struct gradient_rgb last=start;
    for (unsigned i=1;i<30;i++) {
        struct gradient_rgb p=sofle_gradient_color(lilac,i,30);
        changes += p.r!=last.r || p.g!=last.g || p.b!=last.b;
        assert(p.r>=last.r && p.b<=last.b);
        last=p;
    }
    assert(changes>=10);
    assert(sofle_gradient_color(lilac,0,0).r==47);
    assert(sofle_gradient_color(lilac,1,1).r==47);
    assert(sofle_gradient_color(lilac,65535,30).r==end.r);
    puts("PASS: lilac-to-pink endpoints, smooth hue progression, black/off and brightness bounds across 122880 samples");
}
