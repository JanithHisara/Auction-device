#ifndef HEADER_TEMPLATE_H
#define HEADER_TEMPLATE_H

#include <Arduino.h>
#include <lvgl.h>
LV_FONT_DECLARE(lv_font_montserrat_14);

// ================== GLOBAL UI OBJECTS ==================
extern lv_obj_t *top_bar;
extern lv_obj_t *label_battery;
extern lv_obj_t *label_battery_percent;
extern lv_obj_t *label_wifi;
extern lv_obj_t *label_date; 
extern lv_obj_t *content_area;
extern lv_obj_t *label_mqtt;

// ================== FUNCTIONS ==================
void create_topbar(uint16_t screen_width, uint16_t screen_height);
// Update the battery percentage
void set_battery_percent(uint8_t percent);
void set_battery_color(lv_color_t color);

// Update WiFi status
void set_wifi_connected(bool connected);
void set_mqtt_connected(bool connected); 

// Update date/time
void set_date_time(const char* datetime);

// Toggle dark mode
void set_dark_mode(bool enable);

#endif // HEADER_TEMPLATE_H
