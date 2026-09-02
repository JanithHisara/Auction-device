// #include "AuctionScreen.h"
// #include <cstring>

// #define TFT_WIDTH  240
// #define TFT_HEIGHT 320

// // ================== GLOBALS ==================
// AuctionDisplay auction_list[MAX_AUCTIONS];
// int auction_count = 0;
// int current_index = 0;

// lv_obj_t* main_card = nullptr;
// lv_obj_t* label_name = nullptr;
// lv_obj_t* label_id = nullptr;
// lv_obj_t* label_status_text = nullptr;
// lv_obj_t* label_mode = nullptr;
// lv_obj_t* label_mode_icon = nullptr;
// lv_obj_t* label_status_icon = nullptr;
// lv_obj_t* label_datetime = nullptr;
// lv_obj_t* label_items = nullptr;
// lv_obj_t* label_users = nullptr;
// lv_obj_t* arrow_up = nullptr;
// lv_obj_t* arrow_down = nullptr;
// lv_obj_t* page_indicator = nullptr;
// lv_obj_t* loading_label = nullptr;

// // Color definitions
// #define COLOR_PRIMARY     lv_color_hex(0x5C9ACF)  // Blue
// #define COLOR_SUCCESS     lv_color_hex(0x68B382)  // Green
// #define COLOR_DANGER      lv_color_hex(0xD96B6B)  // Red
// #define COLOR_WARNING     lv_color_hex(0xD19A66)  // Orange
// #define lv_color_white()        lv_color_hex(0xFFFFFF)  // Dark Gray
// #define COLOR_LIGHT       lv_color_hex(0xF5F5F5)  // Light Gray
// #define COLOR_WHITE       lv_color_hex(0x2B2B36)  // White
// #define COLOR_BLACK       lv_color_hex(0x000000)  // Black
// #define COLOR_BORDER      lv_color_hex(0x3A3A4A)  // Light Gray Border
// #define COLOR_SELECTED    lv_color_hex(0x3B3B4A)  // Light Pink

// // Unicode Icons
// #define ICON_USER         "\uF07B"  // User icon
// #define ICON_CALENDAR     "\uF00B"  // Calendar icon
// #define ICON_BOX          "\uF01C"  // Box icon
// #define ICON_SETTINGS     "\uF013"  // Lock icon
// #define BID_MODE          "\uF06E"  // OPEN icon
// #define BID_MODE_CLOSED   "\uF070"  // CLOSE icon
// #define ICON_TAG          "\uF021"  // Tag icon
// #define ICON_ARROW_UP     "\uF077"  // Arrow up
// #define ICON_ARROW_DOWN   "\uF078"  // Arrow down

// // ================== CREATE AUCTION DISPLAY ==================
// void create_auction_display(lv_obj_t* content_area) {
//     // Configure content area
//     lv_obj_set_style_pad_all(content_area, 0, 0);
//     lv_obj_set_style_bg_color(content_area, COLOR_WHITE, 0);
//     lv_obj_set_style_bg_opa(content_area, LV_OPA_COVER, 0);

//     // Create loading label
//     loading_label = lv_label_create(content_area);
//     lv_label_set_text(loading_label, "Loading...");
//     lv_obj_set_style_text_color(loading_label, lv_color_white(), 0);
//     lv_obj_set_style_text_font(loading_label, &lv_font_montserrat_14, 0);
//     lv_obj_center(loading_label);

//     // ================== TOP ARROW ==================
//     arrow_up = lv_label_create(content_area);
//     lv_label_set_text(arrow_up, LV_SYMBOL_LEFT);
//     lv_obj_set_style_text_color(arrow_up, COLOR_DANGER, 0);
//     lv_obj_set_style_text_font(arrow_up, &lv_font_montserrat_14, 0);
//     lv_obj_align(arrow_up, LV_ALIGN_LEFT_MID, 5, 0);
//     lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);

//     // ================== MAIN CARD ==================
//     int card_width = 220;
//     int card_height = 240;

//     main_card = lv_obj_create(content_area);
//     lv_obj_set_size(main_card, card_width, card_height);
//     lv_obj_center(main_card);  // Perfectly centered

//     // Card styling
//     lv_obj_set_style_radius(main_card, 12, 0);
//     lv_obj_set_style_border_width(main_card, 2, 0);
//     lv_obj_set_style_border_color(main_card, COLOR_BORDER, 0);
//     lv_obj_set_style_bg_color(main_card, COLOR_WHITE, 0);
//     lv_obj_set_style_shadow_width(main_card, 10, 0);
//     lv_obj_set_style_shadow_color(main_card, lv_color_hex(0xAAAAAA), 0);
//     lv_obj_set_style_shadow_ofs_x(main_card, 2, 0);
//     lv_obj_set_style_shadow_ofs_y(main_card, 2, 0);
//     lv_obj_set_style_pad_all(main_card, 15, 0);

//     // ===== CARD CONTENT with proper positioning =====

//     // Auction Name (top left, with scroll animation)
//     label_name = lv_label_create(main_card);
//     lv_obj_set_pos(label_name, 15, 18);
//     lv_obj_set_width(label_name, card_width - 70);
//     lv_obj_set_style_text_color(label_name, COLOR_DANGER, 0);
//     lv_obj_set_style_text_font(label_name, &lv_font_montserrat_14, 0);
//     lv_label_set_long_mode(label_name, LV_LABEL_LONG_SCROLL_CIRCULAR);
//     lv_label_set_text(label_name, "Auction Name");

//     // Separator line 1
//     lv_obj_t* line1 = lv_obj_create(main_card);
//     lv_obj_set_size(line1, card_width - 30, 1);
//     lv_obj_set_pos(line1, 0, 45);
//     lv_obj_set_style_bg_color(line1, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_border_width(line1, 0, 0);
//     lv_obj_set_style_pad_all(line1, 0, 0);

//     //  ID Icon
//     lv_obj_t* ID_icon = lv_label_create(main_card);
//     lv_obj_set_pos(ID_icon, 15, 55);
//     lv_label_set_text(ID_icon, ICON_SETTINGS);
//     lv_obj_set_style_text_color(ID_icon, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(ID_icon, &lv_font_montserrat_14, 0);

