#ifndef LVGLSCREEN_H
#define LVGLSCREEN_H

#include <lvgl.h>
#include <Ticker.h>
#include "tft_init.h"

// ------------------ External references ------------------
// Declare these as extern so only one instance exists (defined in .cpp)
extern Ticker lvgl_tick;
extern lv_disp_draw_buf_t draw_buf;
extern lv_color_t buf[];

// Must be initialized in main.cpp before calling LVGLScreen
extern TFT_init tft;

// ------------------ Functions ------------------
void lvgl_tick_cb();
void lvgl_init();

#endif
