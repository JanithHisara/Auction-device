#include "NFCMQTT.h"

NFCMQTT::NFCMQTT(MQTTClient& client)
    : _mqttClient(client) {}

void NFCMQTT::begin(const char* reqTopic, const char* resTopic, const char* deviceId) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    _deviceId = deviceId;

    // _mqttClient.setBufferSize(2048);
}

void NFCMQTT::loop() {
    // Reserved for future
}

bool NFCMQTT::checkAccess(const char* nfcUid, const char* auctionName, const char* messageId) {
    if (!_mqttClient.connected()) {
        Serial.println("⚠️ MQTT not connected, cannot publish NFC request");
        return false;
    }

    if (!messageId) {
        static uint32_t msgCounter = 1000;
        char msgIdBuffer[32];
        snprintf(msgIdBuffer, sizeof(msgIdBuffer), "NFC_%04lu", msgCounter++);
        messageId = msgIdBuffer;
    }

    // Store the UID we're waiting for
    _pendingUid = String(nfcUid);
    
    JsonDocument doc;
    doc["Message_ID"] = messageId;
    doc["Device_ID"]  = _deviceId;
    doc["Action"]     = "CHECK_ACCESS";
    doc["Msg_Type"]   = "request";
    doc["NFC_UID"]    = nfcUid;
    if (auctionName && strlen(auctionName) > 0) {
        doc["Auction_Name"] = auctionName;
    }
    doc["DateTime"]   = millis();  // Consider using ISO format if needed

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';  // ✅ CRITICAL: Null-terminate for MQTT publish

    Serial.print("📤 Sending NFC request for UID: ");
    Serial.print(nfcUid);
    Serial.print(" (");
    Serial.print(n);
    Serial.print(" bytes): ");
    Serial.println(buffer);

    // Use null-terminated string version
    bool result = _mqttClient.publish(_reqTopic, buffer, false, 1);
    
    if (!result) {
        Serial.println("❌ Failed to publish NFC request");
        _pendingUid = ""; // Clear on failure
    }
    
    return result;
}

void NFCMQTT::onAction(const char* action, NFCHandler handler) {
    if (handlerCount < MAX_HANDLERS) {
        handlers[handlerCount].action = action;
        handlers[handlerCount].handler = handler;
        handlerCount++;
    } else {
        Serial.println("⚠️ Max NFC handlers reached");
    }
}

void NFCMQTT::handleMessage(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, _resTopic) != 0) return;

    char* jsonBuffer = (char*)malloc(length + 1);
    if (!jsonBuffer) {
        Serial.println("❌ Memory allocation failed");
        return;
    }

    memcpy(jsonBuffer, payload, length);
    jsonBuffer[length] = '\0';

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (const char*)jsonBuffer);
    free(jsonBuffer);

    if (err) {
        Serial.print("❌ JSON parse failed: "); Serial.println(err.c_str());
        return;
    }

    const char* action = doc["Action"];
    if (!action || strcmp(action, "CHECK_ACCESS") != 0) {
        return;
    }

    const char* status = doc["Status"] | "FAILED";
    const char* messageId = doc["Message_ID"] | "";
    const char* nfcUid = doc["NFC_UID"] | "";
    bool granted = doc["Access"]["Granted"] | false;
    const char* userId = doc["Access"]["User_ID"] | "";
    const char* userName = doc["Access"]["User_Name"] | "";
    const char* userRole = doc["Access"]["Role"] | "";
    int reason = doc["Reason"] | 0;

    Serial.println("\n=== NFC ACCESS RESPONSE ===");
    Serial.print("Action: "); Serial.println(action);
    Serial.print("Message_ID: "); Serial.println(messageId);
    Serial.print("Response UID: "); Serial.println(nfcUid);
    Serial.print("Pending UID: "); Serial.println(_pendingUid);
    Serial.print("Status: "); Serial.println(status);
    Serial.print("Granted: "); Serial.println(granted);

    // CRITICAL: Only process if UID matches
    if (_pendingUid.length() > 0 && _pendingUid != String(nfcUid)) {
        Serial.println("⚠️ UID mismatch - ignoring response (waiting for different card)");
        return;  // Don't process responses for wrong cards
    }

    // Clear pending UID since we got a match
    _pendingUid = "";

    if (granted) {
        Serial.print("User Name: "); Serial.println(userName);
        Serial.print("Role: "); Serial.println(userRole);
    }

    // Fill lastResponse template
    lastResponse.Message_ID = messageId;
    lastResponse.NFC_UID = nfcUid;
    lastResponse.Status = status;
    lastResponse.Access.Granted = granted;
    lastResponse.Access.User_ID = userId;
    lastResponse.Access.User_Name = userName;
    lastResponse.Access.Role = userRole;
    lastResponse.Access.Reason = reason;

    // Call popup close callback if registered
    if (closePopupCallback != NULL) {
        closePopupCallback();
    }

    // Call registered handlers
    for (int i = 0; i < handlerCount; i++) {
        if (strcmp(handlers[i].action, action) == 0) {
            if (handlers[i].handler) {
                handlers[i].handler(doc);
            }
            return;
        }
    }

    // If no handler but navigation callback exists
    if (granted && navigateToItemCallback != NULL) {
        delay(100);
        navigateToItemCallback(userName, userRole, userId);
    }
}