//     // ID
//     label_id = lv_label_create(main_card);
//     lv_obj_set_pos(label_id, 40, 55);
//     lv_obj_set_style_text_color(label_id, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(label_id, &lv_font_montserrat_14, 0);
//     lv_label_set_text(label_id, "ID: ABC123");

//     // Calendar Icon
//     lv_obj_t* cal_icon = lv_label_create(main_card);
//     lv_obj_set_pos(cal_icon, 15, 80);
//     lv_label_set_text(cal_icon, ICON_CALENDAR);
//     lv_obj_set_style_text_color(cal_icon, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(cal_icon, &lv_font_montserrat_14, 0);

//     // Date/Time
//     label_datetime = lv_label_create(main_card);
//     lv_obj_set_pos(label_datetime, 40, 80);
//     lv_obj_set_style_text_color(label_datetime, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(label_datetime, &lv_font_montserrat_14, 0);
//     lv_label_set_text(label_datetime, "01/01 00:00");

//     // Mode Icon
//     label_mode_icon = lv_label_create(main_card);
//     lv_obj_set_pos(label_mode_icon, 15, 105);
//     lv_obj_set_style_text_font(label_mode_icon, &lv_font_montserrat_14, 0);
//     lv_label_set_text(label_mode_icon, BID_MODE);

//     // Mode Text
//     label_mode = lv_label_create(main_card);
//     lv_obj_set_pos(label_mode, 40, 105);
//     lv_obj_set_style_text_color(label_mode, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(label_mode, &lv_font_montserrat_14, 0);
//     lv_label_set_text(label_mode, "Open");

//     // Status Icon (left side)
//     label_status_icon = lv_label_create(main_card);
//     lv_obj_set_pos(label_status_icon, 15, 130);
//     lv_obj_set_style_text_font(label_status_icon, &lv_font_montserrat_14, 0);
//     lv_label_set_text(label_status_icon, ICON_TAG);

//     // Status Text (next to icon) - FIXED: renamed to status_text_label
//     lv_obj_t* status_text_label = lv_label_create(main_card);
//     lv_obj_set_pos(status_text_label, 40, 130);
//     lv_obj_set_style_text_color(status_text_label, lv_color_hex(0x160000),
//     0); lv_obj_set_style_text_font(status_text_label, &lv_font_montserrat_14,
//     0); lv_label_set_text(status_text_label, "Live"); label_status_text =
//     status_text_label;  // Assign to global

//     // Items Icon
//     lv_obj_t* items_icon = lv_label_create(main_card);
//     lv_obj_set_pos(items_icon, 15, 155);
//     lv_label_set_text(items_icon, ICON_BOX);
//     lv_obj_set_style_text_color(items_icon, COLOR_PRIMARY, 0);
//     lv_obj_set_style_text_font(items_icon, &lv_font_montserrat_14, 0);

//     // Items count
//     label_items = lv_label_create(main_card);
//     lv_obj_set_pos(label_items, 45, 155);
//     lv_obj_set_style_text_color(label_items, COLOR_PRIMARY, 0);
//     lv_obj_set_style_text_font(label_items, &lv_font_montserrat_14, 0);
//     lv_label_set_text(label_items, "0");

//     // Items text (this is fine, it's a local variable)
//     lv_obj_t* items_text = lv_label_create(main_card);
//     lv_obj_set_pos(items_text, 70, 155);
//     lv_obj_set_style_text_color(items_text, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(items_text, &lv_font_montserrat_14, 0);
//     lv_label_set_text(items_text, "Items");

//     // Users Icon
//     lv_obj_t* users_icon = lv_label_create(main_card);
//     lv_obj_set_pos(users_icon, 15, 180);
//     lv_label_set_text(users_icon, ICON_USER);
//     lv_obj_set_style_text_color(users_icon, COLOR_SUCCESS, 0);
//     lv_obj_set_style_text_font(users_icon, &lv_font_montserrat_14, 0);

//     // Users count
//     label_users = lv_label_create(main_card);
//     lv_obj_set_pos(label_users, 45, 180);
//     lv_obj_set_style_text_color(label_users, COLOR_SUCCESS, 0);
//     lv_obj_set_style_text_font(label_users, &lv_font_montserrat_14, 0);
//     lv_label_set_text(label_users, "0");

//     // Users text
//     lv_obj_t* users_text = lv_label_create(main_card);
//     lv_obj_set_pos(users_text, 70, 180);
//     lv_obj_set_style_text_color(users_text, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(users_text, &lv_font_montserrat_14, 0);
//     lv_label_set_text(users_text, "Registered");

//     // ================== BOTTOM ARROW ==================
//     arrow_down = lv_label_create(content_area);
//     lv_label_set_text(arrow_down, LV_SYMBOL_RIGHT);
//     lv_obj_set_style_text_color(arrow_down, COLOR_DANGER, 0);
//     lv_obj_set_style_text_font(arrow_down, &lv_font_montserrat_14, 0);
//     lv_obj_align(arrow_down, LV_ALIGN_BOTTOM_MID, 0, -1);

//     // ================== PAGE INDICATOR ==================
//     page_indicator = lv_label_create(content_area);
//     lv_obj_set_style_text_color(page_indicator, lv_color_hex(0x160000), 0);
//     lv_obj_set_style_text_font(page_indicator, &lv_font_montserrat_14, 0);
//     lv_obj_align(page_indicator, LV_ALIGN_BOTTOM_MID, 85, -28);
//     lv_label_set_text(page_indicator, "1/1");

//     // Initially hide main card and show loading
//     lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
// }

// // ================== UPDATE ARROW VISIBILITY ==================
// void update_arrow_visibility() {
//     if(auction_count <= 1) {
//         lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
//     } else {
//         // Show up arrow if not at first item
//         if(current_index > 0) {
//             lv_obj_clear_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
//         } else {
//             lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
//         }

//         // Show down arrow if not at last item
//         if(current_index < auction_count - 1) {
//             lv_obj_clear_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
//         } else {
//             lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
//         }
//     }
// }

