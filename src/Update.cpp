#include "Update.h"

UpdateMQTT::UpdateMQTT(MQTTClient& client)
    : _mqttClient(client) {}

void UpdateMQTT::begin(const char* reqTopic, const char* resTopic, const char* deviceId) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    _deviceId = deviceId;
    // _mqttClient.setBufferSize(4096);
}

void UpdateMQTT::loop() {
    // Reserved for future
}

/* ---------- Send CHECK_UPDATE Request ---------- */
bool UpdateMQTT::checkUpdate(
    const char* messageId,
    const char* mac,
    const char* fwVersion,
    const char* hwVersion,
    int bootCount
) {
    if (!_mqttClient.connected()) {
        Serial.println("⚠️ MQTT not connected, cannot send CHECK_UPDATE");
        return false;
    }

    JsonDocument doc;

    doc["Message_ID"] = messageId;
    doc["Protocol_Version"] = "1.0";
    doc["Action"] = "CHECK_UPDATE";
    doc["Msg_Type"] = "request";
    doc["MAC_Address"] = mac;
    doc["Device_ID"] = _deviceId;
    doc["Firmware_Version"] = fwVersion;
    doc["Hardware_Version"] = hwVersion;
    doc["Boot_Count"] = bootCount;
    doc["DateTime"] = millis();

    char buffer[512];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';  // ✅ CRITICAL: Null-terminate for MQTT publish

    Serial.println("📤 Sending CHECK_UPDATE:");
    Serial.println(buffer);

    return _mqttClient.publish(_reqTopic, buffer, false, 1);  // Removed length parameter
}
/* ---------- Register Handler ---------- */
void UpdateMQTT::onAction(const char* action, UpdateHandler handler) {
    if (handlerCount < MAX_UPDATE_HANDLERS) {
        handlers[handlerCount++] = {action, handler};
    }
}

/* ---------- Handle Incoming Message ---------- */
void UpdateMQTT::handleMessage(char* topic, byte* payload, unsigned int length) {

    // ✅ Topic filter first
    if (strcmp(topic, _resTopic) != 0) return;

    char* jsonBuffer = (char*)malloc(length + 1);
    if (!jsonBuffer) return;

    memcpy(jsonBuffer, payload, length);
    jsonBuffer[length] = '\0';

    JsonDocument doc; // Safe margin
    DeserializationError err = deserializeJson(doc, (const char*)jsonBuffer);
    free(jsonBuffer);

    if (err) {
        Serial.println("❌ Update JSON parse failed");
        return;
    }

    const char* action = doc["Action"];
    if (!action) return;

    if (strcmp(action, "CHECK_UPDATE") != 0) return;

    const char* status = doc["Status"] | "FAILED";

    Serial.println("📥 CHECK_UPDATE Response Received");

    /* ---------- Fill Template ---------- */
    lastResponse.Message_ID = doc["Message_ID"] | "";
    lastResponse.Status = status;
    lastResponse.Update_Available = doc["Update_Available"] | false;
    lastResponse.Latest_Firmware_Version = doc["Latest_Firmware_Version"] | "";
    lastResponse.Mandatory = doc["Mandatory"] | false;
    lastResponse.Release_Date = doc["Release_Date"] | "";
    lastResponse.Firmware_Size_Bytes = doc["Firmware_Size_Bytes"] | 0;
    lastResponse.Download_URL = doc["Download_URL"] | "";
    lastResponse.DateTime = doc["DateTime"] | "";

    /* ---------- Call Handlers ---------- */
    for (int i = 0; i < handlerCount; i++) {
        if (strcmp(handlers[i].action, action) == 0) {
            handlers[i].handler(doc);
            return;
        }
    }
}