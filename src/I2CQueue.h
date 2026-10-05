#ifndef I2C_QUEUE_H
#define I2C_QUEUE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

typedef enum {
    EVENT_BATTERY_UPDATE,
    EVENT_KEYPAD_PRESS,
    EVENT_NFC_SCANNED
} I2CEventType;

typedef struct {
    I2CEventType type;
    union {
        struct {
            float soc;
            float voltage;
            uint8_t percentage;
        } battery;
        
        char keyPressed;
        
        struct {
            uint8_t uid[7];
            uint8_t uidLength;
        } nfc;
    } data;
} I2CEvent;

extern QueueHandle_t i2cEventQueue;
extern volatile bool nfcPollingActive;

void initI2CQueue();
void startI2CPollingTask();

#endif // I2C_QUEUE_H
