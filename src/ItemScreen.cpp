// #include "ItemScreen.h"
// #include "BidMQTT.h"
// #include "NFCMQTT.h"
// #include <cstring>
// #include <cstdio>
// #include <ArduinoJson.h>

// // Declare the external bid object
// extern BidMQTT bid;
// extern const char* get_current_nfc_uid();

// // Simple temp message function
// static void show_temp_message(const char* title, const char* message, uint32_t duration_ms) {
//     lv_obj_t* mbox = lv_msgbox_create(NULL, title, message, NULL, true);
//     lv_obj_center(mbox);
//     lv_timer_handler();
    
//     uint32_t start = millis();
//     while (millis() - start < duration_ms) {
//         lv_timer_handler();
//         delay(10);
//     }
    
//     lv_msgbox_close(mbox);
//     lv_timer_handler();
// }

// LV_FONT_DECLARE(lv_font_montserrat_14);

// // ================== COLORS ==================
// #define COLOR_PRIMARY     lv_color_hex(0x5C9ACF)
// #define COLOR_SUCCESS     lv_color_hex(0x68B382)
// #define COLOR_DANGER      lv_color_hex(0xD96B6B)
// #define COLOR_WARNING     lv_color_hex(0xD19A66)
// #define lv_color_white()        lv_color_hex(0xFFFFFF)
// #define lv_color_white()       lv_color_hex(0x2B2B36)
// #define COLOR_BORDER      lv_color_hex(0x3A3A4A)
// #define COLOR_SELECTED    lv_color_hex(0x3B3B4A)

// // ================== ICONS ==================
// #define ICON_ARROW_LEFT   "\uF053"
// #define ICON_ARROW_RIGHT  "\uF054"
// #define ICON_CLOCK        "\uF0F3"
// #define ICON_TAG          "\uF00B"
// #define ICON_GAVEL        "\uF0E7"
// #define ICON_USER         "\uF007"

// // ================== FUNCTION DECLARATIONS ==================
// static void show_bid_popup();
// static void bid_confirm_cb(lv_event_t* e);
// static void bid_cancel_cb(lv_event_t* e);

// // ================== GLOBALS ==================
// static ItemData item_list[MAX_ITEMS];
// static int item_count = 0;
// static int current_index = 0;
// static AuctionMode current_mode = AUCTION_MODE_UNKNOWN;
// static bool screen_visible = false;
// static bool initialized = false;
// static bool nfc_authenticated = false;
// static char nfc_user_id[32] = "";
// static char nfc_user_role[16] = "";

// // Current and next item data
// static ItemData current_item_data;
// static ItemData next_item_data;

// // Auction details storage
// static char current_auction_name[64] = "";
// static char current_auction_id[32] = "";
// static AuctionMode current_auction_mode = AUCTION_MODE_UNKNOWN;

// // Bid popup globals
// static lv_obj_t* bid_popup = nullptr;
// static lv_obj_t* bid_textarea = nullptr;
// static lv_obj_t* bid_title_label = nullptr;
// static lv_obj_t* bid_min_label = nullptr;
// static lv_obj_t* bid_confirm_btn = nullptr;
// static lv_obj_t* bid_cancel_btn = nullptr;
// static lv_obj_t* modal_overlay = nullptr;
// static bool bid_popup_active = false;

// // Single buffers for current item (reused)
// static char current_id_buf[ITEM_ID_LEN];
// static char current_name_buf[ITEM_NAME_LEN];
// static char current_currency_buf[ITEM_CURRENCY_LEN];
// static char current_datetime_buf[32];

// // Single buffers for next item (reused)
// static char next_id_buf[ITEM_ID_LEN];
// static char next_name_buf[ITEM_NAME_LEN];
// static char next_currency_buf[ITEM_CURRENCY_LEN];
// static char next_datetime_buf[32];

// // LVGL Objects
// lv_obj_t* item_main_card = nullptr;
// lv_obj_t* item_card1 = nullptr;
// lv_obj_t* item_card2 = nullptr;
// lv_obj_t* item_status_label = nullptr;
// lv_obj_t* item_auction_name_label = nullptr;
// lv_obj_t* item_auction_id_label = nullptr;
// lv_obj_t* item_name_label1 = nullptr;
// lv_obj_t* item_name_label2 = nullptr;
// lv_obj_t* item_timer_label = nullptr;
// lv_obj_t* item_timer_icon = nullptr;
// lv_obj_t* item_details_label = nullptr;
// lv_obj_t* item_price_label = nullptr;
// lv_obj_t* item_price_icon = nullptr;
// lv_obj_t* item_increment_label = nullptr;
// lv_obj_t* item_increment_icon = nullptr;
// lv_obj_t* item_bid_label = nullptr;
// lv_obj_t* item_bid_icon = nullptr;
// lv_obj_t* item_place_bid_btn = nullptr;
// lv_obj_t* item_place_bid_label = nullptr;
// lv_obj_t* item_arrow_left = nullptr;
// lv_obj_t* item_arrow_right = nullptr;
// lv_obj_t* item_page_indicator = nullptr;
// lv_obj_t* item_loading_label = nullptr;

// // External NFC UID
// extern String lastNfcUid;

// // ================== HELPER FUNCTIONS ==================
// static void format_price(double price, const char* currency, char* buffer, size_t size) {
//     if (currency && strlen(currency) > 0) {
//         if (price >= 1000) snprintf(buffer, size, "%s %.0f", currency, price);
//         else snprintf(buffer, size, "%s %.2f", currency, price);
//     } else snprintf(buffer, size, "%.2f", price);
// }

// static void format_remaining_time(int seconds, char* buffer, size_t size) {
//     if (seconds < 0) { snprintf(buffer, size, "Ended"); return; }
//     int hours = seconds / 3600;
//     int minutes = (seconds % 3600) / 60;
//     int secs = seconds % 60;
    
//     if (hours > 0) snprintf(buffer, size, "%02dh %02dm", hours, minutes);
//     else snprintf(buffer, size, "%02d:%02d", minutes, secs);
// }

// static void update_arrow_visibility() {
//     if (!item_arrow_left || !item_arrow_right) return;

//     if (item_count <= 1) {
//         // Only 0 or 1 item → hide both arrows
//         lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
//     } else {
//         // Left arrow visible only if we can go back
//         if (current_index > 0)
//             lv_obj_clear_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
//         else
//             lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);

//         // Right arrow visible only if we can go forward
//         if (current_index < item_count - 1)
//             lv_obj_clear_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
//         else
//             lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
//     }
// }

// static void flash_card() {
//     if (item_main_card) {
//         lv_obj_set_style_bg_color(item_main_card, COLOR_SELECTED, 0);
//         lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
//             lv_obj_set_style_bg_color(item_main_card, lv_color_white(), 0);
//             lv_timer_del(t);
//         }, 100, nullptr);
//         lv_timer_set_repeat_count(timer, 1);
//     }
// }

// // ================== INITIALIZATION ==================

// void init_item_screen(lv_obj_t* parent) {
//     if (!parent) return;
    
//     Serial.println("Initializing Item Screen...");
    
//     lv_obj_set_style_pad_all(parent, 0, 0);
//     lv_obj_set_style_bg_color(parent, lv_color_white(), 0);
    
//     // Loading label
//     item_loading_label = lv_label_create(parent);
//     lv_label_set_text(item_loading_label, "Loading Items1...");
//     lv_obj_center(item_loading_label);
//     lv_obj_set_style_text_color(item_loading_label, lv_color_hex(0xFFFFFF), 0);
//     lv_obj_set_style_text_font(item_loading_label, &lv_font_montserrat_14, 0);
//     lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
    
//     // Status label
//     item_status_label = lv_label_create(parent);
//     lv_obj_set_pos(item_status_label, 150, 5);
//     lv_label_set_text(item_status_label, "LIVE");
//     lv_obj_set_style_text_color(item_status_label, COLOR_DANGER, 0);
//     lv_obj_set_style_text_font(item_status_label, &lv_font_montserrat_14, 0);
//     lv_obj_add_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
    
//     // Auction name
//     item_auction_name_label = lv_label_create(parent);
//     lv_obj_set_pos(item_auction_name_label, 20, 5);
//     lv_obj_set_width(item_auction_name_label, 120);
//     lv_label_set_long_mode(item_auction_name_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
//     lv_obj_set_style_text_color(item_auction_name_label, COLOR_DANGER, 0);
//     lv_obj_set_style_text_font(item_auction_name_label, &lv_font_montserrat_14, 0);
//     lv_obj_add_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
    
//     // Auction ID label
//     item_auction_id_label = lv_label_create(parent);
//     lv_obj_set_pos(item_auction_id_label, 20, 25);
//     lv_obj_set_width(item_auction_id_label, 120);
//     lv_obj_set_style_text_color(item_auction_id_label, lv_color_white(), 0);
//     lv_obj_set_style_text_font(item_auction_id_label, &lv_font_montserrat_14, 0);
//     lv_obj_add_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
    
//     // Arrows
//     item_arrow_left = lv_label_create(parent);
//     lv_obj_set_pos(item_arrow_left, 5, 55);
//     lv_label_set_text(item_arrow_left, ICON_ARROW_LEFT);
//     lv_obj_set_style_text_color(item_arrow_left, COLOR_DANGER, 0);
//     lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
    
//     item_arrow_right = lv_label_create(parent);
//     lv_obj_set_pos(item_arrow_right, 222, 55);
//     lv_label_set_text(item_arrow_right, ICON_ARROW_RIGHT);
//     lv_obj_set_style_text_color(item_arrow_right, COLOR_DANGER, 0);
//     lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
    
//     // Top cards
//     item_card1 = lv_obj_create(parent);
//     lv_obj_set_pos(item_card1, 20, 45);
//     lv_obj_set_size(item_card1, 95, 45);
//     lv_obj_set_style_border_color(item_card1, COLOR_BORDER, 0);
//     lv_obj_set_style_border_width(item_card1, 2, 0);
//     lv_obj_set_style_radius(item_card1, 8, 0);
//     lv_obj_set_style_pad_all(item_card1, 0, 0);
//     lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
    
//     item_name_label1 = lv_label_create(item_card1);
//     lv_obj_set_width(item_name_label1, 85);
//     lv_label_set_long_mode(item_name_label1, LV_LABEL_LONG_SCROLL_CIRCULAR);
//     lv_obj_set_style_text_align(item_name_label1, LV_TEXT_ALIGN_CENTER, 0);
//     lv_obj_set_style_text_font(item_name_label1, &lv_font_montserrat_14, 0);
//     lv_obj_center(item_name_label1);
    
//     item_card2 = lv_obj_create(parent);
//     lv_obj_set_pos(item_card2, 125, 45);
//     lv_obj_set_size(item_card2, 95, 45);
//     lv_obj_set_style_border_color(item_card2, COLOR_BORDER, 0);
//     lv_obj_set_style_border_width(item_card2, 2, 0);
//     lv_obj_set_style_radius(item_card2, 8, 0);
//     lv_obj_set_style_pad_all(item_card2, 0, 0);
//     lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
    
//     item_name_label2 = lv_label_create(item_card2);
//     lv_obj_set_width(item_name_label2, 85);
//     lv_label_set_long_mode(item_name_label2, LV_LABEL_LONG_SCROLL_CIRCULAR);
//     lv_obj_set_style_text_align(item_name_label2, LV_TEXT_ALIGN_CENTER, 0);
//     lv_obj_set_style_text_font(item_name_label2, &lv_font_montserrat_14, 0);
//     lv_obj_center(item_name_label2);
    
