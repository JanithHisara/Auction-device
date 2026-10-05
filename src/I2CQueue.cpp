#include "I2CQueue.h"
#include "Hardware.h"

extern KeypadManager keypad;
extern BatteryManager battery;
extern NFCManager nfc;

QueueHandle_t i2cEventQueue = NULL;
volatile bool nfcPollingActive = false;
extern bool bootComplete;

void i2cPollingTask(void * parameter) {
    unsigned long lastBatteryTime = 0;
    unsigned long lastKeyTime = 0;
    
    while(true) {
        if (!bootComplete) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // 1. Poll Keypad (Debounced to ~150ms)
        if (millis() - lastKeyTime > 150) {
            char key = keypad.scan();
            if (key != '\0') {
                I2CEvent ev;
                ev.type = EVENT_KEYPAD_PRESS;
                ev.data.keyPressed = key;
                xQueueSend(i2cEventQueue, &ev, 0);
                lastKeyTime = millis();
            }
        }

        // 2. Poll Battery (Every 5 seconds)
        if (lastBatteryTime == 0 || millis() - lastBatteryTime > 5000) {
            I2CEvent ev;
            ev.type = EVENT_BATTERY_UPDATE;
            ev.data.battery.soc = battery.readSOC();
            ev.data.battery.voltage = battery.readVoltage();
            ev.data.battery.percentage = (uint8_t)battery.readPercent();
            xQueueSend(i2cEventQueue, &ev, 0);
            lastBatteryTime = millis();
        }

        // 3. Poll NFC (Only if requested)
        if (nfcPollingActive) {
            uint8_t uid[7];
            uint8_t uidLength;
            // This blocks for up to 50ms (timeout configured in PN532 setup)
            if (nfc.readUID(uid, &uidLength)) {
                I2CEvent ev;
                ev.type = EVENT_NFC_SCANNED;
                ev.data.nfc.uidLength = uidLength;
                memcpy(ev.data.nfc.uid, uid, uidLength);
                
                xQueueSend(i2cEventQueue, &ev, 0);
                nfcPollingActive = false; // Disable until requested again
            }
        }

        // Yield to allow other tasks to run on Core 0 (like WiFi/Watchdogs)
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void initI2CQueue() {
    i2cEventQueue = xQueueCreate(10, sizeof(I2CEvent));
}

void startI2CPollingTask() {
    xTaskCreatePinnedToCore(i2cPollingTask, "I2CPollTask", 4096, NULL, 1, NULL, 0);
}
