#include "AuctionMQTT.h"

AuctionMQTT::AuctionMQTT(PubSubClient& mqttClient)
    : _mqttClient(mqttClient) {}

void AuctionMQTT::begin(const char* reqTopic, const char* resTopic, const char* deviceId) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    _deviceId = deviceId;

    _mqttClient.setBufferSize(4096); // handle large messages
}

bool AuctionMQTT::publishRequest(const char* action, const char* messageId) {
    if (!_mqttClient.connected()) {
        Serial.println("⚠️ MQTT not connected, cannot publish");
        return false;
    }
    
    char msgIdBuffer[32];
    if (!messageId) {
        static uint32_t msgCounter = 0;
        snprintf(msgIdBuffer, sizeof(msgIdBuffer), "MSG_%04lu", msgCounter++);
        messageId = msgIdBuffer;
    }

    // Use JsonDocument
    JsonDocument doc;
    doc["Message_ID"] = messageId;
    doc["Device_ID"]  = _deviceId;
    doc["Action"]     = action;
    doc["Msg_Type"]   = "request";  // Keep as "request" or change to "request_ESP32" if needed
    doc["DateTime"]   = millis();   // Keep as millis() or change to time string

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';  // ✅ CRITICAL: Null-terminate for MQTT publish

    Serial.print("📤 Sending: ");
    Serial.println(buffer);

    // Use the version that expects a null-terminated string
    return _mqttClient.publish(_reqTopic, buffer);
}

void AuctionMQTT::onAction(const char* action, ActionHandler handler) {
    if (handlerCount < 10) {
        handlers[handlerCount++] = {action, handler};
    } else {
        Serial.println("⚠️ Maximum handlers reached");
    }
}

void AuctionMQTT::handleMessage(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, _resTopic) != 0) return; // Only handle subscribed response topic

    if (length > 32768) { // 32KB limit
        Serial.println("❌ Message too large");
        return;
    }

    char* jsonBuffer = (char*)malloc(length + 1);
    if (!jsonBuffer) {
        Serial.println("❌ Memory allocation failed");
        return;
    }

    memcpy(jsonBuffer, payload, length);
    jsonBuffer[length] = '\0';

    // Use JsonDocument instead of deprecated DynamicJsonDocument
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (const char*)jsonBuffer);
    free(jsonBuffer);

    if (err) {
        Serial.print("❌ JSON parse failed: "); 
        Serial.println(err.c_str());
        return;
    }

    const char* action = doc["Action"];
    if (!action) {
        Serial.println("⚠️ JSON missing Action");
        return;
    }

    // Only process GET_AUCTION automatically
    if (strcmp(action, "GET_AUCTION") == 0) {
        const char* status = doc["Status"] | "UNKNOWN";
        const char* messageId = doc["Message_ID"] | "";

        Serial.print("📥 GET_AUCTION Received - Status: "); 
        Serial.println(status);

        if (strcmp(status, "SUCCESS") == 0) {
            lastResponse.Message_ID = messageId;
            lastResponse.Status = status;
            lastResponse.Auctions.clear();

            // Use doc["Auctions"].is<JsonArray>() instead of containsKey
            if (doc["Auctions"].is<JsonArray>()) {
                JsonArray auctions = doc["Auctions"].as<JsonArray>();
                for (JsonObject auc : auctions) {
                    Auction a;
                    a.Auction_ID       = auc["Auction_ID"].as<const char*>();
                    a.Name             = auc["Name"].as<const char*>();
                    a.Auction_Mode     = auc["Auction_Mode"].as<const char*>();
                    a.Auction_Status   = auc["Auction_Status"].as<const char*>();
                    a.Start_DateTime   = auc["Start_DateTime"].as<const char*>();
                    a.End_DateTime     = auc["End_DateTime"].as<const char*>();
                    a.Items_Count      = auc["Items_Count"].as<int>();
                    a.Registered_Count = auc["Registered_Count"].as<int>();
                    lastResponse.Auctions.push_back(a);
                }
            }
        }
    }

    // Call any user-registered handlers for this action
    for (int i = 0; i < handlerCount; i++) {
        if (strcmp(handlers[i].action, action) == 0) {
            handlers[i].handler(doc);
            return;
        }
    }
}