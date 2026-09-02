// #ifndef ITEM_SCREEN_H
// #define ITEM_SCREEN_H

// #include <lvgl.h>
// #include <ArduinoJson.h>

// // ================== CONSTANTS ==================
// #define MAX_ITEMS 20
// #define ITEM_ID_LEN 16
// #define ITEM_NAME_LEN 64
// #define ITEM_CURRENCY_LEN 8
// #define ITEM_PRICE_LEN 32
// #define ITEM_INCREMENT_LEN 32
// #define ITEM_TIMER_LEN 32

// // ================== ENUMS ==================
// typedef enum {
//     AUCTION_MODE_UNKNOWN,
//     AUCTION_MODE_ENGLISH,
//     AUCTION_MODE_CLOSED_BID
// } AuctionMode;

// // ================== DATA STRUCTURES ==================
// typedef struct {
//     const char* item_id;
//     const char* name;
//     const char* status;
//     const char* currency;
//     double current_price;
//     double next_min_bid;
//     bool your_bid_submitted;
//     const char* end_datetime;
//     int remaining_seconds;
//     char price_formatted[ITEM_PRICE_LEN];
//     char increment_formatted[ITEM_INCREMENT_LEN];
//     char timer_formatted[ITEM_TIMER_LEN];
// } ItemData;

// // ================== FUNCTION DECLARATIONS ==================
// void init_item_screen(lv_obj_t* parent);
// void show_item_screen();
// void hide_item_screen();
// bool is_item_screen_visible();
// void update_item_display();
// void next_item();
// void prev_item();
// bool handle_item_buttons(int button_id);
// void update_item_timer();

// // Data loading functions
// void load_english_dummy_data();
// void load_closed_dummy_data();
// void load_luxury_dummy_data();
// void load_items_from_mqtt(const JsonArray& items_array, const char* auction_mode, const char* auction_status = "");
// void set_auction_details(const char* auction_id, const char* auction_name, AuctionMode mode);

// // Getter functions
// int get_item_count();
// const char* get_current_item_id();
// double get_current_bid_amount();

// // LVGL Objects - Renamed with item_ prefix to avoid conflicts
// extern lv_obj_t* item_main_card;
// extern lv_obj_t* item_card1;
// extern lv_obj_t* item_card2;
// extern lv_obj_t* item_status_label;
// extern lv_obj_t* item_auction_name_label;
// extern lv_obj_t* item_name_label1;
// extern lv_obj_t* item_name_label2;
// extern lv_obj_t* item_timer_label;
// extern lv_obj_t* item_timer_icon;
// extern lv_obj_t* item_details_label;
// extern lv_obj_t* item_price_label;
// extern lv_obj_t* item_price_icon;
// extern lv_obj_t* item_increment_label;
// extern lv_obj_t* item_increment_icon;
// extern lv_obj_t* item_bid_label;
// extern lv_obj_t* item_bid_icon;
// extern lv_obj_t* item_place_bid_btn;
// extern lv_obj_t* item_place_bid_label;
// extern lv_obj_t* item_arrow_left;
// extern lv_obj_t* item_arrow_right;
// extern lv_obj_t* item_page_indicator;
// extern lv_obj_t* item_loading_label;

// #endif // ITEM_SCREEN_H

#ifndef ITEM_SCREEN_H
#define ITEM_SCREEN_H

#include <lvgl.h>
#include <ArduinoJson.h>

// ================== CONSTANTS ==================
#define MAX_ITEMS 15
#define ITEM_ID_LEN 8
#define ITEM_NAME_LEN 64
#define ITEM_CURRENCY_LEN 8
#define ITEM_PRICE_LEN 16
#define ITEM_INCREMENT_LEN 16
#define ITEM_TIMER_LEN 32

// ================== ENUMS ==================
typedef enum {
    AUCTION_MODE_UNKNOWN,
    AUCTION_MODE_ENGLISH,
    AUCTION_MODE_CLOSED_BID
} AuctionMode;

// ================== DATA STRUCTURES ==================
typedef struct {
    const char* item_id;
    const char* name;
    const char* status;
    const char* currency;
    double current_price;
    double next_min_bid;
    bool your_bid_submitted;
    const char* end_datetime;
    int remaining_seconds;
    char price_formatted[ITEM_PRICE_LEN];
    char increment_formatted[ITEM_INCREMENT_LEN];
    char timer_formatted[ITEM_TIMER_LEN];
} ItemData;

// ================== FUNCTION DECLARATIONS ==================
void init_item_screen(lv_obj_t* parent);
void show_item_screen();
void hide_item_screen();
bool is_item_screen_visible();
void update_item_display();
void next_item();
void prev_item();
bool handle_item_buttons(int button_id);
void update_item_timer();
bool is_bid_popup_active();
void add_to_bid_textarea(const char* text);
void backspace_bid_textarea();
void clear_bid_textarea();
void confirm_bid();
void cancel_bid();
const char* get_current_item_id(); 
// Add this function declaration
void handle_bid_response(const char* status, const char* bidStatus, float currentBid, int reason);
void set_current_auction_id(const char* auction_id);
const char* get_current_auction_id();

// Data loading functions - ONLY MQTT version
void load_items_from_mqtt(const JsonArray& items_array, const char* auction_mode, const char* auction_status = "");
void set_auction_details(const char* auction_name, AuctionMode mode);
void set_nfc_authenticated(bool authenticated, const char* userId = "", const char* role = "");
bool is_nfc_authenticated();
const char* get_nfc_user_id();
const char* get_nfc_user_role();

// Getter functions
int get_item_count();
const char* get_current_item_id();
double get_current_bid_amount();

// LVGL Objects
extern lv_obj_t* item_main_card;
extern lv_obj_t* item_card1;
extern lv_obj_t* item_card2;
extern lv_obj_t* item_status_label;
extern lv_obj_t* item_auction_name_label;
extern lv_obj_t* item_name_label1;
extern lv_obj_t* item_name_label2;
extern lv_obj_t* item_timer_label;
extern lv_obj_t* item_timer_icon;
extern lv_obj_t* item_details_label;
extern lv_obj_t* item_price_label;
extern lv_obj_t* item_price_icon;
extern lv_obj_t* item_increment_label;
extern lv_obj_t* item_increment_icon;
extern lv_obj_t* item_bid_label;
extern lv_obj_t* item_bid_icon;
extern lv_obj_t* item_place_bid_btn;
extern lv_obj_t* item_place_bid_label;
extern lv_obj_t* item_arrow_left;
extern lv_obj_t* item_arrow_right;
extern lv_obj_t* item_page_indicator;
extern lv_obj_t* item_loading_label;

#endif // ITEM_SCREEN_H