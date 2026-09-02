// #include "Button.h"

// Button::Button(uint8_t pin, uint16_t debounce)
// {
//     _pin = pin;
//     _debounceDelay = debounce;
// }

// void Button::begin()
// {
//     pinMode(_pin, INPUT);   // external pull-up wiring
//     _state = HIGH;
//     _lastReading = HIGH;
//     _pressEvent = false;
//     _lastChangeTime = 0;
// }

// void Button::update()
// {
//     bool reading = digitalRead(_pin);

//     if (reading != _lastReading) {
//         _lastChangeTime = millis();
//     }

//     if ((millis() - _lastChangeTime) > _debounceDelay) {
//         if (reading != _state) {
//             _state = reading;

//             if (_state == LOW) {
//                 _pressEvent = true;
//             }
//         }
//     }

//     _lastReading = reading;
// }

// bool Button::pressed()
// {
//     if (_pressEvent) {
//         _pressEvent = false;
//         return true;
//     }
//     return false;
// }

// bool Button::isDown()
// {
//     return (_state == LOW);
// }
#include "Button.h"

Button::Button(uint8_t pin, uint16_t debounce)
{
    _pin = pin;
    _debounceDelay = debounce;
    _initialized = false;
}

void Button::begin()
{
    pinMode(_pin, INPUT);   // use external pull-up
    _state = HIGH;
    _lastReading = HIGH;
    _pressEvent = false;
    _lastChangeTime = 0;
    _initialized = false;   // ensure first read is ignored
}

void Button::update()
{
    bool reading = digitalRead(_pin);

    // Ignore first unstable reading
    if (!_initialized) {
        _state = reading;
        _lastReading = reading;
        _initialized = true;
        _pressEvent = false;
        return;   // skip first update
    }

    // Detect change
    if (reading != _lastReading) {
        _lastChangeTime = millis();
    }

    // If stable past debounce delay
    if ((millis() - _lastChangeTime) > _debounceDelay) {
        if (reading != _state) {
            _state = reading;

            // Trigger press event only when LOW (button pressed)
            if (_state == LOW) {
                _pressEvent = true;
            }
        }
    }

    _lastReading = reading;
}

bool Button::pressed()
{
    if (_pressEvent) {
        _pressEvent = false;
        return true;
    }
    return false;
}

bool Button::isDown()
{
    return (_state == LOW);
}
