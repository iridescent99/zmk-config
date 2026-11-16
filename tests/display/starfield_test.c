/* Real LVGL drawing and production animation callback; simulated clock/activity. */
#include <stdio.h>
#include <string.h>
#include "fakes.h"
#include "lvgl_display.h"
#include "../../boards/shields/sharp_mip/widgets/right_starfield.c"
static int64_t now;
static struct k_work_q queue;
static enum zmk_activity_state activity = ZMK_ACTIVITY_ACTIVE;
static uint8_t framebuffer[160 * 68];
static unsigned flushes;
int64_t k_uptime_get(void) { return now; }
struct k_work_q *zmk_display_work_q(void) { return &queue; }
enum zmk_activity_state zmk_activity_get_state(void) { return activity; }
int k_work_reschedule_for_queue(struct k_work_q *q, struct k_work_delayable *w, int64_t delay) {
    assert(q == &queue); w->pending = true; w->due = now + delay; return 1;
}
int k_work_cancel_delayable(struct k_work_delayable *w) { w->pending = false; return 0; }
/* Sharp LS0xx sends packed bits unchanged; hardware high/1 is white.
 * Use the real pinned Zephyr mono adapter to get from LVGL to these bits. */
int display_write(const struct device *dev, uint16_t x, uint16_t y,
                  const struct display_buffer_descriptor *desc, const void *data) {
    (void)dev;
    assert(x==0 && desc->width==160 && desc->pitch==160);
    assert(y+desc->height<=68 && desc->buf_size==20U*desc->height);
    const uint8_t *bits=data;
    for (unsigned row=0;row<desc->height;row++) for (unsigned col=0;col<160;col++) {
        framebuffer[(y+row)*160+col]=(bits[row*20+col/8]>>(col%8))&1U;
    }
    flushes++;
    return 0;
}
static void verify_polarity(uint32_t frame) {
    uint8_t expected[STARFIELD_WIDTH*STARFIELD_HEIGHT];
    sofle_starfield_render(expected,frame,true);
    unsigned stars=0;
    for (unsigned y=0;y<68;y++) for (unsigned x=0;x<140;x++) {
        unsigned physical_white=framebuffer[y*160+x+20];
        assert(physical_white==expected[y*140+x]);
        stars+=physical_white;
    }
    /* More than 94% dark sky, with discrete white stars, not the reverse. */
    assert(stars>180 && stars<500);
}
static void advance(int64_t target) {
    while (starfield_work.pending && starfield_work.due <= target) {
        now = starfield_work.due; starfield_work.pending = false;
        starfield_work.work.fn(&starfield_work.work);
    }
    now = target; lv_refr_now(NULL);
}
int main(void) {
    lv_init();
    static uint8_t drawbuf[160 * 68 / 8];
    static lv_disp_draw_buf_t buf;
    lv_disp_draw_buf_init(&buf, drawbuf, NULL, 160 * 68);
    static lv_disp_drv_t drv;
    lv_disp_drv_init(&drv); drv.hor_res=160; drv.ver_res=68; drv.flush_cb=lvgl_flush_cb_mono; drv.draw_buf=&buf;
    drv.set_px_cb=lvgl_set_px_cb_mono;
    drv.rounder_cb=lvgl_rounder_cb_mono;
    static struct lvgl_disp_data data={.cap={
        .x_resolution=160,.y_resolution=68,.current_pixel_format=PIXEL_FORMAT_MONO01,
        .screen_info=SCREEN_INFO_X_ALIGNMENT_WIDTH}};
    drv.user_data=&data;
    lv_disp_t *disp=lv_disp_drv_register(&drv);
    lv_disp_set_theme(disp, lv_theme_mono_init(disp, false, &lv_font_montserrat_16));
    lv_obj_t *screen=lv_obj_create(NULL);
    sofle_right_starfield_init(screen);
    lv_scr_load(screen); advance(0);
    verify_polarity(0);
    static uint8_t first[sizeof(framebuffer)], previous[sizeof(framebuffer)];
    memcpy(first, framebuffer, sizeof(first)); memcpy(previous, first, sizeof(first));
    /* Every successive frame moves near stars; header pixels remain unchanged. */
    for (unsigned frame=1;frame<=184;frame++) {
        advance(frame*125);
        verify_polarity(frame);
        assert(memcmp(previous, framebuffer, sizeof(previous)));
        for (int y=0;y<68;y++) for (int x=0;x<20;x++) {
            assert(framebuffer[y*160+x]==first[y*160+x]); /* Status area untouched. */
        }
        memcpy(previous, framebuffer, sizeof(previous));
    }

    activity=ZMK_ACTIVITY_IDLE; activity_update(activity_get(NULL));
    assert(!starfield_work.pending);
    unsigned saved_flushes=flushes;
    advance(now+10000); assert(flushes==saved_flushes);
    activity=ZMK_ACTIVITY_SLEEP; activity_update(activity_get(NULL));
    advance(now+10000); assert(flushes==saved_flushes);
    activity=ZMK_ACTIVITY_ACTIVE; activity_update(activity_get(NULL));
    advance(now+125); assert(starfield_work.pending && flushes>saved_flushes);
    puts("PASS: 184 real LVGL + Zephyr MONO01 frames, dark sky/white stars, meteor cycle, status isolation, idle/sleep pause and wake");
}
