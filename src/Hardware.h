// #ifndef HARDWARE_H
// #define HARDWARE_H

// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_MCP23X17.h>
// #include <Adafruit_PN532.h>

// //////////////////////
// // Keypad
// //////////////////////
// class KeypadManager {
// public:
//     KeypadManager(Adafruit_MCP23X17* mcp, const uint8_t* rows, const uint8_t* cols, char keymap[5][3]);
//     void begin();
//     char scan();  // returns key pressed or 0
// private:
//     Adafruit_MCP23X17* _mcp;
//     const uint8_t* _rowPins;
//     const uint8_t* _colPins;
//     char (*_keys)[3];
// };

// //////////////////////
// // NFC
// //////////////////////
// class NFCManager {
// public:
//     NFCManager(uint8_t irq=-1, uint8_t reset=-1);
//     void begin();
//     bool readUID(uint8_t* uid, uint8_t* length); // returns true if tag detected
// private:
//     Adafruit_PN532 _nfc;
// };

// //////////////////////
// // Battery / Fuel Gauge
// //////////////////////
// class BatteryManager {
// public:
//     BatteryManager(uint8_t addr = 0x36, float minV = 3.3, float maxV = 4.15);
//     void begin();
//     float readVoltage();  // in volts
//     float readPercent();  // 0-100% based on 3.3-4.15V
// private:
//     uint8_t _addr;
//     float _minVoltage;
//     float _maxVoltage;
// };

// #endif
#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_PN532.h>

//////////////////////
// Keypad
//////////////////////
class KeypadManager {
public:
    KeypadManager(Adafruit_MCP23X17* mcp, const uint8_t* rows, const uint8_t* cols, char keymap[4][4]);
    void begin();
    char scan();  // returns key pressed or 0
private:
    Adafruit_MCP23X17* _mcp;
    const uint8_t* _rowPins;
    const uint8_t* _colPins;
    char (*_keys)[4];
};

//////////////////////
// NFC
//////////////////////
class NFCManager {
public:
    NFCManager(uint8_t irq=-1, uint8_t reset=-1);
    void begin();
    bool readUID(uint8_t* uid, uint8_t* length); // returns true if tag detected
private:
    Adafruit_PN532 _nfc;
};

//////////////////////
// Battery / Fuel Gauge
//////////////////////
class BatteryManager {
public:
    BatteryManager(uint8_t addr = 0x36, float minV = 3.3, float maxV = 4.15);
    void begin();
    void setVoltageRange(float minV, float maxV); // new
    float readVoltage();  // in volts
    float readPercent();  // 0-100% based on min/max voltage
      float readSOC();
private:
    uint8_t _addr;
    float _minVoltage;
    float _maxVoltage;
};

//////////////////////
// LEDs via MCP
//////////////////////
class LEDManager {
public:
    LEDManager(Adafruit_MCP23X17* mcp, const uint8_t* pins, uint8_t count);
    void begin();
    void set(uint8_t index, bool state);  // turn on/off single LED
    void toggle(uint8_t index);           // toggle LED state
    void allOff();
    void allOn();
private:
    Adafruit_MCP23X17* _mcp;
    const uint8_t* _pins;
    uint8_t _count;
    bool* _states;
};

#endif
