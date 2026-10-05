#include "I2CMutex.h"

SemaphoreHandle_t i2cMutex = NULL;

void initI2CMutex() {
    if (i2cMutex == NULL) {
        i2cMutex = xSemaphoreCreateMutex();
    }
}

void lockI2C() {
    if (i2cMutex != NULL) {
        xSemaphoreTake(i2cMutex, portMAX_DELAY);
    }
}

void unlockI2C() {
    if (i2cMutex != NULL) {
        xSemaphoreGive(i2cMutex);
    }
}
