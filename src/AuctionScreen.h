// #ifndef AUCTIONSCREEN_H
// #define AUCTIONSCREEN_H

// #include <lvgl.h>
// #include <Arduino.h>
// #include "AuctionMQTT.h"

// // ================== CONFIG ==================
// #define MAX_AUCTIONS 20

// // ================== AUCTION STRUCT FOR DISPLAY ==================
// struct AuctionDisplay {
//     const char* name;
//     const char* id;
//     const char* status;
//     const char* start_datetime;
//     const char* mode; 
//     int items_count;
//     int registered_count;
// };

// // ================== GLOBALS ==================
// extern AuctionDisplay auction_list[MAX_AUCTIONS];
// extern int auction_count;
// extern int current_index;

// extern lv_obj_t* main_card;
// extern lv_obj_t* label_name;
// extern lv_obj_t* label_id;
// extern lv_obj_t* label_status_icon;
// extern lv_obj_t* label_datetime;
// extern lv_obj_t* label_mode;           // Added for mode
// extern lv_obj_t* label_mode_icon; // Added for mode icon
// extern lv_obj_t* label_items;
// extern lv_obj_t* label_users;
// extern lv_obj_t* arrow_up;
// extern lv_obj_t* arrow_down;
// extern lv_obj_t* page_indicator;
// extern lv_obj_t* loading_label;

// // ================== FUNCTIONS ==================
// void create_auction_display(lv_obj_t* content_area);
// void refresh_display();
// void show_loading();
// void hide_loading();
// void update_auctions_from_mqtt(Auction* mqtt_auctions, int count);
// void next_auction();
// void prev_auction();
// void show_custom_loading(const char* message);
// void update_arrow_visibility();
// const char* print_current_auction_details();

// #endif // AUCTIONSCREEN_H

#ifndef AUCTIONSCREEN_H
#define AUCTIONSCREEN_H

#include <lvgl.h>
#include <Arduino.h>
#include "AuctionMQTT.h"

// ================== CONFIG ==================
#define MAX_AUCTIONS 10

// ================== AUCTION STRUCT FOR DISPLAY ==================
struct AuctionDisplay {
    const char* name;
    const char* id;
    const char* status;
    const char* mode;
    const char* start_datetime;
    const char* end_datetime;
    int items_count;
    int registered_count;
};

// ================== GLOBALS ==================
extern AuctionDisplay auction_list[MAX_AUCTIONS];
extern int auction_count;
extern int current_index;

extern lv_obj_t* main_card;
extern lv_obj_t* label_name;
extern lv_obj_t* label_id;
extern lv_obj_t* label_status_icon;
extern lv_obj_t* label_datetime;
extern lv_obj_t* label_end_datetime;
extern lv_obj_t* label_mode;
extern lv_obj_t* label_mode_icon;
extern lv_obj_t* label_status_text;
extern lv_obj_t* label_items;
extern lv_obj_t* label_users;
extern lv_obj_t* arrow_up;
extern lv_obj_t* arrow_down;
extern lv_obj_t* page_indicator;
extern lv_obj_t* loading_label;

// ================== FUNCTIONS ==================
void create_auction_display(lv_obj_t* content_area);
void refresh_display();
void show_loading();
void hide_loading();
void update_auctions_from_mqtt(Auction* mqtt_auctions, int count);
void next_auction();
void prev_auction();
void show_custom_loading(const char* message);
void update_arrow_visibility();
const char* print_current_auction_details();
void clear_auction_data();
void hide_auction_screen();
void show_auction_screen();

#endif // AUCTIONSCREEN_H