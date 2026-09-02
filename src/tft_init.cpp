#include "tft_init.h"

TFT_init::TFT_init(uint8_t mosi, uint8_t sclk, uint8_t cs, uint8_t dc, uint8_t rst,
                   uint16_t width, uint16_t height) {
    _mosi = mosi;
    _sclk = sclk;
    _cs   = cs;
    _dc   = dc;
    _rst  = rst;
    _width  = width;
    _height = height;
}

void TFT_init::begin() {
    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_rst, OUTPUT);
    digitalWrite(_cs, HIGH);

    SPI.begin(_sclk, -1, _mosi, _cs);
    SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));

    digitalWrite(_rst, LOW);  delay(50);
    digitalWrite(_rst, HIGH); delay(120);

    cmd(0x11); delay(120);   // Sleep out
    cmd(0x3A); data(0x55);   // Set color mode to 16-bit
    cmd(0x36); data(0x00);   // Memory access
    cmd(0x29);               // Display ON
}

void TFT_init::cmd(uint8_t c) {
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    SPI.transfer(c);
    digitalWrite(_cs, HIGH);
}

void TFT_init::data(uint8_t d) {
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    SPI.transfer(d);
    digitalWrite(_cs, HIGH);
}

void TFT_init::setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    cmd(0x2A); data(x0 >> 8); data(x0); data(x1 >> 8); data(x1);
    cmd(0x2B); data(y0 >> 8); data(y0); data(y1 >> 8); data(y1);
    cmd(0x2C);
}

void TFT_init::fillScreen(uint16_t color) {
    setAddr(0, 0, _width - 1, _height - 1);
    for (uint32_t i = 0; i < (uint32_t)_width * _height; i++) {
        data(color >> 8);
        data(color & 0xFF);
    }
}

void TFT_init::pushColors(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, lv_color_t* color_p) {
    setAddr(x0, y0, x1, y1);  // private is fine because called from inside the class
    for (int i = 0; i < (x1 - x0 + 1) * (y1 - y0 + 1); i++) {
        uint16_t c = color_p->full;
        data(c >> 8);
        data(c & 0xFF);
        color_p++;
    }
}

void TFT_init::drawImage(uint16_t x, uint16_t y,
                         uint16_t w, uint16_t h,
                         const uint16_t *image)
{
    setAddr(x, y, x + w - 1, y + h - 1);

    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);

    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        uint16_t color = pgm_read_word(&image[i]);  // read from PROGMEM
        SPI.transfer(color >> 8);
        SPI.transfer(color & 0xFF);
    }

    digitalWrite(_cs, HIGH);
}

// #include <Arduino.h>
// #include <SPI.h>
// #include <lvgl.h>
// #include <Ticker.h>

// // ================== CONFIG ==================
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS   5
// #define TFT_DC   17
// #define TFT_RST  16

// #define TFT_WIDTH  240
// #define TFT_HEIGHT 320
// #define VISIBLE_CARDS 2

// bool dark_mode = false;
// int wifi_connected = 1;

// Ticker lvgl_tick;
// static lv_disp_draw_buf_t draw_buf;
// static lv_color_t buf[TFT_WIDTH * 40];

// // ================== TFT LOW-LEVEL ==================
// void cmd(uint8_t c){
//     digitalWrite(TFT_DC, LOW);
//     digitalWrite(TFT_CS, LOW);
//     SPI.transfer(c);
//     digitalWrite(TFT_CS, HIGH);
// }
// void data(uint8_t d){
//     digitalWrite(TFT_DC, HIGH);
//     digitalWrite(TFT_CS, LOW);
//     SPI.transfer(d);
//     digitalWrite(TFT_CS, HIGH);
// }
// void setAddr(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1){
//     cmd(0x2A); data(x0>>8); data(x0); data(x1>>8); data(x1);
//     cmd(0x2B); data(y0>>8); data(y0); data(y1>>8); data(y1);
//     cmd(0x2C);
// }
// void fillScreen(uint16_t color){
//     setAddr(0,0,TFT_WIDTH-1,TFT_HEIGHT-1);
//     for(uint32_t i=0;i<TFT_WIDTH*TFT_HEIGHT;i++){
//         data(color>>8); data(color);
//     }
// }