//     // Page indicator
//     item_page_indicator = lv_label_create(parent);
//     lv_obj_set_pos(item_page_indicator, 100, 95);
//     lv_obj_set_style_text_font(item_page_indicator, &lv_font_montserrat_14, 0);
//     lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
    
//     // Timer
//     item_timer_icon = lv_label_create(parent);
//     lv_obj_set_pos(item_timer_icon, 20, 115);
//     lv_label_set_text(item_timer_icon, ICON_CLOCK);
//     lv_obj_set_style_text_color(item_timer_icon, COLOR_PRIMARY, 0);
//     lv_obj_add_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
    
//     item_timer_label = lv_label_create(parent);
//     lv_obj_set_pos(item_timer_label, 45, 115);
//     lv_obj_set_style_text_font(item_timer_label, &lv_font_montserrat_14, 0);
//     lv_obj_add_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);
    
//     // Main card
//     item_main_card = lv_obj_create(parent);
//     lv_obj_set_pos(item_main_card, 15, 140);
//     lv_obj_set_size(item_main_card, 210, 120);
//     lv_obj_set_style_border_color(item_main_card, COLOR_BORDER, 0);
//     lv_obj_set_style_border_width(item_main_card, 2, 0);
//     lv_obj_set_style_radius(item_main_card, 12, 0);
//     lv_obj_set_style_pad_all(item_main_card, 10, 0);
//     lv_obj_add_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
    
//     item_details_label = lv_label_create(item_main_card);
//     lv_obj_set_pos(item_details_label, 10, 5);
//     lv_obj_set_width(item_details_label, 180);
//     lv_label_set_long_mode(item_details_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
//     lv_obj_set_style_text_color(item_details_label, lv_color_hex(0xFFFFFF), 0);
//     lv_obj_set_style_text_font(item_details_label, &lv_font_montserrat_14, 0);
    
//     // Price
//     item_price_icon = lv_label_create(item_main_card);
//     lv_obj_set_pos(item_price_icon, 10, 30);
//     lv_label_set_text(item_price_icon, ICON_TAG);
//     lv_obj_set_style_text_color(item_price_icon, COLOR_PRIMARY, 0);
    
//     item_price_label = lv_label_create(item_main_card);
//     lv_obj_set_pos(item_price_label, 35, 30);
//     lv_obj_set_style_text_font(item_price_label, &lv_font_montserrat_14, 0);
    
//     // Increment
//     item_increment_icon = lv_label_create(item_main_card);
//     lv_obj_set_pos(item_increment_icon, 10, 55);
//     lv_label_set_text(item_increment_icon, ICON_GAVEL);
//     lv_obj_set_style_text_color(item_increment_icon, COLOR_SUCCESS, 0);
    
//     item_increment_label = lv_label_create(item_main_card);
//     lv_obj_set_pos(item_increment_label, 35, 55);
//     lv_obj_set_style_text_color(item_increment_label, COLOR_SUCCESS, 0);
//     lv_obj_set_style_text_font(item_increment_label, &lv_font_montserrat_14, 0);
    
//     // Bid
//     item_bid_icon = lv_label_create(item_main_card);
//     lv_obj_set_pos(item_bid_icon, 10, 55);
//     lv_label_set_text(item_bid_icon, ICON_USER);
//     lv_obj_set_style_text_color(item_bid_icon, COLOR_WARNING, 0);
//     lv_obj_add_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
    
//     item_bid_label = lv_label_create(item_main_card);
//     lv_obj_set_pos(item_bid_label, 35, 55);
//     lv_obj_set_style_text_color(item_bid_label, COLOR_WARNING, 0);
//     lv_obj_set_style_text_font(item_bid_label, &lv_font_montserrat_14, 0);
//     lv_obj_add_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);
    
//     // Place bid button
//     item_place_bid_btn = lv_btn_create(parent);
//     lv_obj_set_pos(item_place_bid_btn, 45, 240);
//     lv_obj_set_size(item_place_bid_btn, 150, 35);
//     lv_obj_set_style_bg_color(item_place_bid_btn, COLOR_PRIMARY, 0);
//     lv_obj_set_style_radius(item_place_bid_btn, 20, 0);
//     lv_obj_add_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
    
//     item_place_bid_label = lv_label_create(item_place_bid_btn);
//     lv_label_set_text(item_place_bid_label, "PLACE BID");
//     lv_obj_set_style_text_color(item_place_bid_label, lv_color_white(), 0);
//     lv_obj_center(item_place_bid_label);
    
//     hide_item_screen();
//     initialized = true;
//     Serial.println("Item Screen initialized");
// }

// // ================== SCREEN VISIBILITY ==================

// void show_item_screen() {
//     if (!initialized) return;
//     screen_visible = true;
    
//     // Hide all UI elements first, show loading
//     lv_obj_add_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
    
//     if (item_loading_label) {
//         lv_label_set_text(item_loading_label, "Loading Items...");
//         lv_obj_clear_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
//     }
    
//     Serial.println("Item screen shown - loading items");
// }

// void hide_item_screen() {
//     screen_visible = false;
    
//     // Hide all elements
//     lv_obj_add_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_increment_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_increment_icon, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
    
//     if (item_loading_label) {
//         lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
//     }
    
//     Serial.println("Item screen hidden");
// }

// bool is_item_screen_visible() { 
//     return screen_visible; 
// }

// // ================== AUCTION DETAILS FUNCTIONS ==================

// void set_auction_details(const char* auction_name, AuctionMode mode) {
//     if (auction_name) strlcpy(current_auction_name, auction_name, sizeof(current_auction_name));
//     current_auction_mode = mode;
//     current_mode = mode;
    
//     if (item_auction_name_label) {
//         lv_label_set_text(item_auction_name_label, current_auction_name);
//     }
    
//     Serial.printf("📋 Auction details set: %s - Mode: %d\n", 
//                   current_auction_name, mode);
// }

// void set_current_auction_id(const char* auction_id) {
//     if (auction_id) {
//         strlcpy(current_auction_id, auction_id, sizeof(current_auction_id));
        
//         if (item_auction_id_label) {
//             char id_text[48];
//             snprintf(id_text, sizeof(id_text), "ID: %s", current_auction_id);
//             lv_label_set_text(item_auction_id_label, id_text);
//         }
//     }
// }

// const char* get_auction_name() {
//     return current_auction_name;
// }

// const char* get_auction_id() {
//     return current_auction_id;
// }

// AuctionMode get_auction_mode() {
//     return current_auction_mode;
// }
// static void load_current_item_data() {
//     if (current_index < 0 || current_index >= item_count) return;
    
//     // Copy data from item_list to current_item_data
//     current_item_data.item_id = item_list[current_index].item_id;
//     current_item_data.name = item_list[current_index].name;
//     current_item_data.currency = item_list[current_index].currency;
//     current_item_data.current_price = item_list[current_index].current_price;
//     current_item_data.next_min_bid = item_list[current_index].next_min_bid;
//     current_item_data.your_bid_submitted = item_list[current_index].your_bid_submitted;
//     current_item_data.end_datetime = item_list[current_index].end_datetime;
//     current_item_data.remaining_seconds = item_list[current_index].remaining_seconds;
    
//     // Copy formatted strings
//     strlcpy(current_item_data.price_formatted, 
//             item_list[current_index].price_formatted, ITEM_PRICE_LEN);
//     strlcpy(current_item_data.increment_formatted, 
//             item_list[current_index].increment_formatted, ITEM_INCREMENT_LEN);
//     strlcpy(current_item_data.timer_formatted, 
//             item_list[current_index].timer_formatted, ITEM_TIMER_LEN);
    
//     // Load next item data if available
//     if (current_index < item_count - 1) {
//         next_item_data.item_id = item_list[current_index + 1].item_id;
//         next_item_data.name = item_list[current_index + 1].name;
//         next_item_data.currency = item_list[current_index + 1].currency;
//         next_item_data.current_price = item_list[current_index + 1].current_price;
//         next_item_data.next_min_bid = item_list[current_index + 1].next_min_bid;
//         next_item_data.your_bid_submitted = item_list[current_index + 1].your_bid_submitted;
//         next_item_data.end_datetime = item_list[current_index + 1].end_datetime;
//         next_item_data.remaining_seconds = item_list[current_index + 1].remaining_seconds;
        
//         strlcpy(next_item_data.price_formatted, 
//                 item_list[current_index + 1].price_formatted, ITEM_PRICE_LEN);
//         strlcpy(next_item_data.increment_formatted, 
//                 item_list[current_index + 1].increment_formatted, ITEM_INCREMENT_LEN);
//         strlcpy(next_item_data.timer_formatted, 
//                 item_list[current_index + 1].timer_formatted, ITEM_TIMER_LEN);
//     }
// }
// // ================== ITEM LOADING FUNCTIONS ==================

// void load_items_from_mqtt(const JsonArray& items_array, const char* auction_mode, const char* auction_status) {
//     if (!items_array) {
//         Serial.println("ERROR: Invalid JsonArray");
//         return;
//     }
    
//     item_count = min((int)items_array.size(), MAX_ITEMS);
    
//     if (strcmp(auction_mode, "CLOSED") == 0) {
//         current_mode = AUCTION_MODE_CLOSED_BID;
//     } else {
//         current_mode = AUCTION_MODE_ENGLISH;
//     }
    
//     if (item_count == 0) {
//         if (item_loading_label && screen_visible) {
//             lv_label_set_text(item_loading_label, "No Items Available");
//         }
//         return;
//     }
    
//     // Store all items in item_list array
//     for (int i = 0; i < item_count; i++) {
//         JsonObject item = items_array[i];
        
//         // Allocate memory for each item's strings
//         if (item_list[i].item_id == nullptr) {
//             item_list[i].item_id = new char[ITEM_ID_LEN];
//             item_list[i].name = new char[ITEM_NAME_LEN];
//             item_list[i].currency = new char[ITEM_CURRENCY_LEN];
//             item_list[i].end_datetime = new char[32];
//         }
        
//         strlcpy((char*)item_list[i].item_id, item["Item_ID"] | "UNKNOWN", ITEM_ID_LEN);
//         strlcpy((char*)item_list[i].name, item["Name"] | "Unknown", ITEM_NAME_LEN);
//         strlcpy((char*)item_list[i].currency, item["Currency"] | "LKR", ITEM_CURRENCY_LEN);
//         strlcpy((char*)item_list[i].end_datetime, item["End_DateTime"] | "", 32);
        
//         item_list[i].current_price = item["Current_Price"] | 0.0;
//         item_list[i].next_min_bid = item["Next_Min_Bid"] | 0.0;
//         item_list[i].your_bid_submitted = item["Your_Bid_Submitted"] | false;
//         item_list[i].remaining_seconds = item["Remaining_Seconds"] | 0;
        
//         format_price(item_list[i].current_price, item_list[i].currency, 
//                     item_list[i].price_formatted, ITEM_PRICE_LEN);
//         format_price(item_list[i].next_min_bid, item_list[i].currency, 
//                     item_list[i].increment_formatted, ITEM_INCREMENT_LEN);
//         format_remaining_time(item_list[i].remaining_seconds,
//                             item_list[i].timer_formatted, ITEM_TIMER_LEN);
//     }
    
