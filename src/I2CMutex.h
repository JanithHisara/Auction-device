#ifndef I2C_MUTEX_H
#define I2C_MUTEX_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Global mutex for I2C bus access
extern SemaphoreHandle_t i2cMutex;

// Initialize the I2C mutex
void initI2CMutex();

// Helper functions for easy locking/unlocking
void lockI2C();
void unlockI2C();

#endif // I2C_MUTEX_H