// // ================== TFT INIT ==================
// void tft_init(){
//     pinMode(TFT_CS,OUTPUT); 
//     pinMode(TFT_DC,OUTPUT);
//     pinMode(TFT_RST,OUTPUT);
//     digitalWrite(TFT_CS,HIGH);

//     SPI.begin(TFT_SCLK,-1,TFT_MOSI,TFT_CS);
//     SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));

//     digitalWrite(TFT_RST,LOW); delay(50);
//     digitalWrite(TFT_RST,HIGH); delay(120);

//     cmd(0x11); delay(120);
//     cmd(0x3A); data(0x55);
//     cmd(0x36); data(0x00);
//     cmd(0x29);
//     //fillScreen(0xFFFF);
// }

// // ================== LVGL ==================
// void lv_tick_cb(){ lv_tick_inc(1); }

// void my_disp_flush(lv_disp_drv_t *disp,const lv_area_t *area,lv_color_t *color_p){
//     setAddr(area->x1,area->y1,area->x2,area->y2);
//     for(int y=area->y1;y<=area->y2;y++){
//         for(int x=area->x1;x<=area->x2;x++){
//             uint16_t c=color_p->full;
//             data(c>>8); data(c);
//             color_p++;
//         }
//     }
//     lv_disp_flush_ready(disp);
// }

// // ================== GLOBAL UI ==================
// lv_obj_t *top_bar;
// lv_obj_t *label_battery;
// lv_obj_t *label_battery_percent;
// lv_obj_t *label_wifi;
// lv_obj_t *label_date;
// lv_obj_t *content_area;


// // ================== TOP BAR (UNCHANGED) ==================
// void create_topbar() {
//     lv_obj_t *scr = lv_scr_act();
//     lv_obj_clean(scr);

//     top_bar = lv_obj_create(scr);
//     lv_obj_set_size(top_bar, TFT_WIDTH, 40);
//     lv_obj_set_pos(top_bar, 0, 0);

//     label_battery = lv_label_create(top_bar);
//     lv_label_set_text(label_battery, "\uF241");
//     lv_obj_align(label_battery, LV_ALIGN_TOP_LEFT, 5, 5);

//     label_battery_percent = lv_label_create(top_bar);
//     lv_label_set_text(label_battery_percent, "70%");
//     lv_obj_align(label_battery_percent, LV_ALIGN_TOP_LEFT, 30, 5);

//     label_date = lv_label_create(top_bar);
//     lv_label_set_text(label_date, "00:00 01/01");
//     lv_obj_align(label_date, LV_ALIGN_TOP_RIGHT, -5, 5);

//     label_wifi = lv_label_create(top_bar);
//     lv_label_set_text(label_wifi, "\uF1EB");
//     lv_obj_align(label_wifi, LV_ALIGN_TOP_RIGHT, -95, 5);

//     content_area = lv_obj_create(scr);
//     lv_obj_set_size(content_area, TFT_WIDTH, TFT_HEIGHT - 40);
//     lv_obj_set_pos(content_area, 0, 40);
//     lv_obj_clear_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);
// }
// // ================== AUCTION DATA ==================
// struct Auction {
//     const char* name;
//     const char* description;
//     const char* status;
// };

// #define MAX_AUCTIONS 20
// Auction auction_list[MAX_AUCTIONS];
// int auction_count = 0;
// int selected_index = 0;

// // ================== CARD OBJECTS ==================
// lv_obj_t* card[VISIBLE_CARDS];
// lv_obj_t* label_name[VISIBLE_CARDS];
// lv_obj_t* label_desc[VISIBLE_CARDS];
// lv_obj_t* label_status[VISIBLE_CARDS];

// // ================== CREATE CARDS (ORIGINAL ALIGNMENT) ==================


// void create_cards() {

//     int card_width  = 230;
//     int card_height = 100;
//     int spacing     = 10;
//     int top_margin  = 30; // space below top bar

//     // 🔴 CRITICAL: match original centering behavior
//     lv_obj_set_style_pad_all(content_area, 0, 0);
//     lv_obj_clear_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_OFF);