//     //current_index = 0;
//     if (current_index >= item_count) {
//     current_index = item_count - 1;
//     }
//     load_current_item_data(); // Load first item into current_item_data
    
//     if (screen_visible) {
//         if (item_loading_label) {
//             lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
//         }
        
//         lv_obj_clear_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
        
//         update_item_display();
//         lv_timer_handler();
//     }
    
//     Serial.printf("Loaded %d items from MQTT\n", item_count);
// }

// // ===================== ITEM DISPLAY =====================
// void update_item_display() {

//     // -------- No Items --------
//     if (item_count == 0 || current_index >= item_count) {
//         if (item_card1) lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
//         if (item_card2) lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
//         if (item_page_indicator) lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
//         update_arrow_visibility();
//         return;
//     }

//     char buf[128];

//     // =========================================================
//     // ------------------ CARD 1 (CURRENT) ---------------------
//     // =========================================================
//     if (item_card1 && item_name_label1) {

//         lv_obj_clear_flag(item_card1, LV_OBJ_FLAG_HIDDEN);

//         strncpy(buf, current_item_data.item_id, sizeof(buf) - 1);
//         buf[sizeof(buf) - 1] = '\0';
//         lv_label_set_text(item_name_label1, buf);
//         lv_obj_invalidate(item_name_label1);

//         lv_obj_set_style_border_color(item_card1, COLOR_SELECTED, 0);
//         lv_obj_set_style_border_width(item_card1, 3, 0);
//     }

//     // =========================================================
//     // ------------------ CARD 2 (NEXT) ------------------------
//     // =========================================================
//     if (item_card2 && item_name_label2) {

//         int next_index = current_index + 1;

//         if (next_index < item_count) {

//             lv_obj_clear_flag(item_card2, LV_OBJ_FLAG_HIDDEN);

//             next_item_data = item_list[next_index];

//             strncpy(buf, next_item_data.item_id, sizeof(buf) - 1);
//             buf[sizeof(buf) - 1] = '\0';
//             lv_label_set_text(item_name_label2, buf);

//             lv_obj_set_style_border_color(item_card2, COLOR_BORDER, 0);
//             lv_obj_set_style_border_width(item_card2, 2, 0);

//         } else {
//             // No next item → hide second card
//             lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
//         }
//     }

//     // =========================================================
//     // ------------------ ITEM DETAILS -------------------------
//     // =========================================================
//     snprintf(buf, sizeof(buf), "%s - %s",
//              current_item_data.item_id,
//              current_item_data.name);
//     lv_label_set_text(item_details_label, buf);

//     snprintf(buf, sizeof(buf), "Price: %s",
//              current_item_data.price_formatted);
//     lv_label_set_text(item_price_label, buf);

//     // =========================================================
//     // ------------------ AUCTION MODE UI ----------------------
//     // =========================================================
//     if (current_mode == AUCTION_MODE_ENGLISH) {

//         snprintf(buf, sizeof(buf), "Min Bid: +%s",
//                   current_item_data.price_formatted);
//         lv_label_set_text(item_increment_label, buf);

//         lv_obj_clear_flag(item_increment_icon, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_increment_label, LV_OBJ_FLAG_HIDDEN);

//         lv_obj_add_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);

//         lv_label_set_text(item_place_bid_label, "PLACE BID");

//     } else {

//         if (current_item_data.your_bid_submitted) {
//             snprintf(buf, sizeof(buf), "Your Bid: %s",
//                      current_item_data.price_formatted);
//             lv_label_set_text(item_bid_label, buf);
//         } else {
//             lv_label_set_text(item_bid_label, "No bid placed");
//         }

//         lv_obj_add_flag(item_increment_icon, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(item_increment_label, LV_OBJ_FLAG_HIDDEN);

//         lv_obj_clear_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);

//         lv_label_set_text(item_place_bid_label,
//             current_item_data.your_bid_submitted ? "UPDATE BID" : "PLACE BID");
//     }

//     // =========================================================
//     // ------------------ TIMER -------------------------------
//     // =========================================================
//     snprintf(buf, sizeof(buf), "%s remaining",
//              current_item_data.timer_formatted);
//     lv_label_set_text(item_timer_label, buf);

//     lv_obj_set_style_text_color(
//         item_timer_label,
//         (current_item_data.remaining_seconds < 60 &&
//          current_item_data.remaining_seconds > 0)
//             ? COLOR_DANGER
//             : lv_color_white(),
//         0
//     );

//     // =========================================================
//     // ------------------ PAGE INDICATOR -----------------------
//     // =========================================================
//     snprintf(buf, sizeof(buf), "%d/%d",
//              current_index + 1,
//              item_count);
//     lv_label_set_text(item_page_indicator, buf);
//     lv_obj_clear_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);

//     // =========================================================
//     // ------------------ ARROW VISIBILITY ---------------------
//     // =========================================================
//     update_arrow_visibility();
// }
// void next_item() {
//     if (current_index < item_count - 1) {
//         current_index++;
//         load_current_item_data(); // Load the new current item
//         update_item_display();
//         flash_card();
//         Serial.printf("Moved to next item: %d - %s\n", current_index, current_item_data.item_id);
//     }
// }

// void prev_item() {
//     if (current_index > 0) {
//         current_index--;
//         load_current_item_data(); // Load the new current item
//         update_item_display();
//         flash_card();
//         Serial.printf("Moved to previous item: %d - %s\n", current_index, current_item_data.item_id);
//     }
// }
// // Call next_item() / prev_item() from button handlers:
// // e.g., btnRight.pressed() -> next_item();
// //       btnLeft.pressed()  -> prev_item();

// bool handle_item_buttons(int button_id) {
//     if (!screen_visible) return false;
    
//     bool popup_active = is_bid_popup_active();
    
//     if (popup_active) {
//         if (button_id == 2) { // OK
//             confirm_bid();
//         } else if (button_id == 0) { // Left
//             cancel_bid();
//         }
//         return true;
//     }
    
//     switch(button_id) {
//         case 0: prev_item(); return true;
//         case 1: next_item(); return true;
//         case 2:
//             if (item_count > 0) {
//                 show_bid_popup();
//             }
//             return true;
//     }
//     return false;
// }

// void update_item_timer() {
//     if (!screen_visible || item_count == 0) return;
    
//     if (current_item_data.remaining_seconds > 0) {
//         current_item_data.remaining_seconds--;
//         format_remaining_time(current_item_data.remaining_seconds,
//                             current_item_data.timer_formatted, ITEM_TIMER_LEN);
        
//         // Also update the item_list array to keep it in sync
//         item_list[current_index].remaining_seconds = current_item_data.remaining_seconds;
//         strlcpy(item_list[current_index].timer_formatted, 
//                 current_item_data.timer_formatted, ITEM_TIMER_LEN);
        
//         char timer_text[32];
//         snprintf(timer_text, sizeof(timer_text), "%s remaining", 
//                  current_item_data.timer_formatted);
//         lv_label_set_text(item_timer_label, timer_text);
        
//         if (current_item_data.remaining_seconds < 60) {
//             lv_obj_set_style_text_color(item_timer_label, COLOR_DANGER, 0);
//         }
//     }
// }

// // ================== GETTER FUNCTIONS ==================

// int get_item_count() { return item_count; }
// int get_current_index() { return current_index; }
// const char* get_current_item_id() { return current_item_data.item_id; }
// double get_current_bid_amount() { return current_item_data.next_min_bid; }

// // ================== BID POPUP FUNCTIONS ==================

// static void bid_cancel_cb(lv_event_t* e) {
//     // Delete popup first
//     if (bid_popup) {
//         lv_obj_del(bid_popup);
//         bid_popup = nullptr;
//     }
    
//     // Delete overlay
//     if (modal_overlay) {
//         lv_obj_del(modal_overlay);
//         modal_overlay = nullptr;
//     }
    
//     // Reset all bid-related variables
//     bid_textarea = nullptr;
//     bid_title_label = nullptr;
//     bid_min_label = nullptr;
//     bid_confirm_btn = nullptr;
//     bid_cancel_btn = nullptr;
//     bid_popup_active = false;
    
//     Serial.println("Bid cancelled - popup and overlay closed");
// }

// static void bid_confirm_cb(lv_event_t* e) {
//     if (!bid_textarea) return;
    
//     const char* bid_text = lv_textarea_get_text(bid_textarea);
    
//     if (strlen(bid_text) == 0) {
//         lv_label_set_text(bid_title_label, "❌ Error");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
//         lv_label_set_text(bid_min_label, "Please enter a bid amount");
//         return;
//     }
    
//     float bid_amount = atof(bid_text);
//     if (bid_amount <= 0) {
//         lv_label_set_text(bid_title_label, "❌ Error");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
//         lv_label_set_text(bid_min_label, "Please enter a valid amount");
//         return;
//     }
    
//     const char* auction_id = get_auction_name();
//     const char* item_id = get_current_item_id();
//     const char* nfcUid = get_current_nfc_uid();
    
//     if (!auction_id || strlen(auction_id) == 0) {
//         lv_label_set_text(bid_title_label, "❌ Error");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
//         lv_label_set_text(bid_min_label, "No auction selected");
//         return;
//     }
    
//     if (!item_id || strlen(item_id) == 0) {
//         lv_label_set_text(bid_title_label, "❌ Error");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
//         lv_label_set_text(bid_min_label, "No item selected");
//         return;
//     }
    
//     if (current_mode == AUCTION_MODE_ENGLISH) {
//         if (bid_amount < current_item_data.current_price) {
//             char error_msg[64];
//             snprintf(error_msg, sizeof(error_msg), 
//                     "Minimum bid is %s", 
//                     current_item_data.price_formatted);
            
//             lv_label_set_text(bid_title_label, "❌ Invalid Bid");
//             lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
//             lv_label_set_text(bid_min_label, error_msg);
//             return;
//         }
//     }
//     else if (current_mode == AUCTION_MODE_CLOSED_BID) {
//         if (bid_amount <= current_item_data.current_price) {
//             char error_msg[64];
//             snprintf(error_msg, sizeof(error_msg), 
//                     "Bid must be above %s", 
//                     current_item_data.price_formatted);
            
//             lv_label_set_text(bid_title_label, "❌ Invalid Bid");
//             lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
//             lv_label_set_text(bid_min_label, error_msg);
//             return;
//         }
//     }
    
//     lv_label_set_text(bid_title_label, "⏳ Sending...");
//     lv_obj_set_style_text_color(bid_title_label, COLOR_WARNING, 0);
//     lv_label_set_text(bid_min_label, "Please wait");
//     lv_timer_handler();
    
//     static int bid_counter = 0;
//     char message_id[32];
//     snprintf(message_id, sizeof(message_id), "BID_%d_%lu", ++bid_counter, millis() % 10000);
    
//     // Just pass an empty string for NFC UID - no external variable needed
//     //const char* nfcUid = "";
//     Serial.printf("📤 Submitting bid:\n");
//     Serial.printf("  Auction: %s\n", auction_id);
//     Serial.printf("  Item: %s\n", item_id);
//     Serial.printf("  NFC UID: %s\n", nfcUid);
//     Serial.printf("  Amount: %.2f %s\n", bid_amount, current_item_data.currency);
    
//     bool sent = bid.submitBid(
//         auction_id,
//         item_id,
//         nfcUid,  
//         bid_amount,
//         current_item_data.currency,
//         message_id
//     );
    
