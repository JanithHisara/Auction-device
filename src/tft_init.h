#ifndef TFT_INIT_H
#define TFT_INIT_H

#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>  // ✅ Include LVGL here

class TFT_init {
public:
    TFT_init(uint8_t mosi, uint8_t sclk, uint8_t cs, uint8_t dc, uint8_t rst,
             uint16_t width, uint16_t height);

    void begin();
    void fillScreen(uint16_t color);

    // Public method to send pixels to LVGL
    void pushColors(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, lv_color_t* color_p);
    void drawImage(uint16_t x, uint16_t y,
               uint16_t w, uint16_t h,
               const uint16_t *image);

private:
    uint8_t _mosi, _sclk, _cs, _dc, _rst;
    uint16_t _width, _height;

    void cmd(uint8_t c);
    void data(uint8_t d);
    void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
};

#endif // TFT_INIT_H