//     int margin_x = (TFT_WIDTH - card_width) / 2;

//     for(int i = 0; i < VISIBLE_CARDS; i++){
//         card[i] = lv_obj_create(content_area);
//         lv_obj_set_size(card[i], card_width, card_height);

//         // Add top margin only to the first card
//         int y_pos = top_margin + i * (card_height + spacing);
//         lv_obj_set_pos(card[i], margin_x, y_pos);

//         lv_obj_set_style_radius(card[i], 8, 0);
//         lv_obj_set_style_border_width(card[i], 2, 0);
//         lv_obj_set_style_bg_color(card[i], lv_color_hex(0x1E1E1E), 0);

//         // Labels
//         label_name[i] = lv_label_create(card[i]);
//         lv_obj_set_pos(label_name[i], 30, 10);

//         label_desc[i] = lv_label_create(card[i]);
//         lv_obj_set_pos(label_desc[i], 38, 30);

//         label_status[i] = lv_label_create(card[i]);
//         lv_obj_set_pos(label_status[i], 150, 20);
//         lv_obj_set_style_text_color(label_status[i], lv_color_hex(0xFFFFFF), 0);
//     }
// }


// // ================== REFRESH 2 VISIBLE CARDS ==================
// void refresh_cards() {

//     for(int i=0;i<VISIBLE_CARDS;i++){
//         int idx = selected_index + i;

//         if(idx < auction_count){

//             lv_label_set_text(label_name[i], auction_list[idx].name);
//             lv_label_set_text(label_desc[i], auction_list[idx].description);
//             lv_label_set_text(label_status[i], auction_list[idx].status);

//             // 🔴 Border only for Live Auction
//             if(strcmp(auction_list[idx].name, "Live Auction") == 0){
//                 lv_obj_set_style_border_color(card[i], lv_color_hex(0xFF0000), 0);
//             } else {
//                 lv_obj_set_style_border_color(card[i], lv_color_hex(0x666666), 0);
//             }

//             // Highlight top card only
//             lv_obj_set_style_bg_color(
//                 card[i],
//                 (i == 0) ? lv_color_hex(0xFF6B6B) : lv_color_hex(0x1E1E1E),
//                 0
//             );

//             lv_obj_clear_flag(card[i], LV_OBJ_FLAG_HIDDEN);
//         } else {
//             lv_obj_add_flag(card[i], LV_OBJ_FLAG_HIDDEN);
//         }
//     }
// }

// // ================== UPDATE AUCTIONS ==================
// void update_auctions_fixed(Auction* auctions, int count) {
//     auction_count = count;
//     selected_index = 0;

//     for(int i=0;i<count;i++)
//         auction_list[i] = auctions[i];

//     refresh_cards();
// }

// // ================== NAVIGATION ==================
// void next_auction(){
//     if(selected_index < auction_count - 1){
//         selected_index++;
//         refresh_cards();
//     }
// }
// void prev_auction(){
//     if(selected_index > 0){
//         selected_index--;
//         refresh_cards();
//     }
// }

// // ================== SETUP ==================
// void setup(){
//     Serial.begin(115200);
//     tft_init();

//     lv_init();
//     lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH*40);

//     static lv_disp_drv_t drv;
//     lv_disp_drv_init(&drv);
//     drv.hor_res = TFT_WIDTH;
//     drv.ver_res = TFT_HEIGHT;
//     drv.flush_cb = my_disp_flush;
//     drv.draw_buf = &draw_buf;
//     lv_disp_drv_register(&drv);

//     lvgl_tick.attach_ms(1, lv_tick_cb);

//     create_topbar();
//     create_cards();

//     Auction demo[] = {
//         {"Live Auction","Description 1","Open"},
//         {"Auction 2","Description 2","Closed"},
//         {"Auction 3","Description 3","Open"},
//         {"Auction 4","Description 4","closed"},
//     };
//     update_auctions_fixed(demo, 4);
// }

// // ================== LOOP ==================
// void loop(){
//     lv_timer_handler();
//     delay(5);
//     delay(2000);
//     next_auction();
//     delay(2000);
//     /*
//     if(down_button_pressed) next_auction();
//     if(up_button_pressed)   prev_auction();
//     */
// }