//     if (!sent) {
//         lv_label_set_text(bid_title_label, "❌ Failed");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
//         lv_label_set_text(bid_min_label, "Failed to send bid");
        
//         lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
//             if (bid_popup) {
//                 lv_obj_del(bid_popup);
//                 bid_popup = nullptr;
//                 modal_overlay = nullptr;
//                 bid_textarea = nullptr;
//                 bid_popup_active = false;
//             }
//             lv_timer_del(t);
//         }, 2000, nullptr);
//     } else {
//         lv_label_set_text(bid_title_label, "⏳ Waiting");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_WARNING, 0);
//         lv_label_set_text(bid_min_label, "Waiting for response...");
        
//         lv_obj_add_state(bid_confirm_btn, LV_STATE_DISABLED);
//         lv_obj_add_state(bid_cancel_btn, LV_STATE_DISABLED);
//     }
// }

// static void show_bid_popup() {
//     if (!screen_visible || item_count == 0 || bid_popup_active) return;
    
//     bid_popup_active = true;
    
//     modal_overlay = lv_obj_create(lv_scr_act());
//     lv_obj_set_size(modal_overlay, LV_HOR_RES, LV_VER_RES);
//     lv_obj_set_pos(modal_overlay, 0, 0);
//     lv_obj_set_style_bg_color(modal_overlay, lv_color_hex(0x000000), 0);
//     lv_obj_set_style_bg_opa(modal_overlay, LV_OPA_50, 0);
//     lv_obj_set_style_border_width(modal_overlay, 0, 0);
//     lv_obj_add_flag(modal_overlay, LV_OBJ_FLAG_CLICKABLE);
    
//     bid_popup = lv_obj_create(lv_scr_act());
//     lv_obj_set_size(bid_popup, 220, 180);
//     lv_obj_center(bid_popup);
//     lv_obj_set_style_bg_color(bid_popup, lv_color_white(), 0);
//     lv_obj_set_style_border_color(bid_popup, COLOR_PRIMARY, 0);
//     lv_obj_set_style_border_width(bid_popup, 2, 0);
//     lv_obj_set_style_radius(bid_popup, 10, 0);
//     lv_obj_set_style_pad_all(bid_popup, 10, 0);
    
//     bid_title_label = lv_label_create(bid_popup);
//     lv_label_set_text(bid_title_label, "Enter Your Bid");
//     lv_obj_set_style_text_color(bid_title_label, COLOR_PRIMARY, 0);
//     lv_obj_set_style_text_font(bid_title_label, &lv_font_montserrat_14, 0);
//     lv_obj_align(bid_title_label, LV_ALIGN_TOP_MID, 0, 5);
    
//     bid_min_label = lv_label_create(bid_popup);
//     lv_obj_set_width(bid_min_label, 200);
//     lv_label_set_long_mode(bid_min_label, LV_LABEL_LONG_WRAP);
//     lv_obj_set_style_text_color(bid_min_label, lv_color_white(), 0);
//     lv_obj_set_style_text_font(bid_min_label, &lv_font_montserrat_14, 0);
//     lv_obj_align(bid_min_label, LV_ALIGN_TOP_MID, 0, 30);
    
//     char min_text[64];
//     if (current_mode == AUCTION_MODE_ENGLISH) {
//         snprintf(min_text, sizeof(min_text), "Min Bid: %s ", 
//              current_item_data.price_formatted);
//      } 
//      //else {
//     //     snprintf(min_text, sizeof(min_text), "Current: %s", 
//     //             current_item_data.price_formatted);
//     // }
//     lv_label_set_text(bid_min_label, min_text);
    
//     bid_textarea = lv_textarea_create(bid_popup);
//     lv_obj_set_size(bid_textarea, 180, 35);
//     lv_obj_align(bid_textarea, LV_ALIGN_TOP_MID, 0, 70);
//     lv_textarea_set_placeholder_text(bid_textarea, "Enter amount");
//     lv_textarea_set_one_line(bid_textarea, true);
//     lv_textarea_set_cursor_pos(bid_textarea, 0);
//     lv_obj_set_style_border_color(bid_textarea, COLOR_BORDER, 0);
//     lv_obj_set_style_border_width(bid_textarea, 1, 0);
//     lv_obj_set_style_radius(bid_textarea, 5, 0);
    
//     bid_confirm_btn = lv_btn_create(bid_popup);
//     lv_obj_set_size(bid_confirm_btn, 70, 30);
//     lv_obj_align(bid_confirm_btn, LV_ALIGN_BOTTOM_RIGHT, -15, -10);
//     lv_obj_set_style_bg_color(bid_confirm_btn, COLOR_SUCCESS, 0);
    
//     lv_obj_t* confirm_label = lv_label_create(bid_confirm_btn);
//     lv_label_set_text(confirm_label, "Bid");
//     lv_obj_center(confirm_label);
    
//     bid_cancel_btn = lv_btn_create(bid_popup);
//     lv_obj_set_size(bid_cancel_btn, 70, 30);
//     lv_obj_align(bid_cancel_btn, LV_ALIGN_BOTTOM_LEFT, 15, -10);
//     lv_obj_set_style_bg_color(bid_cancel_btn, COLOR_DANGER, 0);
    
//     lv_obj_t* cancel_label = lv_label_create(bid_cancel_btn);
//     lv_label_set_text(cancel_label, "Cancel");
//     lv_obj_center(cancel_label);

    
//     lv_obj_add_event_cb(bid_confirm_btn, bid_confirm_cb, LV_EVENT_CLICKED, nullptr);
//     lv_obj_add_event_cb(bid_cancel_btn, bid_cancel_cb, LV_EVENT_CLICKED, nullptr);
    
//     lv_textarea_set_cursor_pos(bid_textarea, 0);
// }

// void handle_bid_response(const char* status, const char* bidStatus, float currentBid, int reason) {
//     if (!bid_popup_active || !bid_popup) return;
    
//     if (strcmp(status, "SUCCESS") == 0) {
//         lv_label_set_text(bid_title_label, "✅ Success");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_SUCCESS, 0);
        
//         if (strcmp(bidStatus, "WINNING") == 0) {
//             lv_label_set_text(bid_min_label, "You are winning!");
//         } else if (strcmp(bidStatus, "OUTBID") == 0) {
//             char msg[64];
//             snprintf(msg, sizeof(msg), "Outbid! Current: %.2f", currentBid);
//             lv_label_set_text(bid_min_label, msg);
//         } else {
//             lv_label_set_text(bid_min_label, "Bid accepted!");
//         }
//     } else {
//         lv_label_set_text(bid_title_label, "❌ Failed");
//         lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
        
//         const char* errorMsg = "Bid failed";
//         switch(reason) {
//             case 1: errorMsg = "Bid too low"; break;
//             case 2: errorMsg = "Auction ended"; break;
//             case 3: errorMsg = "Invalid item"; break;
//             case 4: errorMsg = "Not registered"; break;
//             case 5: errorMsg = "Auction not started"; break;
//         }
//         lv_label_set_text(bid_min_label, errorMsg);
//     }
    
//     lv_obj_clear_state(bid_confirm_btn, LV_STATE_DISABLED);
//     lv_obj_clear_state(bid_cancel_btn, LV_STATE_DISABLED);
    
//     lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
//         // Delete popup
//         if (bid_popup) {
//             lv_obj_del(bid_popup);
//             bid_popup = nullptr;
//         }
        
//         // Delete overlay
//         if (modal_overlay) {
//             lv_obj_del(modal_overlay);
//             modal_overlay = nullptr;
//         }
        
//         // Reset variables
//         bid_textarea = nullptr;
//         bid_title_label = nullptr;
//         bid_min_label = nullptr;
//         bid_confirm_btn = nullptr;
//         bid_cancel_btn = nullptr;
//         bid_popup_active = false;
        
//         lv_timer_del(t);
//     }, 3000, nullptr);
// }
// // ================== BID POPUP HELPER FUNCTIONS ==================

// bool is_bid_popup_active() { return bid_popup_active; }

// void add_to_bid_textarea(const char* text) {
//     if (bid_textarea != nullptr && bid_popup_active) {
//         lv_textarea_add_text(bid_textarea, text);
//     }
// }

// void backspace_bid_textarea() {
//     if (bid_textarea != nullptr && bid_popup_active) {
//         lv_textarea_del_char(bid_textarea);
//     }
// }

// void clear_bid_textarea() {
//     if (bid_textarea != nullptr && bid_popup_active) {
//         lv_textarea_set_text(bid_textarea, "");
//     }
// }

// void confirm_bid() {
//     if (bid_confirm_btn != nullptr && bid_popup_active) {
//         lv_event_send(bid_confirm_btn, LV_EVENT_CLICKED, nullptr);
//     }
// }

// void cancel_bid() {
//     if (bid_popup_active && bid_cancel_btn != nullptr) {
//         lv_event_send(bid_cancel_btn, LV_EVENT_CLICKED, nullptr);
//     }
// }

// const char* get_bid_textarea_text() {
//     if (bid_textarea != nullptr) {
//         return lv_textarea_get_text(bid_textarea);
//     }
//     return nullptr;
// }

// // Add these functions
// void set_nfc_authenticated(bool authenticated, const char* userId, const char* role) {
//     nfc_authenticated = authenticated;
//     if (userId) strlcpy(nfc_user_id, userId, sizeof(nfc_user_id));
//     if (role) strlcpy(nfc_user_role, role, sizeof(nfc_user_role));
    
//     // You could add a small NFC indicator on the screen
//     Serial.printf("NFC Auth: %s, User: %s, Role: %s\n", 
//                   authenticated ? "YES" : "NO", nfc_user_id, nfc_user_role);
// }

// bool is_nfc_authenticated() {
//     return nfc_authenticated;
// }

// const char* get_nfc_user_id() {
//     return nfc_user_id;
// }

// const char* get_nfc_user_role() {
//     return nfc_user_role;
// }


#include "ItemScreen.h"
#include "BidMQTT.h"
#include "NFCMQTT.h"
#include <cstring>
#include <cstdio>
#include <ArduinoJson.h>

// Declare the external bid object
extern const char* get_current_user_name();
extern BidMQTT bid;
extern const char* get_current_nfc_uid();

// Simple temp message function
static void show_temp_message(const char* title, const char* message, uint32_t duration_ms) {
    lv_obj_t* mbox = lv_msgbox_create(NULL, title, message, NULL, true);
    lv_obj_center(mbox);
    lv_timer_handler();
    
    uint32_t start = millis();
    while (millis() - start < duration_ms) {
        lv_timer_handler();
        delay(10);
    }
    
    lv_msgbox_close(mbox);
    lv_timer_handler();
}

LV_FONT_DECLARE(lv_font_montserrat_14);

// ================== COLORS ==================
#define COLOR_PRIMARY     lv_color_hex(0x5C9ACF)
#define COLOR_SUCCESS     lv_color_hex(0x68B382)
#define COLOR_DANGER      lv_color_hex(0xD96B6B)
#define COLOR_WARNING     lv_color_hex(0xD19A66)
#define lv_color_white()        lv_color_hex(0xFFFFFF)
#define lv_color_white()       lv_color_hex(0x2B2B36)
#define COLOR_BORDER      lv_color_hex(0x3A3A4A)
#define COLOR_SELECTED    lv_color_hex(0x3B3B4A)

