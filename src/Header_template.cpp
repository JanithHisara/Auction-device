#include "Header_template.h"

// ================== GLOBAL UI OBJECTS ==================
lv_obj_t *top_bar;
lv_obj_t *label_battery;
lv_obj_t *label_battery_percent;
lv_obj_t *label_wifi;
lv_obj_t *label_date;
lv_obj_t *content_area;
lv_obj_t *label_mqtt;

// State variables (optional)
static bool dark_mode_state;
static bool wifi_connected_state;
static bool mqtt_connected_state;

// ================== TOP BAR CREATION ==================
void create_topbar(uint16_t screen_width, uint16_t screen_height) {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);

    // Top bar
    top_bar = lv_obj_create(scr);
    lv_obj_set_size(top_bar, screen_width, 40);
    lv_obj_set_pos(top_bar, 0, 0);

    // Battery icon
    label_battery = lv_label_create(top_bar);
    lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(label_battery, LV_ALIGN_TOP_LEFT, 5, 5);

    // Battery percentage
    label_battery_percent = lv_label_create(top_bar);
    lv_obj_set_style_text_font(label_battery_percent, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_battery_percent, "0%");
    lv_obj_align(label_battery_percent, LV_ALIGN_TOP_LEFT, 30, 5);

    // Date / Time
    label_date = lv_label_create(top_bar);
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_date, "00:00 01/01");
    lv_obj_align(label_date, LV_ALIGN_TOP_RIGHT, -5, 5);

    // WiFi icon
    label_wifi = lv_label_create(top_bar);
    lv_obj_set_style_text_font(label_wifi, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_wifi, LV_SYMBOL_WIFI);
    lv_obj_align(label_wifi, LV_ALIGN_TOP_RIGHT, -92, 5);

    // MQTT icon
    label_mqtt = lv_label_create(top_bar);
    lv_obj_set_style_text_font(label_mqtt, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_mqtt, LV_SYMBOL_LOOP);   // MQTT icon
    lv_obj_align(label_mqtt, LV_ALIGN_TOP_RIGHT, -120, 5);

    // Content area below top bar
    content_area = lv_obj_create(scr);
    lv_obj_set_size(content_area, screen_width, screen_height - 40);
    lv_obj_set_pos(content_area, 0, 40);
    lv_obj_clear_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);
    
    // Set default text color for everything in the content area to white/light-gray
    lv_obj_set_style_text_color(content_area, lv_color_white(), 0);

    // Apply initial dark mode
    set_dark_mode(dark_mode_state);
}

// ================== HELPER FUNCTIONS ==================
void set_battery_color(lv_color_t color) {
    lv_obj_set_style_text_color(label_battery, color, 0);
    lv_obj_set_style_text_color(label_battery_percent, color, 0);
}

void set_battery_percent(uint8_t percent) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%d%%", percent);
    lv_label_set_text(label_battery_percent, buf);
}

// void set_wifi_connected(bool connected) {
//     wifi_connected_state = connected;
//     if (connected) {
//         lv_label_set_text(label_wifi, "\uF1EB"); // WiFi icon
//     } else {
//         //lv_label_set_text(label_wifi, "\uF1F6"); // WiFi off icon
//         lv_obj_add_flag(label_wifi, LV_OBJ_FLAG_HIDDEN); 
//     }
// }

void set_wifi_connected(bool connected) {
    wifi_connected_state = connected;
    if (!connected) {
        lv_obj_add_flag(label_wifi, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_set_style_bg_opa(label_wifi, LV_OPA_TRANSP, 0);   // transparent background
        lv_obj_set_style_border_width(label_wifi, 0, 0);         // remove border
    } else {
        lv_obj_clear_flag(label_wifi, LV_OBJ_FLAG_HIDDEN); 
        lv_label_set_text(label_wifi, LV_SYMBOL_WIFI);
    }
}

void set_mqtt_connected(bool connected) {
    mqtt_connected_state = connected;

    if (!connected) {
        lv_obj_add_flag(label_mqtt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_opa(label_mqtt, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(label_mqtt, 0, 0);
    } else {
        lv_obj_clear_flag(label_mqtt, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(label_mqtt, LV_SYMBOL_LOOP);   // MQTT icon
    }
}

void set_date_time(const char* datetime) {
    lv_label_set_text(label_date, datetime);
}

void set_dark_mode(bool enable) {
    dark_mode_state = enable;
    if (enable) {
        lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x2B2B36), 0);
        lv_obj_set_style_text_color(label_battery, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_battery_percent, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_wifi, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_date, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_mqtt, lv_color_white(), 0);
    } else {
        lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x2B2B36), 0);
        lv_obj_set_style_text_color(label_battery, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_battery_percent, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_wifi, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_date, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_mqtt, lv_color_white(), 0);
    }
}