// // ================== SHOW LOADING ==================
// void show_loading() {
//     if(loading_label) {
//         lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
// }

// // ================== HIDE LOADING ==================
// void hide_loading() {
//     if(loading_label) {
//         lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     lv_obj_clear_flag(main_card, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_clear_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
//     update_arrow_visibility();
// }

// // ================== REFRESH DISPLAY ==================
// void refresh_display() {
//     if(auction_count == 0 || current_index >= auction_count) {
//         show_loading();
//         return;
//     }

//     hide_loading();

//     // Get current auction
//     AuctionDisplay& current = auction_list[current_index];

//     // Update content
//     lv_label_set_text(label_name, current.name);

//     // Format ID
//     char id_buf[32];
//     snprintf(id_buf, sizeof(id_buf), "%s", current.id);
//     lv_label_set_text(label_id, id_buf);

//     // Set date/time
//     lv_label_set_text(label_datetime, current.start_datetime);

//         // Set mode text and icon
//     const char* mode = current.mode;
//     lv_label_set_text(label_mode, mode);

//     // Set mode icon based on mode
//     if(strcmp(mode, "ENGLISH") == 0) {
//         lv_label_set_text(label_mode_icon, BID_MODE);
//         lv_obj_set_style_text_color(label_mode_icon, COLOR_SUCCESS, 0);  //
//         Green for open
//     } else {
//         lv_label_set_text(label_mode_icon, BID_MODE_CLOSED);
//         lv_obj_set_style_text_color(label_mode_icon, COLOR_WARNING, 0);  //
//         Dark for closed
//     }

//     // Set status icon and text
//     const char* status = current.status;
//     lv_label_set_text(label_status_text, status);

//     // Set status icon based on status text
//     if(strcmp(status, "Live") == 0 || strcmp(status, "Open") == 0) {
//         lv_label_set_text(label_status_icon, ICON_TAG);
//         lv_obj_set_style_text_color(label_status_icon, COLOR_DANGER, 0);  //
//         Red for live
//     } else if(strcmp(status, "Register") == 0 || strcmp(status,
//     "Registration") == 0) {
//         lv_label_set_text(label_status_icon, ICON_TAG);
//         lv_obj_set_style_text_color(label_status_icon, COLOR_SUCCESS, 0);  //
//         Green for register
//     } else if(strcmp(status, "Closed") == 0) {
//         lv_label_set_text(label_status_icon, ICON_TAG);
//         lv_obj_set_style_text_color(label_status_icon,
//         lv_color_hex(0x160000), 0);  // Dark for closed
//     } else {
//         lv_label_set_text(label_status_icon, ICON_TAG);
//         lv_obj_set_style_text_color(label_status_icon, COLOR_WARNING, 0);  //
//         Orange for other
//     }

//     // Set items count
//     char items_buf[8];
//     snprintf(items_buf, sizeof(items_buf), "%d", current.items_count);
//     lv_label_set_text(label_items, items_buf);

//     // Set users count
//     char users_buf[8];
//     snprintf(users_buf, sizeof(users_buf), "%d", current.registered_count);
//     lv_label_set_text(label_users, users_buf);

//     // Update page indicator
//     char page_buf[16];
//     snprintf(page_buf, sizeof(page_buf), "%d/%d", current_index + 1,
//     auction_count); lv_label_set_text(page_indicator, page_buf);

//     // Update arrow visibility
//     update_arrow_visibility();
// }
// // ================== UPDATE AUCTIONS FROM MQTT ==================
// void update_auctions_from_mqtt(Auction* mqtt_auctions, int count) {
//     auction_count = (count > MAX_AUCTIONS) ? MAX_AUCTIONS : count;
//     current_index = 0;

//     // Static buffers for string storage
//     static char name_buffers[MAX_AUCTIONS][32];
//     static char id_buffers[MAX_AUCTIONS][20];
//     static char mode_buffers[MAX_AUCTIONS][16];
//     static char status_buffers[MAX_AUCTIONS][16];
//     static char datetime_buffers[MAX_AUCTIONS][32];

//     for(int i = 0; i < auction_count; i++) {
//         // Copy name
//         strncpy(name_buffers[i], mqtt_auctions[i].Name.c_str(), 31);
//         name_buffers[i][31] = '\0';

//         // Copy ID (truncate if too long)
//         strncpy(id_buffers[i], mqtt_auctions[i].Auction_ID.c_str(), 19);
//         id_buffers[i][19] = '\0';

//         // Copy mode
//         strncpy(mode_buffers[i], mqtt_auctions[i].Auction_Mode.c_str(), 15);
//         mode_buffers[i][15] = '\0';

//         // Copy status
//         strncpy(status_buffers[i], mqtt_auctions[i].Auction_Status.c_str(),
//         15); status_buffers[i][15] = '\0';

//         // Format date/time nicely
//         const char* start_datetime = mqtt_auctions[i].Start_DateTime.c_str();

//         if(strlen(start_datetime) >= 16) {
//             // Format as "MM/DD HH:MM"
//             snprintf(datetime_buffers[i], 31, "%.5s %.5s",
//                      start_datetime + 5,
//                      start_datetime + 11);
//             // Replace '-' with '/'
//             for(char *p = datetime_buffers[i]; *p; p++) {
//                 if(*p == '-') *p = '/';
//             }
//         } else {
//             strncpy(datetime_buffers[i], start_datetime, 31);
//         }

//         // Assign to display struct
//         auction_list[i].name = name_buffers[i];
//         auction_list[i].id = id_buffers[i];
//         auction_list[i].status = status_buffers[i];
//         auction_list[i].mode = mode_buffers[i];
//         auction_list[i].start_datetime = datetime_buffers[i];
//         auction_list[i].items_count = mqtt_auctions[i].Items_Count;
//         auction_list[i].registered_count = mqtt_auctions[i].Registered_Count;
//     }

//     refresh_display();
// }

// // ================== NAVIGATION ==================
// void next_auction() {
//     if(current_index < auction_count - 1) {
//         current_index++;
//         refresh_display();

