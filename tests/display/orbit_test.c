/* Real LVGL drawing and production animation callback; simulated clock/activity. */
#include <stdio.h>
#include <string.h>
#include "fakes.h"
#include "../../boards/shields/sharp_mip/widgets/right_animation.c"
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
static void flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *colours) {
    for (int y = area->y1; y <= area->y2; y++) for (int x = area->x1; x <= area->x2; x++) {
        assert(x >= 0 && x < 160 && y >= 0 && y < 68);
        framebuffer[y*160+x] = lv_color_to1(*colours++);
    }
    flushes++;
    lv_disp_flush_ready(drv);
}
static void advance(int64_t target) {
    while (orbit_work.pending && orbit_work.due <= target) {
        now = orbit_work.due; orbit_work.pending = false;
        orbit_work.work.fn(&orbit_work.work);
    }
    now = target; lv_refr_now(NULL);
}
int main(void) {
    lv_init();
    static lv_color_t drawbuf[160 * 68];
    static lv_disp_draw_buf_t buf;
    lv_disp_draw_buf_init(&buf, drawbuf, NULL, 160 * 68);
    static lv_disp_drv_t drv;
    lv_disp_drv_init(&drv); drv.hor_res=160; drv.ver_res=68; drv.flush_cb=flush; drv.draw_buf=&buf;
    lv_disp_t *disp=lv_disp_drv_register(&drv);
    lv_disp_set_theme(disp, lv_theme_mono_init(disp, false, &lv_font_montserrat_16));
    lv_obj_t *screen=lv_obj_create(NULL);
    sofle_right_animation_init(screen);
    lv_scr_load(screen); advance(0);
    unsigned dark=0;
    for (unsigned i=0;i<sizeof(framebuffer);i++) { dark += !framebuffer[i]; }
    assert(dark>100);
    static uint8_t first[sizeof(framebuffer)], previous[sizeof(framebuffer)];
    memcpy(first, framebuffer, sizeof(first)); memcpy(previous, first, sizeof(first));
    /* Every successive frame moves the moon; one revolution repeats exactly. */
    for (unsigned frame=1;frame<=32;frame++) {
        advance(frame*250);
        assert(memcmp(previous, framebuffer, sizeof(previous)));
        for (int y=0;y<68;y++) for (int x=0;x<68;x++) {
            assert(framebuffer[y*160+x]==first[y*160+x]); /* Status area untouched. */
        }
        memcpy(previous, framebuffer, sizeof(previous));
    }
    assert(!memcmp(first,framebuffer,sizeof(first)));
    activity=ZMK_ACTIVITY_IDLE; activity_update(activity_get(NULL));
    assert(!orbit_work.pending);
    unsigned saved_flushes=flushes;
    advance(now+10000); assert(flushes==saved_flushes);
    activity=ZMK_ACTIVITY_SLEEP; activity_update(activity_get(NULL));
    advance(now+10000); assert(flushes==saved_flushes);
    activity=ZMK_ACTIVITY_ACTIVE; activity_update(activity_get(NULL));
    advance(now+250); assert(orbit_work.pending && flushes>saved_flushes);
    puts("PASS: 32 real LVGL animation frames, eight-second loop, status isolation, idle/sleep pause and wake");
}