// ================== ICONS ==================
#define ICON_ARROW_LEFT   LV_SYMBOL_LEFT
#define ICON_ARROW_RIGHT  LV_SYMBOL_RIGHT
#define ICON_CLOCK        LV_SYMBOL_BELL
#define ICON_TAG          "\uF054"
#define ICON_GAVEL        LV_SYMBOL_EDIT
#define ICON_USER         "\uF015"
#define ICON_SUCCESS      "\uF00C"
#define ICON_FAILED       "\uF071"
#define ICON_WAITING      "\uF021"

// ================== FUNCTION DECLARATIONS ==================
static void show_bid_popup();
static void bid_confirm_cb(lv_event_t* e);
static void bid_cancel_cb(lv_event_t* e);

// ================== GLOBALS ==================
static ItemData item_list[MAX_ITEMS];
static int item_count = 0;
static int current_index = 0;
static AuctionMode current_mode = AUCTION_MODE_UNKNOWN;
static bool screen_visible = false;
static bool initialized = false;
static bool nfc_authenticated = false;
static char nfc_user_id[32] = "";
static char nfc_user_role[16] = "";
static char current_username[64] = "";

// Current and next item data
static ItemData current_item_data;
static ItemData next_item_data;

// Auction details storage
static char current_auction_name[64] = "";
static char current_auction_id[32] = "";
static AuctionMode current_auction_mode = AUCTION_MODE_UNKNOWN;

// Bid popup globals
static lv_obj_t* bid_popup = nullptr;
static lv_obj_t* bid_textarea = nullptr;
static lv_obj_t* bid_title_label = nullptr;
static lv_obj_t* bid_item_label = nullptr;
static lv_obj_t* bid_min_label = nullptr;
static lv_obj_t* bid_confirm_btn = nullptr;
static lv_obj_t* bid_cancel_btn = nullptr;
static lv_obj_t* bid_close_btn = nullptr;
static lv_obj_t* modal_overlay = nullptr;
static lv_obj_t* bid_user_label = nullptr; 
static bool bid_popup_active = false;


// Single buffers for current item (reused)
static char current_id_buf[ITEM_ID_LEN];
static char current_name_buf[ITEM_NAME_LEN];
static char current_currency_buf[ITEM_CURRENCY_LEN];
static char current_datetime_buf[32];

// Single buffers for next item (reused)
static char next_id_buf[ITEM_ID_LEN];
static char next_name_buf[ITEM_NAME_LEN];
static char next_currency_buf[ITEM_CURRENCY_LEN];
static char next_datetime_buf[32];

// LVGL Objects
lv_obj_t* item_main_card = nullptr;
lv_obj_t* item_card1 = nullptr;
lv_obj_t* item_card2 = nullptr;
lv_obj_t* item_status_label = nullptr;
lv_obj_t* item_auction_name_label = nullptr;
lv_obj_t* item_auction_id_label = nullptr;
lv_obj_t* item_name_label1 = nullptr;
lv_obj_t* item_name_label2 = nullptr;
lv_obj_t* item_user_fixed_label = nullptr; 
lv_obj_t* item_username_label = nullptr; 
// lv_obj_t* item_timer_label = nullptr;
// lv_obj_t* item_timer_icon = nullptr;
//lv_obj_add_flag(item_username_label, LV_OBJ_FLAG_HIDDEN);
lv_obj_t* item_details_label = nullptr;
lv_obj_t* item_price_label = nullptr;
lv_obj_t* item_price_icon = nullptr;
lv_obj_t* item_increment_label = nullptr;
lv_obj_t* item_increment_icon = nullptr;
lv_obj_t* item_bid_label = nullptr;
lv_obj_t* item_bid_icon = nullptr;
lv_obj_t* item_place_bid_btn = nullptr;
lv_obj_t* item_place_bid_label = nullptr;
lv_obj_t* item_arrow_left = nullptr;
lv_obj_t* item_arrow_right = nullptr;
lv_obj_t* item_page_indicator = nullptr;
lv_obj_t* item_loading_label = nullptr;

// External NFC UID
extern String lastNfcUid;


// ================== HELPER FUNCTIONS ==================
static void format_price(double price, const char* currency, char* buffer, size_t size) {
    if (currency && strlen(currency) > 0) {
        if (price >= 1000) snprintf(buffer, size, "%s %.0f", currency, price);
        else snprintf(buffer, size, "%s %.2f", currency, price);
    } else snprintf(buffer, size, "%.2f", price);
}

static void format_remaining_time(int seconds, char* buffer, size_t size) {
    if (seconds < 0) { snprintf(buffer, size, "Ended"); return; }
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (hours > 0) snprintf(buffer, size, "%02dh %02dm", hours, minutes);
    else snprintf(buffer, size, "%02d:%02d", minutes, secs);
}

static void update_arrow_visibility() {
    if (!item_arrow_left || !item_arrow_right) return;

    if (item_count <= 1) {
        // Only 0 or 1 item → hide both arrows
        lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
    } else {
        // Left arrow visible only if we can go back
        if (current_index > 0)
            lv_obj_clear_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);

        // Right arrow visible only if we can go forward
        if (current_index < item_count - 1)
            lv_obj_clear_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
    }
}

static void flash_card() {
    if (item_main_card) {
        lv_obj_set_style_bg_color(item_main_card, COLOR_SELECTED, 0);
        lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
            lv_obj_set_style_bg_color(item_main_card, lv_color_white(), 0);
            lv_timer_del(t);
        }, 100, nullptr);
        lv_timer_set_repeat_count(timer, 1);
    }
}

// ================== INITIALIZATION ==================