//         // Simple visual feedback - flash the card
//         lv_obj_set_style_bg_color(main_card, COLOR_SELECTED, 0);
//         lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
//             lv_obj_set_style_bg_color(main_card, COLOR_WHITE, 0);
//             lv_timer_del(t);
//         }, 100, nullptr);
//         lv_timer_set_repeat_count(timer, 1);
//     }
// }

// void prev_auction() {
//     if(current_index > 0) {
//         current_index--;
//         refresh_display();

//         // Simple visual feedback - flash the card
//         lv_obj_set_style_bg_color(main_card, COLOR_SELECTED, 0);
//         lv_timer_t* timer = lv_timer_create([](lv_timer_t* t) {
//             lv_obj_set_style_bg_color(main_card, COLOR_WHITE, 0);
//             lv_timer_del(t);
//         }, 100, nullptr);
//         lv_timer_set_repeat_count(timer, 1);
//     }
// }

// // ================== SHOW CUSTOM LOADING ==================
// void show_custom_loading(const char* message) {
//     if(loading_label) {
//         lv_label_set_text(loading_label, message);
//         lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
//     lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);

//     lv_timer_handler();
// }

// // ================== PRINT CURRENT AUCTION DETAILS AND RETURN ID
// ================== const char* print_current_auction_details() {
//     if(auction_count == 0) {
//         Serial.println("\n❌ No auctions available");
//         return nullptr;
//     }

//     if(current_index < 0 || current_index >= auction_count) {
//         Serial.println("\n❌ Invalid auction index");
//         return nullptr;
//     }

//     AuctionDisplay& current = auction_list[current_index];

//     Serial.println("\n╔════════════════════════════════════╗");
//     Serial.println("║     CURRENT AUCTION DETAILS       ║");
//     Serial.println("╠════════════════════════════════════╣");

//     // Auction Name
//     Serial.print("║ Name      : ");
//     if(current.name) {
//         Serial.print(current.name);
//         int len = strlen(current.name);
//         for(int i = len; i < 20; i++) Serial.print(" ");
//     } else {
//         Serial.print("Unknown               ");
//     }
//     Serial.println(" ║");

//     // Auction ID - highlight this one
//     Serial.print("║ ID        : ");
//     if(current.id) {
//         Serial.print(current.id);
//         int len = strlen(current.id);
//         for(int i = len; i < 20; i++) Serial.print(" ");
//     } else {
//         Serial.print("Unknown               ");
//     }
//     Serial.println(" ║");

//     // Date/Time
//     Serial.print("║ Date/Time : ");
//     if(current.start_datetime) {
//         Serial.print(current.start_datetime);
//         int len = strlen(current.start_datetime);
//         for(int i = len; i < 20; i++) Serial.print(" ");
//     } else {
//         Serial.print("Unknown               ");
//     }
//     Serial.println(" ║");

//     // Mode
//     Serial.print("║ Mode      : ");
//     if(current.mode) {
//         Serial.print(current.mode);
//         int len = strlen(current.mode);
//         for(int i = len; i < 20; i++) Serial.print(" ");
//     } else {
//         Serial.print("Unknown               ");
//     }
//     Serial.println(" ║");

//     // Status
//     Serial.print("║ Status    : ");
//     if(current.status) {
//         Serial.print(current.status);
//         int len = strlen(current.status);
//         for(int i = len; i < 20; i++) Serial.print(" ");
//     } else {
//         Serial.print("Unknown               ");
//     }
//     Serial.println(" ║");

//     // Items Count
//     Serial.print("║ Items     : ");
//     Serial.print(current.items_count);
//     char items_buf[10];
//     snprintf(items_buf, sizeof(items_buf), "%d", current.items_count);
//     int len = strlen(items_buf);
//     for(int i = len; i < 20; i++) Serial.print(" ");
//     Serial.println(" ║");

//     // Registered Users
//     Serial.print("║ Registered: ");
//     Serial.print(current.registered_count);
//     char reg_buf[10];
//     snprintf(reg_buf, sizeof(reg_buf), "%d", current.registered_count);
//     len = strlen(reg_buf);
//     for(int i = len; i < 20; i++) Serial.print(" ");
//     Serial.println(" ║");

//     Serial.println("╚════════════════════════════════════╝");
//     Serial.print("📊 Auction ");
//     Serial.print(current_index + 1);
//     Serial.print(" of ");
//     Serial.println(auction_count);
//     //show_custom_loading("Loading...");
//     // Return the auction ID
//     return current.id;

// }

#include "AuctionScreen.h"
#include <cstring>

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

// ================== GLOBALS ==================
AuctionDisplay auction_list[MAX_AUCTIONS];
int auction_count = 0;
int current_index = 0;

// Storage for all auction data (only active when viewing auctions)
static char name_buffers[MAX_AUCTIONS][36];
static char id_buffers[MAX_AUCTIONS][24];
static char mode_buffers[MAX_AUCTIONS][36];
static char status_buffers[MAX_AUCTIONS][24];
static char start_datetime_buffers[MAX_AUCTIONS][24];
static char end_datetime_buffers[MAX_AUCTIONS][24];

// LVGL Objects
lv_obj_t *main_card = nullptr;
lv_obj_t *label_name = nullptr;
lv_obj_t *label_id = nullptr;
lv_obj_t *label_status_text = nullptr;
lv_obj_t *label_mode = nullptr;
lv_obj_t *label_mode_icon = nullptr;
lv_obj_t *label_status_icon = nullptr;
lv_obj_t *label_datetime = nullptr;
lv_obj_t *label_end_datetime = nullptr;
lv_obj_t *label_items = nullptr;
lv_obj_t *label_users = nullptr;
lv_obj_t *arrow_up = nullptr;
lv_obj_t *arrow_down = nullptr;
lv_obj_t *page_indicator = nullptr;
lv_obj_t *loading_label = nullptr;

// Color definitions
#define COLOR_PRIMARY lv_color_hex(0x5C9ACF)
#define COLOR_SUCCESS lv_color_hex(0x68B382)
#define COLOR_DANGER lv_color_hex(0xD96B6B)
#define COLOR_WARNING lv_color_hex(0xD19A66)
#define lv_color_white() lv_color_hex(0xFFFFFF)
#define COLOR_WHITE lv_color_hex(0x2B2B36)
#define COLOR_BORDER lv_color_hex(0x3A3A4A)
#define COLOR_SELECTED lv_color_hex(0x3B3B4A)

