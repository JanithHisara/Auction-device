#include "Register.h"

RegisterMQTT::RegisterMQTT(PubSubClient& client) : _mqttClient(client) {}

void RegisterMQTT::begin(const char* reqTopic, const char* resTopic, const char* macAddress,
                         const char* firmwareVersion, const char* hardwareVersion, int bootCount) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    _macAddress = macAddress;
    _firmwareVersion = firmwareVersion;
    _hardwareVersion = hardwareVersion;
    _bootCount = bootCount;

    _mqttClient.setBufferSize(1024); // Adjust if needed
}

void RegisterMQTT::loop() {
    // Reserved for future
}

bool RegisterMQTT::publishRequest(const char* messageId) {
    if (!messageId) messageId = "MSG_0000";

    JsonDocument doc;
    doc["Message_ID"] = messageId;
    doc["Protocol_Version"] = "1.0";
    doc["Action"] = "DEVICE_REGISTER";
    doc["Msg_Type"] = "request";
    doc["MAC_Address"] = _macAddress;
    doc["Device_ID"] = nullptr;
    doc["Firmware_Version"] = _firmwareVersion;
    doc["Hardware_Version"] = _hardwareVersion;
    doc["Boot_Count"] = _bootCount;
    doc["DateTime"] = millis(); // replace with RTC time if needed

    char buffer[512];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';  // ✅ CRITICAL: Null-terminate for MQTT publish

    Serial.print("📤 Sending DEVICE_REGISTER request: ");
    Serial.println(buffer);

    return _mqttClient.publish(_reqTopic, buffer);  // Removed length parameter
}

void RegisterMQTT::onAction(const char* action, RegisterHandler handler) {
    if (handlerCount < MAX_REG_HANDLERS) {
        handlers[handlerCount++] = {action, handler};
    } else {
        Serial.println("⚠️ Max register handlers reached");
    }
}

void RegisterMQTT::handleMessage(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, _resTopic) != 0) return; // Only handle registered response topic

    char* jsonBuffer = (char*)malloc(length + 1);
    if (!jsonBuffer) {
        Serial.println("❌ Memory allocation failed");
        return;
    }

    memcpy(jsonBuffer, payload, length);
    jsonBuffer[length] = '\0';

    JsonDocument doc; // Safe margin
    DeserializationError err = deserializeJson(doc, (const char*)jsonBuffer);
    free(jsonBuffer);

    if (err) {
        Serial.print("❌ JSON parse failed: "); 
        Serial.println(err.c_str());
        return;
    }

    const char* action = doc["Action"] | "";
    if (strcmp(action, "DEVICE_REGISTER") != 0) return; // Filter: only process DEVICE_REGISTER

    const char* status = doc["Status"] | "FAILED";
    const char* messageId = doc["Message_ID"] | "";

    Serial.print("📥 Register Action: "); Serial.println(action);
    Serial.print("Message_ID: "); Serial.println(messageId);
    Serial.print("Status: "); Serial.println(status);

    // Fill lastResponse
    lastResponse.Message_ID = messageId;
    lastResponse.Protocol_Version = doc["Protocol_Version"] | "";
    lastResponse.Status = status;
    lastResponse.MAC_Address = doc["MAC_Address"] | "";
    lastResponse.Device_ID = doc["Device_ID"] | "";
    lastResponse.Heartbeat_Interval = doc["Heartbeat_Interval"] | 0;
    lastResponse.DateTime = doc["DateTime"] | "";

    // Call user-registered handlers
    for (int i = 0; i < handlerCount; i++) {
        if (strcmp(handlers[i].action, action) == 0) {
            handlers[i].handler(doc);
            return;
        }
    }

}