void init_item_screen(lv_obj_t* parent) {
    if (!parent) return;
    
    Serial.println("Initializing Item Screen...");
    
    lv_obj_set_style_pad_all(parent, 0, 0);
    lv_obj_set_style_bg_color(parent, lv_color_white(), 0);
    
    // Loading label
    item_loading_label = lv_label_create(parent);
    lv_label_set_text(item_loading_label, "Loading Items1...");
    lv_obj_center(item_loading_label);
    lv_obj_set_style_text_color(item_loading_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(item_loading_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
    
    // Status label
    item_status_label = lv_label_create(parent);
    lv_obj_set_pos(item_status_label, 150, 5);
    lv_label_set_text(item_status_label, "LIVE");
    lv_obj_set_style_text_color(item_status_label, COLOR_DANGER, 0);
    lv_obj_set_style_text_font(item_status_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
    
    // Auction name
    item_auction_name_label = lv_label_create(parent);
    lv_obj_set_pos(item_auction_name_label, 20, 5);
    lv_obj_set_width(item_auction_name_label, 120);
    lv_label_set_long_mode(item_auction_name_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_color(item_auction_name_label, COLOR_DANGER, 0);
    lv_obj_set_style_text_font(item_auction_name_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
    
    // Auction ID label
    item_auction_id_label = lv_label_create(parent);
    lv_obj_set_pos(item_auction_id_label, 20, 25);
    lv_obj_set_width(item_auction_id_label, 120);
    lv_obj_set_style_text_color(item_auction_id_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(item_auction_id_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
    
    // Arrows
    item_arrow_left = lv_label_create(parent);
    lv_obj_set_pos(item_arrow_left, 5, 55);
    lv_obj_set_style_text_font(item_arrow_left, &lv_font_montserrat_14, 0);
    lv_label_set_text(item_arrow_left, ICON_ARROW_LEFT);
    lv_obj_set_style_text_color(item_arrow_left, COLOR_DANGER, 0);
    lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
    
    item_arrow_right = lv_label_create(parent);
    lv_obj_set_pos(item_arrow_right, 222, 55);
    lv_obj_set_style_text_font(item_arrow_right, &lv_font_montserrat_14, 0);
    lv_label_set_text(item_arrow_right, ICON_ARROW_RIGHT);
    lv_obj_set_style_text_color(item_arrow_right, COLOR_DANGER, 0);
    lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
    
    // Top cards
    item_card1 = lv_obj_create(parent);
    lv_obj_set_pos(item_card1, 20, 45);
    lv_obj_set_size(item_card1, 95, 45);
    lv_obj_set_style_bg_color(item_card1, lv_color_white(), 0);
    lv_obj_set_style_border_color(item_card1, COLOR_BORDER, 0);
    lv_obj_set_style_border_color(item_card1, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(item_card1, 2, 0);
    lv_obj_set_style_radius(item_card1, 8, 0);
    lv_obj_set_style_pad_all(item_card1, 0, 0);
    lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
    
    item_name_label1 = lv_label_create(item_card1);
    lv_obj_set_width(item_name_label1, 85);
    lv_obj_set_style_text_color(item_name_label1, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_long_mode(item_name_label1, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(item_name_label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(item_name_label1, &lv_font_montserrat_14, 0);
    lv_obj_center(item_name_label1);
    
    item_card2 = lv_obj_create(parent);
    lv_obj_set_pos(item_card2, 125, 45);
    lv_obj_set_size(item_card2, 95, 45);
    lv_obj_set_style_bg_color(item_card2, lv_color_white(), 0);
    lv_obj_set_style_border_color(item_card2, COLOR_BORDER, 0);
    lv_obj_set_style_border_color(item_card2, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(item_card2, 2, 0);
    lv_obj_set_style_radius(item_card2, 8, 0);
    lv_obj_set_style_pad_all(item_card2, 0, 0);
    lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
    
    item_name_label2 = lv_label_create(item_card2);
    lv_obj_set_width(item_name_label2, 85);
    lv_obj_set_style_text_color(item_name_label2, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_long_mode(item_name_label2, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(item_name_label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(item_name_label2, &lv_font_montserrat_14, 0);
    lv_obj_center(item_name_label2);
    
    // Page indicator
    item_page_indicator = lv_label_create(parent);
    lv_obj_set_pos(item_page_indicator, 100, 95);
    lv_obj_set_style_text_color(item_page_indicator, lv_color_white(), 0);
    lv_obj_set_style_text_font(item_page_indicator, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
    
    // Timer
    // item_timer_icon = lv_label_create(parent);
    // lv_obj_set_pos(item_timer_icon, 20, 115);
    // lv_label_set_text(item_timer_icon, ICON_CLOCK);
    // lv_obj_set_style_text_color(item_timer_icon, COLOR_PRIMARY, 0);
    // lv_obj_add_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
    
    // item_timer_label = lv_label_create(parent);
    // lv_obj_set_pos(item_timer_label, 45, 115);
    // lv_obj_set_style_text_font(item_timer_label, &lv_font_montserrat_14, 0);
    // lv_obj_add_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);
    item_user_fixed_label = lv_label_create(parent);
    lv_obj_set_pos(item_user_fixed_label, 20, 115);
    lv_label_set_text(item_user_fixed_label, "User:");
    lv_obj_set_style_text_color(item_user_fixed_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(item_user_fixed_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_user_fixed_label, LV_OBJ_FLAG_HIDDEN);

    item_username_label = lv_label_create(parent);
    lv_obj_set_pos(item_username_label, 60, 115);
    lv_obj_set_width(item_username_label, 150);
    lv_label_set_long_mode(item_username_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_color(item_username_label, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(item_username_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_username_label, LV_OBJ_FLAG_HIDDEN);

    
    // Main card
    item_main_card = lv_obj_create(parent);
    lv_obj_set_pos(item_main_card, 15, 140);
    lv_obj_set_size(item_main_card, 210, 120);
    lv_obj_set_style_bg_color(item_main_card, lv_color_white(), 0);
    lv_obj_set_style_border_color(item_main_card, COLOR_BORDER, 0);
    lv_obj_set_style_border_color(item_main_card, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(item_main_card, 2, 0);
    lv_obj_set_style_radius(item_main_card, 12, 0);
    lv_obj_set_style_pad_all(item_main_card, 10, 0);
    lv_obj_add_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
    
    item_details_label = lv_label_create(item_main_card);
    lv_obj_set_pos(item_details_label, 10, 5);
    lv_obj_set_width(item_details_label, 180);
    lv_label_set_long_mode(item_details_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_color(item_details_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(item_details_label, &lv_font_montserrat_14, 0);
    
    // Price
    item_price_icon = lv_label_create(item_main_card);
    lv_obj_set_pos(item_price_icon, 10, 30);
    lv_obj_set_style_text_font(item_price_icon, &lv_font_montserrat_14, 0);
    lv_label_set_text(item_price_icon, ICON_TAG);
    lv_obj_set_style_text_color(item_price_icon, COLOR_PRIMARY, 0);
    
    item_price_label = lv_label_create(item_main_card);
    lv_obj_set_pos(item_price_label, 35, 30);
    lv_obj_set_style_text_font(item_price_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(item_price_label, lv_color_hex(0xFFFFFF), 0);
    
    // Increment
    item_increment_icon = lv_label_create(item_main_card);
    lv_obj_set_pos(item_increment_icon, 10, 55);
    lv_obj_set_style_text_font(item_increment_icon, &lv_font_montserrat_14, 0);
    lv_label_set_text(item_increment_icon, ICON_GAVEL);
    lv_obj_set_style_text_color(item_increment_icon, COLOR_SUCCESS, 0);
    
    item_increment_label = lv_label_create(item_main_card);
    lv_obj_set_pos(item_increment_label, 35, 55);
    lv_obj_set_style_text_color(item_increment_label, COLOR_SUCCESS, 0);
    lv_obj_set_style_text_font(item_increment_label, &lv_font_montserrat_14, 0);
    
    // Bid
    item_bid_icon = lv_label_create(item_main_card);
    lv_obj_set_pos(item_bid_icon, 10, 55);
    lv_obj_set_style_text_font(item_bid_icon, &lv_font_montserrat_14, 0);
    lv_label_set_text(item_bid_icon, ICON_USER);
    lv_obj_set_style_text_color(item_bid_icon, COLOR_WARNING, 0);
    lv_obj_add_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
    
    item_bid_label = lv_label_create(item_main_card);
    lv_obj_set_pos(item_bid_label, 35, 55);
    lv_obj_set_style_text_color(item_bid_label, COLOR_WARNING, 0);
    lv_obj_set_style_text_font(item_bid_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);
    
    // Place bid button
    item_place_bid_btn = lv_btn_create(parent);
    lv_obj_set_pos(item_place_bid_btn, 45, 240);
    lv_obj_set_size(item_place_bid_btn, 150, 35);
    lv_obj_set_style_bg_color(item_place_bid_btn, COLOR_PRIMARY, 0);
    lv_obj_set_style_radius(item_place_bid_btn, 20, 0);
    lv_obj_add_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
    
    item_place_bid_label = lv_label_create(item_place_bid_btn);
    lv_label_set_text(item_place_bid_label, "PLACE BID");
    lv_obj_set_style_text_color(item_place_bid_label, lv_color_white(), 0);
    lv_obj_center(item_place_bid_label);
    
    hide_item_screen();
    initialized = true;
    Serial.println("Item Screen initialized");
}

// ================== SCREEN VISIBILITY ==================

void show_item_screen() {
    if (!initialized) return;
    screen_visible = true;
    
    // Hide all UI elements first, show loading
    lv_obj_add_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_add_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_add_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_user_fixed_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_username_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
    
    if (item_loading_label) {
        lv_label_set_text(item_loading_label, "Loading Items...");
        lv_obj_clear_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
    }
    
    Serial.println("Item screen shown - loading items");
}

void hide_item_screen() {
    screen_visible = false;
    
    // Hide all elements
    lv_obj_add_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_user_fixed_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_username_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_increment_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_increment_icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
    
    if (item_loading_label) {
        lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
    }
    
    Serial.println("Item screen hidden");
}

bool is_item_screen_visible() { 
    return screen_visible; 
}

// ================== AUCTION DETAILS FUNCTIONS ==================

void set_auction_details(const char* auction_name, AuctionMode mode) {
    if (auction_name) strlcpy(current_auction_name, auction_name, sizeof(current_auction_name));
    current_auction_mode = mode;
    current_mode = mode;
    
    if (item_auction_name_label) {
        lv_label_set_text(item_auction_name_label, current_auction_name);
    }
    
    Serial.printf("📋 Auction details set: %s - Mode: %d\n", 
                  current_auction_name, mode);
}

void set_current_auction_id(const char* auction_id) {
    if (auction_id) {
        strlcpy(current_auction_id, auction_id, sizeof(current_auction_id));
        
        if (item_auction_id_label) {
            char id_text[48];
            snprintf(id_text, sizeof(id_text), "ID: %s", current_auction_id);
            lv_label_set_text(item_auction_id_label, id_text);
        }
    }
}

const char* get_auction_name() {
    return current_auction_name;
}

const char* get_auction_id() {
    return current_auction_id;
}

AuctionMode get_auction_mode() {
    return current_auction_mode;
}

static void load_current_item_data() {
    if (current_index < 0 || current_index >= item_count) return;
    
    // Copy data from item_list to current_item_data
    current_item_data.item_id = item_list[current_index].item_id;
    current_item_data.name = item_list[current_index].name;
    current_item_data.currency = item_list[current_index].currency;
    current_item_data.current_price = item_list[current_index].current_price;
    current_item_data.next_min_bid = item_list[current_index].next_min_bid;
    current_item_data.your_bid_submitted = item_list[current_index].your_bid_submitted;
    current_item_data.end_datetime = item_list[current_index].end_datetime;
    current_item_data.remaining_seconds = item_list[current_index].remaining_seconds;
    
    // Copy formatted strings
    strlcpy(current_item_data.price_formatted, 
            item_list[current_index].price_formatted, ITEM_PRICE_LEN);
    strlcpy(current_item_data.increment_formatted, 
            item_list[current_index].increment_formatted, ITEM_INCREMENT_LEN);
    strlcpy(current_item_data.timer_formatted, 
            item_list[current_index].timer_formatted, ITEM_TIMER_LEN);
    
    // Load next item data if available
    if (current_index < item_count - 1) {
        next_item_data.item_id = item_list[current_index + 1].item_id;
        next_item_data.name = item_list[current_index + 1].name;
        next_item_data.currency = item_list[current_index + 1].currency;
        next_item_data.current_price = item_list[current_index + 1].current_price;
        next_item_data.next_min_bid = item_list[current_index + 1].next_min_bid;
        next_item_data.your_bid_submitted = item_list[current_index + 1].your_bid_submitted;
        next_item_data.end_datetime = item_list[current_index + 1].end_datetime;
        next_item_data.remaining_seconds = item_list[current_index + 1].remaining_seconds;
        
        strlcpy(next_item_data.price_formatted, 
                item_list[current_index + 1].price_formatted, ITEM_PRICE_LEN);
        strlcpy(next_item_data.increment_formatted, 
                item_list[current_index + 1].increment_formatted, ITEM_INCREMENT_LEN);
        strlcpy(next_item_data.timer_formatted, 
                item_list[current_index + 1].timer_formatted, ITEM_TIMER_LEN);
    }
}

// ================== ITEM LOADING FUNCTIONS ==================

void load_items_from_mqtt(const JsonArray& items_array, const char* auction_mode, const char* auction_status) {
    if (!items_array) {
        Serial.println("ERROR: Invalid JsonArray");
        return;
    }
    
    item_count = min((int)items_array.size(), MAX_ITEMS);
    
    String modeCheck = String(auction_mode != nullptr ? auction_mode : "");
    modeCheck.toUpperCase();
    if (modeCheck.indexOf("CLOSED") >= 0 || modeCheck.indexOf("TENDER") >= 0 || modeCheck.indexOf("FIXED") >= 0 || modeCheck.indexOf("SEALED") >= 0) {
        current_mode = AUCTION_MODE_CLOSED_BID;
    } else {
        current_mode = AUCTION_MODE_ENGLISH;
    }
    
    if (item_count == 0) {
        if (item_loading_label && screen_visible) {
            String stat = String(auction_status != nullptr ? auction_status : "");
            stat.toLowerCase();
            if (stat == "ended" || stat == "completed" || stat == "finished") {
                lv_label_set_text(item_loading_label, "Auction is finished");
            } else {
                lv_label_set_text(item_loading_label, "Items are not placed yet");
            }
            
            // Make sure the label is visible!
            lv_obj_clear_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
            
            // Hide all other item-related UI elements
            if (item_status_label) lv_obj_add_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
            if (item_auction_name_label) lv_obj_add_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
            if (item_auction_id_label) lv_obj_add_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
            if (item_card1) lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
            if (item_card2) lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
            if (item_main_card) lv_obj_add_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
            if (item_place_bid_btn) lv_obj_add_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
            if (item_arrow_left) lv_obj_add_flag(item_arrow_left, LV_OBJ_FLAG_HIDDEN);
            if (item_arrow_right) lv_obj_add_flag(item_arrow_right, LV_OBJ_FLAG_HIDDEN);
            if (item_page_indicator) lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
            if (item_increment_label) lv_obj_add_flag(item_increment_label, LV_OBJ_FLAG_HIDDEN);
            if (item_increment_icon) lv_obj_add_flag(item_increment_icon, LV_OBJ_FLAG_HIDDEN);
            if (item_bid_label) lv_obj_add_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);
            if (item_bid_icon) lv_obj_add_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
            if (item_user_fixed_label) lv_obj_add_flag(item_user_fixed_label, LV_OBJ_FLAG_HIDDEN);
            if (item_username_label) lv_obj_add_flag(item_username_label, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }
    
    // Store all items in item_list array
    for (int i = 0; i < item_count; i++) {
        JsonObject item = items_array[i];
        
        // Allocate memory for each item's strings
        if (item_list[i].item_id == nullptr) {
            item_list[i].item_id = new char[ITEM_ID_LEN];
            item_list[i].name = new char[ITEM_NAME_LEN];
            item_list[i].currency = new char[ITEM_CURRENCY_LEN];
            item_list[i].end_datetime = new char[32];
        }
        
        strlcpy((char*)item_list[i].item_id, item["Item_ID"] | "UNKNOWN", ITEM_ID_LEN);
        strlcpy((char*)item_list[i].name, item["Name"] | "Unknown", ITEM_NAME_LEN);
        strlcpy((char*)item_list[i].currency, item["Currency"] | "$", ITEM_CURRENCY_LEN);
        strlcpy((char*)item_list[i].end_datetime, item["End_DateTime"] | "", 32);
        
        item_list[i].current_price = item["Current_Price"] | 0.0;
        item_list[i].next_min_bid = item["Next_Min_Bid"] | 0.0;
        item_list[i].your_bid_submitted = item["Your_Bid_Submitted"] | false;
        item_list[i].remaining_seconds = item["Remaining_Seconds"] | 0;
        
        format_price(item_list[i].current_price, item_list[i].currency, 
                    item_list[i].price_formatted, ITEM_PRICE_LEN);
        format_price(item_list[i].next_min_bid, item_list[i].currency, 
                    item_list[i].increment_formatted, ITEM_INCREMENT_LEN);
        format_remaining_time(item_list[i].remaining_seconds,
                            item_list[i].timer_formatted, ITEM_TIMER_LEN);
    }
    
    if (current_index >= item_count) {
        current_index = item_count - 1;
    }
    load_current_item_data(); // Load first item into current_item_data
    
    if (screen_visible) {
        if (item_loading_label) {
            lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        lv_obj_clear_flag(item_status_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_auction_name_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_auction_id_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
        // lv_obj_clear_flag(item_timer_icon, LV_OBJ_FLAG_HIDDEN);
        // lv_obj_clear_flag(item_timer_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(item_user_fixed_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_username_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_main_card, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_place_bid_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
        
        update_item_display();
        lv_timer_handler();
    }
    
    Serial.printf("Loaded %d items from MQTT\n", item_count);
}

// ===================== ITEM DISPLAY =====================
void update_item_display() {
    // -------- No Items --------
    if (item_count == 0 || current_index >= item_count) {
        if (item_card1) lv_obj_add_flag(item_card1, LV_OBJ_FLAG_HIDDEN);
        if (item_card2) lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
        if (item_page_indicator) lv_obj_add_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);
        update_arrow_visibility();
        return;
    }

    char buf[128];

    // =========================================================
    // ------------------ CARD 1 (CURRENT) ---------------------
    // =========================================================
    if (item_card1 && item_name_label1) {
        lv_obj_clear_flag(item_card1, LV_OBJ_FLAG_HIDDEN);

        strncpy(buf, current_item_data.item_id, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        lv_label_set_text(item_name_label1, buf);
        lv_obj_invalidate(item_name_label1);

        lv_obj_set_style_border_color(item_card1, COLOR_SELECTED, 0);
        lv_obj_set_style_border_width(item_card1, 3, 0);
    }

    // =========================================================
    // ------------------ CARD 2 (NEXT) ------------------------
    // =========================================================
    if (item_card2 && item_name_label2) {
        int next_index = current_index + 1;

        if (next_index < item_count) {
            lv_obj_clear_flag(item_card2, LV_OBJ_FLAG_HIDDEN);

            next_item_data = item_list[next_index];

            strncpy(buf, next_item_data.item_id, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            lv_label_set_text(item_name_label2, buf);

            lv_obj_set_style_border_color(item_card2, COLOR_BORDER, 0);
            lv_obj_set_style_border_width(item_card2, 2, 0);
        } else {
            // No next item → hide second card
            lv_obj_add_flag(item_card2, LV_OBJ_FLAG_HIDDEN);
        }
    }

    // =========================================================
    // ------------------ ITEM DETAILS -------------------------
    // =========================================================
    snprintf(buf, sizeof(buf), "%s - %s",
             current_item_data.item_id,
             current_item_data.name);
    lv_label_set_text(item_details_label, buf);

    snprintf(buf, sizeof(buf), "Price: %s",
             current_item_data.price_formatted);
    lv_label_set_text(item_price_label, buf);

    // =========================================================
    // ------------------ AUCTION MODE UI ----------------------
    // =========================================================
    if (current_mode == AUCTION_MODE_ENGLISH) {
        if (current_item_data.your_bid_submitted) {
            snprintf(buf, sizeof(buf), "My Bid: %s", current_item_data.increment_formatted);
        } else {
            snprintf(buf, sizeof(buf), "My Bid: --");
        }
        lv_label_set_text(item_bid_label, buf);

        lv_obj_clear_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(item_increment_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(item_increment_label, LV_OBJ_FLAG_HIDDEN);

        lv_label_set_text(item_place_bid_label, "PLACE BID");
    } else {
        if (current_item_data.your_bid_submitted) {
            snprintf(buf, sizeof(buf), "Your Bid: %s",
                     current_item_data.increment_formatted);
            lv_label_set_text(item_bid_label, buf);
        } else {
            lv_label_set_text(item_bid_label, "No bid placed");
        }

        lv_obj_add_flag(item_increment_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(item_increment_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(item_bid_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item_bid_label, LV_OBJ_FLAG_HIDDEN);

        lv_label_set_text(item_place_bid_label,
            current_item_data.your_bid_submitted ? "UPDATE BID" : "PLACE BID");
    }

    // =========================================================
    // ------------------ TIMER -------------------------------
    // =========================================================
    // snprintf(buf, sizeof(buf), "%s remaining",
    //          current_item_data.timer_formatted);
    // lv_label_set_text(item_timer_label, buf);

    // lv_obj_set_style_text_color(
    //     item_timer_label,
    //     (current_item_data.remaining_seconds < 60 &&
    //      current_item_data.remaining_seconds > 0)
    //         ? COLOR_DANGER
    //         : lv_color_white(),
    //     0
    // );

    // Username display with sliding effect
    const char* userName = get_current_user_name();
    if (userName && strlen(userName) > 0) {
        lv_label_set_text(item_username_label, userName);
    } else {
        lv_label_set_text(item_username_label, "Guest");
    }
    lv_label_set_long_mode(item_username_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(item_username_label, 150);

    // =========================================================
    // ------------------ PAGE INDICATOR -----------------------
    // =========================================================
    snprintf(buf, sizeof(buf), "%d/%d",
             current_index + 1,
             item_count);
    lv_label_set_text(item_page_indicator, buf);
    lv_obj_clear_flag(item_page_indicator, LV_OBJ_FLAG_HIDDEN);

    // =========================================================
    // ------------------ ARROW VISIBILITY ---------------------
    // =========================================================
    update_arrow_visibility();
}

void next_item() {
    if (current_index < item_count - 1) {
        current_index++;
        load_current_item_data(); // Load the new current item
        update_item_display();
        flash_card();
        Serial.printf("Moved to next item: %d - %s\n", current_index, current_item_data.item_id);
    }
}

void prev_item() {
    if (current_index > 0) {
        current_index--;
        load_current_item_data(); // Load the new current item
        update_item_display();
        flash_card();
        Serial.printf("Moved to previous item: %d - %s\n", current_index, current_item_data.item_id);
    }
}

bool handle_item_buttons(int button_id) {
    if (!screen_visible) return false;
    
    bool popup_active = is_bid_popup_active();
    
    if (popup_active) {
        if (button_id == 2) { // OK
            confirm_bid();
        } else if (button_id == 0) { // Left
            cancel_bid();
        }
        return true;
    }
    
    switch(button_id) {
        case 0: prev_item(); return true;
        case 1: next_item(); return true;
        case 2:
            if (item_count > 0) {
                show_bid_popup();
            }
            return true;
    }
    return false;
}

// ================== GETTER FUNCTIONS ==================

int get_item_count() { return item_count; }
int get_current_index() { return current_index; }
const char* get_current_item_id() { return current_item_data.item_id; }
double get_current_bid_amount() { return current_item_data.next_min_bid; }

// ================== BID POPUP FUNCTIONS ==================


static void show_bid_popup() {
    if (!screen_visible || item_count == 0 || bid_popup_active) return;
    
    bid_popup_active = true;
    
    modal_overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(modal_overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(modal_overlay, 0, 0);
    lv_obj_set_style_bg_color(modal_overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(modal_overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(modal_overlay, 0, 0);
    lv_obj_add_flag(modal_overlay, LV_OBJ_FLAG_CLICKABLE);
    
    // Adjust height based on auction mode (increase height for user name)
    // Adjust height based on auction mode (increase height for user name and individual scrolling labels)
    int popup_height = current_mode == AUCTION_MODE_ENGLISH ? 180 : 225;
    bid_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bid_popup, 220, popup_height);
    lv_obj_center(bid_popup);
    lv_obj_set_style_bg_color(bid_popup, lv_color_white(), 0);
    lv_obj_set_style_border_color(bid_popup, COLOR_PRIMARY, 0);
    lv_obj_set_style_border_width(bid_popup, 2, 0);
    lv_obj_set_style_radius(bid_popup, 10, 0);
    lv_obj_set_style_pad_all(bid_popup, 10, 0);
    
    // ✅ Add user name label at the top with circular scrolling
    bid_user_label = lv_label_create(bid_popup);
    lv_obj_set_width(bid_user_label, 190);
    lv_label_set_long_mode(bid_user_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(bid_user_label, LV_TEXT_ALIGN_CENTER, 0);
    const char* userName = get_current_user_name();
    if (userName && strlen(userName) > 0) {
        char user_text[64];
        snprintf(user_text, sizeof(user_text), "%s", userName);
        lv_label_set_text(bid_user_label, user_text);
    } else {
        lv_label_set_text(bid_user_label, "Bidder");
    }
    lv_obj_set_style_text_color(bid_user_label, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(bid_user_label, &lv_font_montserrat_14, 0);
    lv_obj_align(bid_user_label, LV_ALIGN_TOP_MID, 0, 2);
    
    bid_title_label = lv_label_create(bid_popup);
    lv_obj_set_width(bid_title_label, 190);
    lv_label_set_long_mode(bid_title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(bid_title_label, LV_TEXT_ALIGN_CENTER, 0);
    if (current_mode == AUCTION_MODE_ENGLISH) {
        lv_label_set_text(bid_title_label, "Confirm Your Bid");
    } else {
        lv_label_set_text(bid_title_label, "Enter Your Bid");
    }
    lv_obj_set_style_text_color(bid_title_label, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(bid_title_label, &lv_font_montserrat_14, 0);
    lv_obj_align(bid_title_label, LV_ALIGN_TOP_MID, 0, 22);
    
    // ✅ Dedicated scrolling item name label
    bid_item_label = lv_label_create(bid_popup);
    lv_obj_set_width(bid_item_label, 190);
    lv_label_set_long_mode(bid_item_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(bid_item_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(bid_item_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(bid_item_label, &lv_font_montserrat_14, 0);
    char item_name_str[128];
    snprintf(item_name_str, sizeof(item_name_str), "Item: %s", current_item_data.name);
    lv_label_set_text(bid_item_label, item_name_str);

    bid_min_label = lv_label_create(bid_popup);
    lv_obj_set_width(bid_min_label, 190);
    lv_label_set_long_mode(bid_min_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(bid_min_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(bid_min_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(bid_min_label, &lv_font_montserrat_14, 0);
    
    char min_text[64];
    snprintf(min_text, sizeof(min_text), "Min Bid: %s", current_item_data.price_formatted);
    lv_label_set_text(bid_min_label, min_text);

    if (current_mode == AUCTION_MODE_ENGLISH) {
        lv_obj_align(bid_item_label, LV_ALIGN_TOP_MID, 0, 46);
        lv_obj_align(bid_min_label, LV_ALIGN_TOP_MID, 0, 70);
    } else {
        lv_obj_align(bid_item_label, LV_ALIGN_TOP_MID, 0, 42);
        lv_obj_align(bid_min_label, LV_ALIGN_TOP_MID, 0, 62);
        
        // Create textarea only for closed bid auctions
        bid_textarea = lv_textarea_create(bid_popup);
        lv_obj_set_size(bid_textarea, 180, 35);
        lv_obj_align(bid_textarea, LV_ALIGN_TOP_MID, 0, 86);
        lv_textarea_set_placeholder_text(bid_textarea, "Enter amount");
        lv_textarea_set_one_line(bid_textarea, true);
        lv_textarea_set_cursor_pos(bid_textarea, 0);
        lv_obj_set_style_border_color(bid_textarea, COLOR_BORDER, 0);
        lv_obj_set_style_border_width(bid_textarea, 1, 0);
        lv_obj_set_style_radius(bid_textarea, 5, 0);
    }
    
    bid_confirm_btn = lv_btn_create(bid_popup);
    lv_obj_set_size(bid_confirm_btn, 70, 30);
    lv_obj_align(bid_confirm_btn, LV_ALIGN_BOTTOM_RIGHT, -15, -10);
    lv_obj_set_style_bg_color(bid_confirm_btn, COLOR_SUCCESS, 0);
    
    lv_obj_t* confirm_label = lv_label_create(bid_confirm_btn);
    lv_label_set_text(confirm_label, "Bid");
    lv_obj_set_style_text_color(confirm_label, lv_color_white(), 0);
    lv_obj_center(confirm_label);
    
    bid_cancel_btn = lv_btn_create(bid_popup);
    lv_obj_set_size(bid_cancel_btn, 70, 30);
    lv_obj_align(bid_cancel_btn, LV_ALIGN_BOTTOM_LEFT, 15, -10);
    lv_obj_set_style_bg_color(bid_cancel_btn, COLOR_DANGER, 0);
    
    lv_obj_t* cancel_label = lv_label_create(bid_cancel_btn);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_set_style_text_color(cancel_label, lv_color_white(), 0);
    lv_obj_center(cancel_label);

    bid_close_btn = lv_btn_create(bid_popup);
    lv_obj_set_size(bid_close_btn, 40, 40);
    lv_obj_align(bid_close_btn, LV_ALIGN_BOTTOM_LEFT, 15, -10);
    lv_obj_set_style_bg_color(bid_close_btn, COLOR_DANGER, 0);
    lv_obj_set_style_radius(bid_close_btn, LV_RADIUS_CIRCLE, 0);
    
    lv_obj_t* close_label_btn = lv_label_create(bid_close_btn);
    lv_label_set_text(close_label_btn, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(close_label_btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(close_label_btn);
    
    lv_obj_add_flag(bid_close_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(bid_close_btn, bid_cancel_cb, LV_EVENT_CLICKED, nullptr);
    
    lv_obj_add_event_cb(bid_confirm_btn, bid_confirm_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(bid_cancel_btn, bid_cancel_cb, LV_EVENT_CLICKED, nullptr);
    
    if (bid_textarea) {
        lv_textarea_set_cursor_pos(bid_textarea, 0);
    }
}

static void bid_cancel_cb(lv_event_t* e) {
    // Delete popup first
    if (bid_popup) {
        lv_obj_del(bid_popup);
        bid_popup = nullptr;
    }
    
    // Delete overlay
    if (modal_overlay) {
        lv_obj_del(modal_overlay);
        modal_overlay = nullptr;
    }
    
    // Reset all bid-related variables
    bid_textarea = nullptr;
    bid_title_label = nullptr;
    bid_min_label = nullptr;
    bid_item_label = nullptr;
    bid_confirm_btn = nullptr;
    bid_user_label = nullptr;
    bid_cancel_btn = nullptr;
    bid_popup_active = false;
    
    Serial.println("Bid cancelled - popup and overlay closed");
}

static void bid_confirm_cb(lv_event_t* e) {
    float bid_amount;
    
    if (current_mode == AUCTION_MODE_ENGLISH) {
        // For English auction, use the next_min_bid value
        bid_amount = current_item_data.current_price;
    } else {
        // For closed bid, get from textarea
        if (!bid_textarea) return;
        
        const char* bid_text = lv_textarea_get_text(bid_textarea);
        
        if (strlen(bid_text) == 0) {
            lv_label_set_text(bid_title_label, ICON_FAILED " Error");
            lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
            lv_label_set_text(bid_min_label, "Please enter a bid amount");
            return;
        }
        
        bid_amount = atof(bid_text);
        if (bid_amount <= 0) {
            lv_label_set_text(bid_title_label, ICON_FAILED " Error");
            lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
            lv_label_set_text(bid_min_label, "Please enter a valid amount");
            return;
        }
        
        // Validate closed bid
        if (bid_amount <= current_item_data.current_price) {
            char error_msg[64];
            snprintf(error_msg, sizeof(error_msg), 
                    "Bid must be above %s", 
                    current_item_data.price_formatted);
            
            lv_label_set_text(bid_title_label, ICON_FAILED " Invalid Bid");
            lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
            lv_label_set_text(bid_min_label, error_msg);
            return;
        }
    }
    
    const char* auction_id = get_auction_name();
    const char* item_id = get_current_item_id();
    const char* nfcUid = get_current_nfc_uid();
    
    if (!auction_id || strlen(auction_id) == 0) {
        lv_label_set_text(bid_title_label, ICON_FAILED " Error");
        lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
        lv_label_set_text(bid_min_label, "No auction selected");
        return;
    }
    
    if (!item_id || strlen(item_id) == 0) {
        lv_label_set_text(bid_title_label, ICON_FAILED " Error");
        lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
        lv_label_set_text(bid_min_label, "No item selected");
        return;
    }
    
    lv_label_set_text(bid_title_label, ICON_WAITING " Sending...");
    lv_obj_set_style_text_color(bid_title_label, COLOR_WARNING, 0);
    lv_label_set_text(bid_min_label, "Please wait");
    lv_timer_handler();
    
    static int bid_counter = 0;
    char message_id[32];
    snprintf(message_id, sizeof(message_id), "BID_%d_%lu", ++bid_counter, millis() % 10000);
    
    Serial.printf("📤 Submitting bid:\n");
    Serial.printf("  Auction: %s\n", auction_id);
    Serial.printf("  Item: %s\n", item_id);
    Serial.printf("  NFC UID: %s\n", nfcUid);
    Serial.printf("  Amount: %.2f %s\n", bid_amount, current_item_data.currency);
    
    bool sent = bid.submitBid(
        auction_id,
        item_id,
        nfcUid,  
        bid_amount,
        current_item_data.currency,
        message_id
    );
    
    if (!sent) {
        lv_label_set_text(bid_title_label, ICON_FAILED " Failed");
        lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
        lv_label_set_text(bid_min_label, "Failed to send bid");
        
        lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
            if (bid_popup) {
                lv_obj_del(bid_popup);
                bid_popup = nullptr;
                modal_overlay = nullptr;
                bid_textarea = nullptr;
                bid_popup_active = false;
            }
            lv_timer_del(t);
        }, 2000, nullptr);
    } else {
        lv_label_set_text(bid_title_label, ICON_WAITING " Waiting");
        lv_obj_set_style_text_color(bid_title_label, COLOR_WARNING, 0);
        lv_label_set_text(bid_min_label, "Waiting for response...");
        
        lv_obj_add_flag(bid_confirm_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bid_cancel_btn, LV_OBJ_FLAG_HIDDEN);
        if (bid_close_btn) lv_obj_clear_flag(bid_close_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

void handle_bid_response(const char* status, const char* bidStatus, float currentBid, int reason) {
    if (!bid_popup_active || !bid_popup) return;
    
    if (strcmp(status, "SUCCESS") == 0) {
        lv_label_set_text(bid_title_label, ICON_SUCCESS " Success");
        lv_obj_set_style_text_color(bid_title_label, COLOR_SUCCESS, 0);
        
        if (strcmp(bidStatus, "WINNING") == 0) {
            lv_label_set_text(bid_min_label, "You are winning!");
        } else if (strcmp(bidStatus, "OUTBID") == 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Outbid! Current: %.2f", currentBid);
            lv_label_set_text(bid_min_label, msg);
        } else {
            lv_label_set_text(bid_min_label, "Bid accepted!");
        }
    } else {
        lv_label_set_text(bid_title_label, ICON_FAILED " Failed");
        lv_obj_set_style_text_color(bid_title_label, COLOR_DANGER, 0);
        
        const char* errorMsg = "Bid failed";
        switch(reason) {
            case 1: errorMsg = "Bid too low"; break;
            case 2: errorMsg = "Auction ended"; break;
            case 3: errorMsg = "Invalid item"; break;
            case 4: errorMsg = "Not registered"; break;
            case 5: errorMsg = "Auction not started"; break;
        }
        lv_label_set_text(bid_min_label, errorMsg);
    }
    
    // Buttons are now handled via hiding/showing
    
    lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
        // Delete popup
        if (bid_popup) {
            lv_obj_del(bid_popup);
            bid_popup = nullptr;
        }
        
        // Delete overlay
        if (modal_overlay) {
            lv_obj_del(modal_overlay);
            modal_overlay = nullptr;
        }
        
        // Reset variables
        bid_textarea = nullptr;
        bid_title_label = nullptr;
        bid_min_label = nullptr;
        bid_item_label = nullptr;
        bid_confirm_btn = nullptr;
        bid_cancel_btn = nullptr;
        bid_user_label = nullptr;
        bid_popup_active = false;
        
        lv_timer_del(t);
    }, 5000, nullptr);
}

// ================== BID POPUP HELPER FUNCTIONS ==================

bool is_bid_popup_active() { return bid_popup_active; }

void add_to_bid_textarea(const char* text) {
    // Only allow input for closed bid auctions
    if (current_mode != AUCTION_MODE_ENGLISH && 
        bid_textarea != nullptr && 
        bid_popup_active) {
        lv_textarea_add_text(bid_textarea, text);
    }
}

void backspace_bid_textarea() {
    // Only allow input for closed bid auctions
    if (current_mode != AUCTION_MODE_ENGLISH && 
        bid_textarea != nullptr && 
        bid_popup_active) {
        lv_textarea_del_char(bid_textarea);
    }
}

void clear_bid_textarea() {
    // Only allow input for closed bid auctions
    if (current_mode != AUCTION_MODE_ENGLISH && 
        bid_textarea != nullptr && 
        bid_popup_active) {
        lv_textarea_set_text(bid_textarea, "");
    }
}

void confirm_bid() {
    if (bid_confirm_btn != nullptr && bid_popup_active) {
        lv_event_send(bid_confirm_btn, LV_EVENT_CLICKED, nullptr);
    }
}

void cancel_bid() {
    if (bid_popup_active && bid_cancel_btn != nullptr) {
        lv_event_send(bid_cancel_btn, LV_EVENT_CLICKED, nullptr);
    }
}

const char* get_bid_textarea_text() {
    if (bid_textarea != nullptr) {
        return lv_textarea_get_text(bid_textarea);
    }
    return nullptr;
}

// Add these functions
void set_nfc_authenticated(bool authenticated, const char* userId, const char* role) {
    nfc_authenticated = authenticated;
    if (userId) strlcpy(nfc_user_id, userId, sizeof(nfc_user_id));
    if (role) strlcpy(nfc_user_role, role, sizeof(nfc_user_role));
    
    // You could add a small NFC indicator on the screen
    Serial.printf("NFC Auth: %s, User: %s, Role: %s\n", 
                  authenticated ? "YES" : "NO", nfc_user_id, nfc_user_role);
}

bool is_nfc_authenticated() {
    return nfc_authenticated;
}

const char* get_nfc_user_id() {
    return nfc_user_id;
}

const char* get_nfc_user_role() {
    return nfc_user_role;
}