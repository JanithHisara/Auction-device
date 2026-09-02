// #include "Hardware.h"
// // KeypadManager
// KeypadManager::KeypadManager(Adafruit_MCP23X17* mcp, const uint8_t* rows, const uint8_t* cols, char keymap[5][3]) {
//     _mcp = mcp;
//     _rowPins = rows;
//     _colPins = cols;
//     _keys = keymap;
// }

// void KeypadManager::begin() {
//     for (int i = 0; i < 5; i++) _mcp->pinMode(_rowPins[i], INPUT_PULLUP);
//     for (int i = 0; i < 3; i++) {
//         _mcp->pinMode(_colPins[i], OUTPUT);
//         _mcp->digitalWrite(_colPins[i], HIGH);
//     }
// }

// char KeypadManager::scan() {
//     for (int c = 0; c < 3; c++) {
//         _mcp->digitalWrite(_colPins[c], LOW);
//         for (int r = 0; r < 5; r++) {
//             if (_mcp->digitalRead(_rowPins[r]) == LOW) {
//                 _mcp->digitalWrite(_colPins[c], HIGH);
//                 return _keys[r][c];
//             }
//         }
//         _mcp->digitalWrite(_colPins[c], HIGH);
//     }
//     return 0;
// }


// // NFCManager
// NFCManager::NFCManager(uint8_t irq, uint8_t reset): _nfc(irq, reset) {}
// void NFCManager::begin() {
//     _nfc.begin();
//     if (!_nfc.getFirmwareVersion()) {
//         Serial.println("PN532 not found!");
//         while(1);
//     }
//     _nfc.SAMConfig();
// }
// bool NFCManager::readUID(uint8_t* uid, uint8_t* length) {
//     return _nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, length, 50);
// }


// // BatteryManager
// BatteryManager::BatteryManager(uint8_t addr, float minV, float maxV) {
//     _addr = addr;
//     _minVoltage = minV;
//     _maxVoltage = maxV;
// }

// void BatteryManager::begin() {
//     Wire.beginTransmission(_addr);
//     Wire.write(0x06); // quick start
//     Wire.write(0x40);
//     Wire.write(0x00);
//     Wire.endTransmission();
// }

// float BatteryManager::readVoltage() {
//     Wire.beginTransmission(_addr);
//     Wire.write(0x02);
//     Wire.endTransmission(false);

//     Wire.requestFrom(_addr, 2);
//     uint16_t raw = (Wire.read()<<8) | Wire.read();
//     return raw * 0.000078125;
// }

// float BatteryManager::readPercent() {
//     float v = readVoltage();
//     float pct = ((v - _minVoltage)/(_maxVoltage - _minVoltage))*100.0;
//     if(pct < 0) pct = 0;
//     if(pct > 100) pct = 100;
//     return pct;
// }

#include "Hardware.h"

//////////////////////
// KeypadManager
//////////////////////
KeypadManager::KeypadManager(Adafruit_MCP23X17* mcp, const uint8_t* rows, const uint8_t* cols, char keymap[4][4]) {
    _mcp = mcp;
    _rowPins = rows;
    _colPins = cols;
    _keys = keymap;
}

void KeypadManager::begin() {
    for (int i = 0; i < 4; i++) _mcp->pinMode(_rowPins[i], INPUT_PULLUP);
    for (int i = 0; i < 4; i++) {
        _mcp->pinMode(_colPins[i], OUTPUT);
        _mcp->digitalWrite(_colPins[i], HIGH);
    }
}

char KeypadManager::scan() {
    for (int c = 0; c < 4; c++) {
        _mcp->digitalWrite(_colPins[c], LOW);
        for (int r = 0; r < 4; r++) {
            if (_mcp->digitalRead(_rowPins[r]) == LOW) {
                _mcp->digitalWrite(_colPins[c], HIGH);
                return _keys[r][c];
            }
        }
        _mcp->digitalWrite(_colPins[c], HIGH);
    }
    return 0;
}

//////////////////////
// NFCManager
//////////////////////
NFCManager::NFCManager(uint8_t irq, uint8_t reset): _nfc(irq, reset) {}

void NFCManager::begin() {
    _nfc.begin();
    if (!_nfc.getFirmwareVersion()) {
        Serial.println("PN532 not found!");
        while(1);
    }
    _nfc.SAMConfig();
}

bool NFCManager::readUID(uint8_t* uid, uint8_t* length) {
    return _nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, length, 50);
}

//////////////////////
// BatteryManager
//////////////////////
BatteryManager::BatteryManager(uint8_t addr, float minV, float maxV) {
    _addr = addr;
    _minVoltage = minV;
    _maxVoltage = maxV;
}

void BatteryManager::setVoltageRange(float minV, float maxV) {
    _minVoltage = minV;
    _maxVoltage = maxV;
}

void BatteryManager::begin() {
    Wire.beginTransmission(_addr);
    Wire.write(0x06); // quick start
    Wire.write(0x40);
    Wire.write(0x00);
    Wire.endTransmission();
}

float BatteryManager::readVoltage() {
    Wire.beginTransmission(_addr);
    Wire.write(0x02);
    Wire.endTransmission(false);

    Wire.requestFrom(_addr, 2);
    uint16_t raw = (Wire.read()<<8) | Wire.read();
    return raw * 0.000078125;
}

float BatteryManager::readSOC() {
    Wire.beginTransmission(_addr);
    Wire.write(0x04);
    Wire.endTransmission(false);

    Wire.requestFrom((uint8_t)_addr, (uint8_t)2);
    uint16_t raw = (Wire.read()<<8) | Wire.read();
    return (float)((raw >> 8) + (raw & 0xFF) / 256.0);
}

float BatteryManager::readPercent() {
    float soc = readSOC();
    float pct = ((soc - 20.0) / (90.0 - 20.0)) * 100.0;
    if(pct < 0) pct = 0;
    if(pct > 100) pct = 100;
    return pct;
}

//////////////////////
// LEDManager
//////////////////////
LEDManager::LEDManager(Adafruit_MCP23X17* mcp, const uint8_t* pins, uint8_t count) {
    _mcp = mcp;
    _pins = pins;
    _count = count;
    _states = new bool[count];
    for(int i=0;i<count;i++) _states[i] = false;
}

void LEDManager::begin() {
    for(int i=0;i<_count;i++){
        _mcp->pinMode(_pins[i], OUTPUT);
        _mcp->digitalWrite(_pins[i], LOW);
    }
}

void LEDManager::set(uint8_t index, bool state){
    if(index >= _count) return;
    _states[index] = state;
    _mcp->digitalWrite(_pins[index], state ? HIGH : LOW);
}

void LEDManager::toggle(uint8_t index){
    if(index >= _count) return;
    _states[index] = !_states[index];
    _mcp->digitalWrite(_pins[index], _states[index] ? HIGH : LOW);
}

void LEDManager::allOff(){
    for(int i=0;i<_count;i++) set(i,false);
}

void LEDManager::allOn(){
    for(int i=0;i<_count;i++) set(i,true);
}
