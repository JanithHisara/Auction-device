import re

with open('src/main.cpp', 'r') as f:
    content = f.read()

# Add includes and Queue variables
includes_replacement = '''#include <Update.h>
#include "I2CQueue.h"

char queuedKey = '\\0';
bool queuedNfcAvailable = false;
uint8_t queuedNfcUid[7];
uint8_t queuedNfcUidLength = 0;

char scanKeypadQueue() {
    char key = queuedKey;
    queuedKey = '\\0';
    return key;
}

void processI2CQueue() {
    I2CEvent ev;
    while(xQueueReceive(i2cEventQueue, &ev, 0)) {
        if (ev.type == EVENT_BATTERY_UPDATE) {
            set_battery_percent(ev.data.battery.percentage);
            // Optionally could store global soc/voltage if needed
        } else if (ev.type == EVENT_KEYPAD_PRESS) {
            if (queuedKey == '\\0') { // Don't overwrite if main loop is slow
                queuedKey = ev.data.keyPressed;
            }
        } else if (ev.type == EVENT_NFC_SCANNED) {
            queuedNfcUidLength = ev.data.nfc.uidLength;
            memcpy(queuedNfcUid, ev.data.nfc.uid, queuedNfcUidLength);
            queuedNfcAvailable = true;
        }
    }
}
'''
content = content.replace('#include <Update.h>', includes_replacement)

# Replace updateBattery() logic
battery_old = '''void updateBattery() {
    if (lastBatteryUpdate == 0 || millis() - lastBatteryUpdate > BATTERY_INTERVAL) {
        lastBatteryUpdate = millis();
        float soc = battery.readSOC();
        float voltage = battery.readVoltage();
        
        uint8_t pct = (uint8_t)battery.readPercent();
        set_battery_percent(pct);
        
        Serial.printf("Battery Voltage: %.2fV, Raw SOC: %.2f%%, Displayed Pct: %d%%\\n", voltage, soc, pct);

        // Low battery logic
        if (pct <= 5 && !lowBatteryWarningShown) {
            show_warning_timeout("\\uF244 Low Battery!", 3000);
            lowBatteryWarningShown = true;
        } else if (pct > 10) {
            lowBatteryWarningShown = false;
        }
    }
}'''
battery_new = '''void updateBattery() {
    // Battery is now updated automatically in processI2CQueue via EVENT_BATTERY_UPDATE
}'''
content = content.replace(battery_old, battery_new)

# Replace readNFC() logic
nfc_old = '''void readNFC() {
    if (millis() - nfcStartTime > NFC_TIMEOUT) {
        nfcState = NFC_IDLE;
        if (currentUI == UI_WAITING_NFC || currentUI == UI_BID_WAIT_NFC) {
            show_warning_timeout("\\uF071 NFC Timeout", 2000);
            currentUI = UI_AUCTION;
        }
        return;
    }
    
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    if (nfc.readUID(uid, &uidLength)) {'''
nfc_new = '''void readNFC() {
    if (millis() - nfcStartTime > NFC_TIMEOUT) {
        nfcState = NFC_IDLE;
        nfcPollingActive = false; // STOP polling in background
        if (currentUI == UI_WAITING_NFC || currentUI == UI_BID_WAIT_NFC) {
            show_warning_timeout("\\uF071 NFC Timeout", 2000);
            currentUI = UI_AUCTION;
        }
        return;
    }
    
    nfcPollingActive = true; // START polling in background
    
    if (queuedNfcAvailable) {
        queuedNfcAvailable = false;
        uint8_t uidLength = queuedNfcUidLength;
        uint8_t* uid = queuedNfcUid;'''
content = content.replace(nfc_old, nfc_new)

# Replace keypad.scan() with scanKeypadQueue() everywhere
content = content.replace('keypad.scan()', 'scanKeypadQueue()')

# Call processI2CQueue() in loop()
loop_old = '''    updateInputs();      // Buttons + keypad + NFC'''
loop_new = '''    processI2CQueue();   // Process background hardware events
    updateInputs();      // Buttons + keypad + NFC'''
content = content.replace(loop_old, loop_new)

# Init queue and start task in setup()
setup_old = '''    keypad.begin(); 
    nfc.begin(); 
    battery.begin(); '''
setup_new = '''    keypad.begin(); 
    nfc.begin(); 
    battery.begin(); 
    
    initI2CQueue();
    startI2CPollingTask();'''
content = content.replace(setup_old, setup_new)

with open('src/main.cpp', 'w') as f:
    f.write(content)
print("Updated main.cpp")
