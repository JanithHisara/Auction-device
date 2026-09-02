// #ifndef BUTTON_H
// #define BUTTON_H

// #include <Arduino.h>

// class Button {
// public:
//     Button(uint8_t pin, uint16_t debounce = 50);

//     void begin();
//     void update();

//     bool pressed();   // true once per press
//     bool isDown();    // button currently held

// private:
//     uint8_t _pin;
//     uint16_t _debounceDelay;

//     bool _state;
//     bool _lastReading;
//     bool _pressEvent;

//     unsigned long _lastChangeTime;
// };

// #endif
#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
public:
    Button(uint8_t pin, uint16_t debounce = 50);

    void begin();
    void update();

    bool pressed();   // true once per press
    bool isDown();    // button currently held

private:
    uint8_t _pin;
    uint16_t _debounceDelay;

    bool _state;          // stable state
    bool _lastReading;    // last read value
    bool _pressEvent;     // flag for pressed

    unsigned long _lastChangeTime;
    bool _initialized;    // ignore first unstable reading
};

#endif
