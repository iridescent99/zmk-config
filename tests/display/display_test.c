/* The production event/work/render adapter and real pinned LVGL software drawing.
 * Zephyr scheduling and hardware/host state are faked; LVGL is not mocked. */
#include <stdio.h>
#include <string.h>
#include "fakes.h"
#include "../../boards/shields/sharp_mip/widgets/keypress.c"
static int64_t now;
static struct k_work_q queue;
static uint8_t framebuffer[160 * 68];
static unsigned flushes;
int64_t k_uptime_get(void) { return now; }
struct k_work_q *zmk_display_work_q(void) { return &queue; }
int k_work_schedule_for_queue(struct k_work_q *q, struct k_work_delayable *w, int64_t delay) {
    assert(q == &queue);
    if (w->pending) { return 0; }
    w->pending = true; w->due = now + delay; return 1;
}
int k_work_reschedule_for_queue(struct k_work_q *q, struct k_work_delayable *w, int64_t delay) {
    assert(q == &queue); w->pending = true; w->due = now + delay; return 1;
}
static void flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *colours) {
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            assert(x >= 0 && x < 160 && y >= 0 && y < 68);
            framebuffer[y * 160 + x] = lv_color_to1(*colours++);
        }
    }
    flushes++;
    lv_disp_flush_ready(drv);
}
static void advance(int64_t target) {
    struct k_work_delayable *works[] = {&render_work, &expire_work};
    for (;;) {
        struct k_work_delayable *next = NULL;
        for (unsigned i = 0; i < 2; i++) {
            if (works[i]->pending && works[i]->due <= target &&
                (!next || works[i]->due < next->due)) { next = works[i]; }
        }
        if (!next) { break; }
        now = next->due; next->pending = false; next->work.fn(&next->work);
    }
    now = target;
    lv_refr_now(NULL);
}
static void key(uint32_t code, bool pressed) {
    zmk_event_t ev = {.usage_page = 7, .keycode = code, .state = pressed};
    assert(key_event(&ev) == ZMK_EV_EVENT_BUBBLE);
}
static unsigned key_pixels(void) {
    unsigned n = 0;
    for (int y = 0; y < 68; y++) {
        for (int x = 68; x < 136; x++) { n += !framebuffer[y * 160 + x]; }
    }
    return n;
}
int main(void) {
    lv_init();
    static lv_color_t drawbuf[160 * 68];
    static lv_disp_draw_buf_t buf;
    lv_disp_draw_buf_init(&buf, drawbuf, NULL, 160 * 68);
    static lv_disp_drv_t drv;
    lv_disp_drv_init(&drv); drv.hor_res = 160; drv.ver_res = 68;
    drv.flush_cb = flush; drv.draw_buf = &buf;
    lv_disp_t *disp = lv_disp_drv_register(&drv);
    lv_disp_set_theme(disp, lv_theme_mono_init(disp, false, &lv_font_montserrat_16));
    lv_obj_t *screen = lv_obj_create(NULL);
    static struct zmk_widget_status widget;
    assert(zmk_widget_status_init(&widget, screen) == 0);
    lv_obj_align(widget.obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_scr_load(screen);
    advance(0);
    assert(key_pixels() == 0);
    key(0x14, true); key(0x14, false); /* Q: a complete tap before render. */
    advance(24); assert(key_pixels() == 0);
    advance(25); assert(!strcmp(shown_label, "Q")); assert(key_pixels() > 30);
    static uint8_t q_frame[sizeof(framebuffer)];
    memcpy(q_frame, framebuffer, sizeof(framebuffer));
    key(0x0d, true); /* J, resolved identically when originating on the right. */
    advance(50); assert(!strcmp(shown_label, "J")); assert(key_pixels() > 30);
    assert(memcmp(q_frame, framebuffer, sizeof(framebuffer)));
    /* A status update must not overwrite the central key canvas. */
    memcpy(q_frame, framebuffer, sizeof(framebuffer));
    status_update(status_get(NULL)); advance(51);
    assert(!memcmp(q_frame, framebuffer, sizeof(framebuffer)));
    key(0x0d, false); advance(1050); assert(key_pixels() > 30);
    advance(1051); assert(key_pixels() == 0 && !render_work.pending && !expire_work.pending);
    /* Rapid distinct taps cannot keep rescheduling the pending frame forever. */
    for (unsigned i = 0; i < 1000; i++) {
        key(4 + i % 26, true); key(4 + i % 26, false); advance(now + 2);
    }
    advance(now + 25); assert(key_pixels() > 30);
    advance(now + 1000); assert(key_pixels() == 0);
    unsigned last_flush = flushes;
    advance(now + 1000); assert(flushes == last_flush);
    const uint32_t specials[] = {0x28,0x29,0x2a,0x2c,0xe0,0xe1,0xe2,0xe3,0x4f,0x50,0x51,0x52};
    for (unsigned i = 0; i < sizeof(specials)/sizeof(*specials); i++) {
        key(specials[i], true); advance(now + 25); assert(key_pixels() > 10);
        key(specials[i], false); advance(now + 1000);
    }
    puts("PASS: real monochrome LVGL pixels, Q/J, tap coalescing, status isolation, expiry, special keys, no idle refresh");
    return 0;
}