// // Unicode Icons
#define ICON_USER "\uF015"   // User icon (MDI Account icon U000F0004)
#define ICON_CALENDAR "\uF00B"   // Calendar icon
#define ICON_BOX "\uF01C"        // Box icon
#define ICON_SETTINGS "\uF013"   // Lock icon
#define ICON_ID "\uF052"         // Auction ID icon
#define ICON_LIVE "\uF1EB"       // Live status icon
#define BID_MODE "\uF06E"        // OPEN icon
#define BID_MODE_CLOSED "\uF070" // CLOSE icon
#define ICON_TAG "\uF021"        // Tag icon
#define ICON_WAITING "\uF021"    // Waiting icon
#define ICON_ARROW_UP "\uF077"   // Arrow up
#define ICON_ARROW_DOWN "\uF078" // Arrow down

// Built-in LVGL Symbols for Icons (Overrides except ICON_USER)
// #define ICON_CALENDAR LV_SYMBOL_BELL
// #define ICON_BOX LV_SYMBOL_DIRECTORY
// #define ICON_SETTINGS LV_SYMBOL_SETTINGS
// #define BID_MODE LV_SYMBOL_EYE_OPEN
// #define BID_MODE_CLOSED LV_SYMBOL_EYE_CLOSE
// #define ICON_TAG LV_SYMBOL_BULLET
// #define ICON_ARROW_UP LV_SYMBOL_UP
// #define ICON_ARROW_DOWN LV_SYMBOL_DOWN

