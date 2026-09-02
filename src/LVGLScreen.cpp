#include "LVGLScreen.h"
#define Screen_WIDTH  240
#define screen_HEIGHT 320
// ------------------ LVGL BUFFER ------------------
lv_disp_draw_buf_t draw_buf;
lv_color_t buf[Screen_WIDTH * 40]; // use TFT_WIDTH from main

// ------------------ Ticker for LVGL ------------------
Ticker lvgl_tick;

// ------------------ LVGL TICK CALLBACK ------------------
void lvgl_tick_cb() {
    lv_tick_inc(1); // increment LVGL internal tick
}

// ------------------ LVGL INIT ------------------
void lvgl_init() {
    lv_init();

    // Initialize display buffer
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, Screen_WIDTH * 40);

    // Register display driver
    static lv_disp_drv_t drv;
    lv_disp_drv_init(&drv);
    drv.hor_res = Screen_WIDTH;
    drv.ver_res = screen_HEIGHT;
    drv.flush_cb = [](lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
        tft.pushColors(area->x1, area->y1, area->x2, area->y2, color_p);
        lv_disp_flush_ready(disp);
    };
    drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&drv);

    // Force the default screen background to dark theme globally
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x1E1E24), 0);

    // Attach 1ms tick
    lvgl_tick.attach_ms(1, lvgl_tick_cb);
}
