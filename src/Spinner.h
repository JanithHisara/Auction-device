#ifndef SPINNER_H
#define SPINNER_H

#include <lvgl.h>

class Spinner {
private:
    static lv_obj_t* window;
    static lv_obj_t* message_label;
    static lv_obj_t* spinner;
    static bool initialized;

public:
    static void begin();
    static void start(const char* message);
    static void stop();
    static void message(const char* text);
    static bool isRunning();
};

#endif