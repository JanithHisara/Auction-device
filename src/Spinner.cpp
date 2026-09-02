#include "Spinner.h"

// Static member initialization
lv_obj_t* Spinner::window = NULL;
lv_obj_t* Spinner::message_label = NULL;
lv_obj_t* Spinner::spinner = NULL;
bool Spinner::initialized = false;

void Spinner::begin() {
    if (initialized) return;
    
    // Create full-screen window
    window = lv_obj_create(lv_scr_act());
    lv_obj_set_size(window, 240, 320);
    lv_obj_set_style_bg_color(window, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(window, LV_OPA_80, 0);
    lv_obj_set_style_border_width(window, 0, 0);
    lv_obj_clear_flag(window, LV_OBJ_FLAG_SCROLLABLE);
    
    // Center content container
    lv_obj_t* container = lv_obj_create(window);
    lv_obj_set_size(container, 200, 160);
    lv_obj_center(container);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x2B2B36), 0);
    lv_obj_set_style_radius(container, 10, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_shadow_width(container, 20, 0);
    lv_obj_set_style_shadow_ofs_x(container, 0, 0);
    lv_obj_set_style_shadow_ofs_y(container, 4, 0);
    lv_obj_set_style_shadow_opa(container, LV_OPA_30, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // Add spinner animation
    spinner = lv_spinner_create(container, 1000, 60);
    lv_obj_set_size(spinner, 60, 60);
    lv_obj_center(spinner);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x3A3A4A), LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x5C9ACF), LV_PART_INDICATOR);
    
    // Add message label
    message_label = lv_label_create(container);
    lv_obj_set_width(message_label, 180);
    lv_label_set_text(message_label, "Loading...");
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(message_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(message_label, &lv_font_montserrat_14, 0);
    lv_obj_align(message_label, LV_ALIGN_BOTTOM_MID, 0, -15);
    
    // Hide by default
    lv_obj_add_flag(window, LV_OBJ_FLAG_HIDDEN);
    
    initialized = true;
}

void Spinner::start(const char* message) {
    begin(); // Auto-initialize if needed
    
    if (message_label && message) {
        lv_label_set_text(message_label, message);
    }
    
    if (window) {
        lv_obj_clear_flag(window, LV_OBJ_FLAG_HIDDEN);
    }
    
    lv_timer_handler(); // Force immediate update
}

void Spinner::stop() {
    if (!initialized) return;
    
    if (window) {
        lv_obj_add_flag(window, LV_OBJ_FLAG_HIDDEN);
    }
    
    lv_timer_handler(); // Force immediate update
}

void Spinner::message(const char* text) {
    if (!initialized || !message_label || !text) return;
    lv_label_set_text(message_label, text);
    lv_timer_handler();
}

bool Spinner::isRunning() {
    if (!initialized || !window) return false;
    return !lv_obj_has_flag(window, LV_OBJ_FLAG_HIDDEN);
}