// ================== CREATE AUCTION DISPLAY ==================
void create_auction_display(lv_obj_t *content_area) {
  // Configure content area
  lv_obj_set_style_pad_all(content_area, 0, 0);
  lv_obj_set_style_bg_color(content_area, COLOR_WHITE, 0);
  lv_obj_set_style_bg_opa(content_area, LV_OPA_COVER, 0);

  // Create loading label
  loading_label = lv_label_create(content_area);
  lv_label_set_text(loading_label, "Loading Auctions...");
  lv_obj_set_style_text_color(loading_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(loading_label, &lv_font_montserrat_14, 0);
  lv_obj_center(loading_label);

  // ================== TOP ARROW ==================
  arrow_up = lv_label_create(content_area);
  lv_label_set_text(arrow_up, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_color(arrow_up, COLOR_DANGER, 0);
  lv_obj_set_style_text_font(arrow_up, &lv_font_montserrat_14, 0);
  lv_obj_align(arrow_up, LV_ALIGN_LEFT_MID, 5, 0);
  lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);

  // ================== MAIN CARD ==================
  int card_width = 220;
  int card_height = 260; // Slightly taller to accommodate end date

  main_card = lv_obj_create(content_area);
  lv_obj_set_size(main_card, card_width, card_height);
  lv_obj_center(main_card);
  lv_obj_set_scrollbar_mode(main_card, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_OFF);
  lv_obj_move_foreground(arrow_up);

  // Card styling
  lv_obj_set_style_radius(main_card, 12, 0);
  lv_obj_set_style_border_width(main_card, 2, 0);
  lv_obj_set_style_border_color(main_card, COLOR_BORDER, 0);
  lv_obj_set_style_bg_color(main_card, COLOR_WHITE, 0);
  lv_obj_set_style_shadow_width(main_card, 10, 0);
  lv_obj_set_style_shadow_color(main_card, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_shadow_ofs_x(main_card, 2, 0);
  lv_obj_set_style_shadow_ofs_y(main_card, 2, 0);
  lv_obj_set_style_pad_all(main_card, 15, 0);

  // ===== CARD CONTENT =====

  // Auction Name (top)
  label_name = lv_label_create(main_card);
    lv_obj_set_pos(label_name, 15, 10);
    lv_obj_set_width(label_name, card_width - 70);
    lv_obj_set_style_text_color(label_name, lv_color_white(), 0);
  lv_obj_set_style_text_color(label_name, COLOR_DANGER, 0);
  lv_obj_set_style_text_font(label_name, &lv_font_montserrat_14, 0);
  lv_label_set_long_mode(label_name, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(label_name, "Auction Name");

  // Separator line 1
  lv_obj_t *line1 = lv_obj_create(main_card);
  lv_obj_set_size(line1, card_width - 30, 1);
  lv_obj_set_pos(line1, 0, 35);
  lv_obj_set_style_bg_color(line1, lv_color_hex(0xCCCCCC), 0);
  lv_obj_set_style_border_width(line1, 0, 0);
  lv_obj_set_style_pad_all(line1, 0, 0);

  // ID Icon
  lv_obj_t *ID_icon = lv_label_create(main_card);
  lv_obj_set_pos(ID_icon, 15, 45);
  lv_label_set_text(ID_icon, ICON_ID);
  lv_obj_set_style_text_color(ID_icon, lv_color_white(), 0);
  lv_obj_set_style_text_font(ID_icon, &lv_font_montserrat_14, 0);

  // ID Label
  label_id = lv_label_create(main_card);
  lv_obj_set_pos(label_id, 40, 45);
  lv_obj_set_width(label_id, card_width - 70);
  lv_label_set_long_mode(label_id, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_text_color(label_id, lv_color_white(), 0);
  lv_obj_set_style_text_font(label_id, &lv_font_montserrat_14, 0);
  lv_label_set_text(label_id, "ID: ABC123");

  // Calendar Icon for Start Date
  lv_obj_t *cal_icon = lv_label_create(main_card);
  lv_obj_set_pos(cal_icon, 15, 70);
  lv_label_set_text(cal_icon, ICON_CALENDAR);
  lv_obj_set_style_text_color(cal_icon, lv_color_white(), 0);
  lv_obj_set_style_text_font(cal_icon, &lv_font_montserrat_14, 0);

  // Start Date/Time
  label_datetime = lv_label_create(main_card);
  lv_obj_set_pos(label_datetime, 40, 70);
  lv_obj_set_width(label_datetime, card_width - 70);
  lv_label_set_long_mode(label_datetime, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_text_color(label_datetime, lv_color_white(), 0);
  lv_obj_set_style_text_font(label_datetime, &lv_font_montserrat_14, 0);
  lv_label_set_text(label_datetime, "Start: --/-- --:--");

  lv_obj_t *cal_icon1 = lv_label_create(main_card);
  lv_obj_set_pos(cal_icon1, 15, 95);
  lv_label_set_text(cal_icon1, ICON_CALENDAR);
  lv_obj_set_style_text_color(cal_icon1, COLOR_DANGER, 0);
  lv_obj_set_style_text_font(cal_icon1, &lv_font_montserrat_14, 0);

  // End Date/Time (with danger color)
  label_end_datetime = lv_label_create(main_card);
  lv_obj_set_pos(label_end_datetime, 40, 95);
  lv_obj_set_width(label_end_datetime, card_width - 70);
  lv_label_set_long_mode(label_end_datetime, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_text_color(label_end_datetime, COLOR_DANGER, 0);
  lv_obj_set_style_text_font(label_end_datetime, &lv_font_montserrat_14, 0);
  lv_label_set_text(label_end_datetime, "End: --/-- --:--");

  // Mode Icon
  label_mode_icon = lv_label_create(main_card);
    lv_obj_set_pos(label_mode_icon, 15, 125);
    lv_obj_set_style_text_font(label_mode_icon, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_mode_icon, lv_color_white(), 0);
  lv_label_set_text(label_mode_icon, BID_MODE);

  // Mode Text
  label_mode = lv_label_create(main_card);
  lv_obj_set_pos(label_mode, 40, 125);
  lv_obj_set_width(label_mode, card_width - 70);
  lv_label_set_long_mode(label_mode, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_text_color(label_mode, lv_color_white(), 0);
  lv_obj_set_style_text_font(label_mode, &lv_font_montserrat_14, 0);
  lv_label_set_text(label_mode, "---");

  // Status Icon
  label_status_icon = lv_label_create(main_card);
    lv_obj_set_pos(label_status_icon, 15, 155);
    lv_obj_set_style_text_font(label_status_icon, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_status_icon, lv_color_white(), 0);
  lv_label_set_text(label_status_icon, ICON_LIVE);

  // Status Text
  label_status_text = lv_label_create(main_card);
  lv_obj_set_pos(label_status_text, 40, 155);
  lv_obj_set_width(label_status_text, card_width - 70);
  lv_label_set_long_mode(label_status_text, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_text_color(label_status_text, lv_color_white(), 0);
  lv_obj_set_style_text_font(label_status_text, &lv_font_montserrat_14, 0);
  lv_label_set_text(label_status_text, "LIVE");

  // Items Icon
  lv_obj_t *items_icon = lv_label_create(main_card);
    lv_obj_set_pos(items_icon, 15, 185);
    lv_label_set_text(items_icon, ICON_BOX);
    lv_obj_set_style_text_color(items_icon, lv_color_white(), 0);
  lv_obj_set_style_text_color(items_icon, COLOR_PRIMARY, 0);
  lv_obj_set_style_text_font(items_icon, &lv_font_montserrat_14, 0);

  // Items count
  label_items = lv_label_create(main_card);
  lv_obj_set_pos(label_items, 45, 185);
  lv_obj_set_style_text_color(label_items, COLOR_PRIMARY, 0);
  lv_obj_set_style_text_font(label_items, &lv_font_montserrat_14, 0);
  lv_label_set_text(label_items, "0");

  // Items text
  lv_obj_t *items_text = lv_label_create(main_card);
  lv_obj_set_pos(items_text, 70, 185);
  lv_obj_set_style_text_color(items_text, lv_color_white(), 0);
  lv_obj_set_style_text_font(items_text, &lv_font_montserrat_14, 0);
  lv_label_set_text(items_text, "Items");

  // Users Icon
  lv_obj_t *users_icon = lv_label_create(main_card);
    lv_obj_set_pos(users_icon, 15, 215);
    lv_label_set_text(users_icon, ICON_USER);
    lv_obj_set_style_text_color(users_icon, lv_color_white(), 0);
  lv_obj_set_style_text_color(users_icon, COLOR_SUCCESS, 0);
  lv_obj_set_style_text_font(users_icon, &lv_font_montserrat_14, 0);

  // Users count
  label_users = lv_label_create(main_card);
  lv_obj_set_pos(label_users, 45, 215);
  lv_obj_set_style_text_color(label_users, COLOR_SUCCESS, 0);
  lv_obj_set_style_text_font(label_users, &lv_font_montserrat_14, 0);
  lv_label_set_text(label_users, "0");

  // Users text
  lv_obj_t *users_text = lv_label_create(main_card);
  lv_obj_set_pos(users_text, 70, 215);
  lv_obj_set_style_text_color(users_text, lv_color_white(), 0);
  lv_obj_set_style_text_font(users_text, &lv_font_montserrat_14, 0);
  lv_label_set_text(users_text, "Users");

  // ================== BOTTOM ARROW ==================
  arrow_down = lv_label_create(content_area);
  lv_label_set_text(arrow_down, LV_SYMBOL_RIGHT);
  lv_obj_set_style_text_color(arrow_down, COLOR_DANGER, 0);
  lv_obj_set_style_text_font(arrow_down, &lv_font_montserrat_14, 0);
  lv_obj_align(arrow_down, LV_ALIGN_RIGHT_MID, -5, 0);

  // ================== PAGE INDICATOR ==================
  page_indicator = lv_label_create(content_area);
  lv_obj_set_style_text_color(page_indicator, lv_color_white(), 0);
  lv_obj_set_style_text_font(page_indicator, &lv_font_montserrat_14, 0);
  lv_obj_align(page_indicator, LV_ALIGN_BOTTOM_MID, 85, -24);
  lv_label_set_text(page_indicator, "1/1");

  // Initially hide main card and show loading
  lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
}

// ================== CLEAR AUCTION DATA ==================
void clear_auction_data() {
  // Clear all buffers to free memory
  for (int i = 0; i < MAX_AUCTIONS; i++) {
    name_buffers[i][0] = '\0';
    id_buffers[i][0] = '\0';
    mode_buffers[i][0] = '\0';
    status_buffers[i][0] = '\0';
    start_datetime_buffers[i][0] = '\0';
    end_datetime_buffers[i][0] = '\0';
  }

  auction_count = 0;
  current_index = 0;

  // Update display to show empty state
  lv_label_set_text(label_name, "No Live Auctions Available");
  lv_label_set_text(label_id, "ID: ---");
  lv_label_set_text(label_datetime, "Start: --/-- --:--");
  lv_label_set_text(label_end_datetime, "End: --/-- --:--");
  lv_label_set_text(label_mode, "---");
  lv_label_set_text(label_status_text, "---");
  lv_label_set_text(label_items, "-");
  lv_label_set_text(label_users, "-");
  lv_label_set_text(page_indicator, "0/0");

  lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);

  Serial.println("🧹 Auction data cleared from RAM");
}

// ================== UPDATE ARROW VISIBILITY ==================
void update_arrow_visibility() {
  if (auction_count <= 1) {
    lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
  } else {
    if (current_index > 0) {
      lv_obj_clear_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
    }

    if (current_index < auction_count - 1) {
      lv_obj_clear_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

// ================== SHOW LOADING ==================
void show_loading() {
  // if(loading_label) {
  //     lv_label_set_text(loading_label, "Loading Auctions...");
  //     lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
  // }
  lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
}

// ================== HIDE LOADING ==================
void hide_loading() {
  if (loading_label) {
    lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
  }

  if (auction_count > 0) {
    lv_obj_clear_flag(main_card, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
    update_arrow_visibility();
  } else {
    // If no auctions, show empty state but keep card visible
    lv_obj_clear_flag(main_card, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(label_name, "No Auctions Available");
  }
}

// ================== REFRESH DISPLAY ==================
void refresh_display() {
  if (auction_count == 0 || current_index >= auction_count) {
    show_loading();
    return;
  }

  hide_loading();

  // Get current auction data from buffers
  const char *name = name_buffers[current_index];
  const char *id = id_buffers[current_index];
  const char *mode = mode_buffers[current_index];
  const char *status = status_buffers[current_index];
  const char *start = start_datetime_buffers[current_index];
  const char *end = end_datetime_buffers[current_index];
  int items = auction_list[current_index].items_count;
  int registered = auction_list[current_index].registered_count;

  // Update display
  lv_label_set_text(label_name, name);

  char id_buf[32];
  snprintf(id_buf, sizeof(id_buf), "ID: %s", id);
  lv_label_set_text(label_id, id_buf);

  char start_buf[32];
  snprintf(start_buf, sizeof(start_buf), "Start: %s", start);
  lv_label_set_text(label_datetime, start_buf);

  char end_buf[32];
  snprintf(end_buf, sizeof(end_buf), "End: %s", end);
  lv_label_set_text(label_end_datetime, end_buf);

  lv_label_set_text(label_mode, mode);

  // Set mode icon
  String modeStr = String(mode);
  modeStr.toUpperCase();
  if (modeStr.indexOf("CLOSED") >= 0 || modeStr.indexOf("TENDER") >= 0 ||
      modeStr.indexOf("FIXED") >= 0 || modeStr.indexOf("SEALED") >= 0) {
    lv_label_set_text(label_mode_icon, BID_MODE_CLOSED);
    lv_obj_set_style_text_color(label_mode_icon, COLOR_WARNING, 0);
  } else {
    lv_label_set_text(label_mode_icon, BID_MODE);
    lv_obj_set_style_text_color(label_mode_icon, COLOR_SUCCESS, 0);
  }

  // Set status
  lv_label_set_text(label_status_text, status);

  // Set status icon and color based on status
  if (strcmp(status, "LIVE") == 0) {
    lv_label_set_text(label_status_icon, ICON_LIVE);
    lv_obj_set_style_text_color(label_status_icon, COLOR_DANGER, 0);
  } else if (strcmp(status, "CLOSED") == 0) {
    lv_label_set_text(label_status_icon, ICON_TAG);
    lv_obj_set_style_text_color(label_status_icon, lv_color_white(), 0);
  } else {
    lv_label_set_text(label_status_icon, ICON_TAG);
    lv_obj_set_style_text_color(label_status_icon, COLOR_WARNING, 0);
  }

  // Items count
  char items_buf[8];
  snprintf(items_buf, sizeof(items_buf), "%d", items);
  lv_label_set_text(label_items, items_buf);

  // Registered users
  char users_buf[8];
  snprintf(users_buf, sizeof(users_buf), "%d", registered);
  lv_label_set_text(label_users, users_buf);

  // Page indicator
  char page_buf[16];
  snprintf(page_buf, sizeof(page_buf), "%d/%d", current_index + 1,
           auction_count);
  lv_label_set_text(page_indicator, page_buf);

  update_arrow_visibility();
}

// ================== UPDATE AUCTIONS FROM MQTT ==================
void update_auctions_from_mqtt(Auction *mqtt_auctions, int count) {
  int old_index = current_index;
  // Clear existing data first
  clear_auction_data();

  auction_count = (count > MAX_AUCTIONS) ? MAX_AUCTIONS : count;
  current_index = (old_index < auction_count)
                      ? old_index
                      : ((auction_count > 0) ? (auction_count - 1) : 0);

  for (int i = 0; i < auction_count; i++) {
    // Copy name
    strncpy(name_buffers[i], mqtt_auctions[i].Name.c_str(), 35);
    name_buffers[i][35] = '\0';

    // Copy ID
    strncpy(id_buffers[i], mqtt_auctions[i].Auction_ID.c_str(), 23);
    id_buffers[i][23] = '\0';

    // Copy mode
    strncpy(mode_buffers[i], mqtt_auctions[i].Auction_Mode.c_str(), 35);
    mode_buffers[i][35] = '\0';

    // Copy status
    strncpy(status_buffers[i], mqtt_auctions[i].Auction_Status.c_str(), 23);
    status_buffers[i][23] = '\0';

    // Format start date/time (MM/DD HH:MM)
    const char *start_datetime = mqtt_auctions[i].Start_DateTime.c_str();
    if (strlen(start_datetime) >= 16) {
      snprintf(start_datetime_buffers[i], 19, "%.5s %.5s", start_datetime + 5,
               start_datetime + 11);
      // Replace '-' with '/'
      for (char *p = start_datetime_buffers[i]; *p; p++) {
        if (*p == '-')
          *p = '/';
      }
    } else {
      strncpy(start_datetime_buffers[i], start_datetime, 19);
    }

    // Format end date/time (MM/DD HH:MM)
    const char *end_datetime = mqtt_auctions[i].End_DateTime.c_str();
    if (strlen(end_datetime) >= 16) {
      snprintf(end_datetime_buffers[i], 19, "%.5s %.5s", end_datetime + 5,
               end_datetime + 11);
      for (char *p = end_datetime_buffers[i]; *p; p++) {
        if (*p == '-')
          *p = '/';
      }
    } else {
      strncpy(end_datetime_buffers[i], end_datetime, 19);
    }

    // Store items and registered counts
    auction_list[i].items_count = mqtt_auctions[i].Items_Count;
    auction_list[i].registered_count = mqtt_auctions[i].Registered_Count;

    // Assign pointers to buffers
    auction_list[i].name = name_buffers[i];
    auction_list[i].id = id_buffers[i];
    auction_list[i].mode = mode_buffers[i];
    auction_list[i].status = status_buffers[i];
    auction_list[i].start_datetime = start_datetime_buffers[i];
    auction_list[i].end_datetime = end_datetime_buffers[i];
  }

  refresh_display();
  Serial.printf("✅ Loaded %d auctions into RAM\n", auction_count);
}

// ================== NAVIGATION ==================
void next_auction() {
  if (current_index < auction_count - 1) {
    current_index++;
    refresh_display();

    // Visual feedback
    lv_obj_set_style_bg_color(main_card, COLOR_SELECTED, 0);
    lv_timer_t *timer = lv_timer_create(
        [](lv_timer_t *t) {
          lv_obj_set_style_bg_color(main_card, COLOR_WHITE, 0);
          lv_timer_del(t);
        },
        100, nullptr);
    lv_timer_set_repeat_count(timer, 1);
  }
}

void prev_auction() {
  if (current_index > 0) {
    current_index--;
    refresh_display();

    // Visual feedback
    lv_obj_set_style_bg_color(main_card, COLOR_SELECTED, 0);
    lv_timer_t *timer = lv_timer_create(
        [](lv_timer_t *t) {
          lv_obj_set_style_bg_color(main_card, COLOR_WHITE, 0);
          lv_timer_del(t);
        },
        100, nullptr);
    lv_timer_set_repeat_count(timer, 1);
  }
}

// ================== SHOW CUSTOM LOADING ==================
void show_custom_loading(const char *message) {
  if (loading_label) {
    lv_label_set_text(loading_label, message);
    lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
  }
  lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);

  lv_timer_handler();
}

// ================== PRINT CURRENT AUCTION DETAILS AND RETURN ID
// ==================
const char *print_current_auction_details() {
  if (auction_count == 0) {
    Serial.println("\n❌ No auctions available");
    return nullptr;
  }

  if (current_index < 0 || current_index >= auction_count) {
    Serial.println("\n❌ Invalid auction index");
    return nullptr;
  }

  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║     CURRENT AUCTION DETAILS       ║");
  Serial.println("╠════════════════════════════════════╣");

  Serial.print("║ Name      : ");
  Serial.print(name_buffers[current_index]);
  int len = strlen(name_buffers[current_index]);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.print("║ ID        : ");
  Serial.print(id_buffers[current_index]);
  len = strlen(id_buffers[current_index]);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.print("║ Start     : ");
  Serial.print(start_datetime_buffers[current_index]);
  len = strlen(start_datetime_buffers[current_index]);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.print("║ End       : ");
  Serial.print(end_datetime_buffers[current_index]);
  len = strlen(end_datetime_buffers[current_index]);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.print("║ Mode      : ");
  Serial.print(mode_buffers[current_index]);
  len = strlen(mode_buffers[current_index]);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.print("║ Status    : ");
  Serial.print(status_buffers[current_index]);
  len = strlen(status_buffers[current_index]);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.print("║ Items     : ");
  Serial.print(auction_list[current_index].items_count);
  char items_buf[10];
  snprintf(items_buf, sizeof(items_buf), "%d",
           auction_list[current_index].items_count);
  len = strlen(items_buf);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.print("║ Registered: ");
  Serial.print(auction_list[current_index].registered_count);
  char reg_buf[10];
  snprintf(reg_buf, sizeof(reg_buf), "%d",
           auction_list[current_index].registered_count);
  len = strlen(reg_buf);
  for (int i = len; i < 20; i++)
    Serial.print(" ");
  Serial.println(" ║");

  Serial.println("╚════════════════════════════════════╝");
  Serial.print("📊 Auction ");
  Serial.print(current_index + 1);
  Serial.print(" of ");
  Serial.println(auction_count);

  return id_buffers[current_index];
}

static void show_temp_message(const char *title, const char *message,
                              uint32_t duration_ms) {
  lv_obj_t *mbox = lv_msgbox_create(NULL, title, message, NULL, true);
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
// ================== HIDE AUCTION SCREEN ==================
void hide_auction_screen() {
  if (main_card)
    lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
  if (arrow_up)
    lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
  if (arrow_down)
    lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
  if (page_indicator)
    lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
  if (loading_label)
    lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
}

// ================== SHOW AUCTION SCREEN ==================
void show_auction_screen() {
  if (auction_count > 0) {
    if (main_card)
      lv_obj_clear_flag(main_card, LV_OBJ_FLAG_HIDDEN);
    if (page_indicator)
      lv_obj_clear_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
    update_arrow_visibility();
  } else {
    if (main_card)
      lv_obj_clear_flag(main_card, LV_OBJ_FLAG_HIDDEN);
  }
}
