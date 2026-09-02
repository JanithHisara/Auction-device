#include "ItemsMQTT.h"

ItemsMQTT::ItemsMQTT(MQTTClient& client) : _mqttClient(client) {}

void ItemsMQTT::begin(const char* reqTopic, const char* resTopic, const char* deviceId) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    _deviceId = deviceId;
    // _mqttClient.setBufferSize(4096); // Handle large messages
}

void ItemsMQTT::loop() {
    // Reserved for future
}

// bool ItemsMQTT::publishRequest(const char* auctionName, const char* messageId, const char* nfcUid) {
//     if (!messageId) messageId = "MSG_0000";

//     StaticJsonDocument<256> doc;
//     doc["Message_ID"] = messageId;
//     doc["Device_ID"]  = _deviceId;
//     doc[\"Action\"]     = \"GET_ITEMS\";
//     doc["Msg_Type"]   = "request";
//     doc["Auction_Name"] = auctionName;
//     doc["DateTime"]   = millis();

//     char buffer[256];
//     size_t len = serializeJson(doc, buffer);

//     Serial.print("📤 Sending GET_ITEMS request: ");
//     Serial.println(buffer);

//     return _mqttClient.publish(_reqTopic, buffer, len);
// }

bool ItemsMQTT::publishRequest(const char* auctionName, const char* messageId, const char* nfcUid) {
    if (!_mqttClient.connected()) {
        Serial.println("⚠️ MQTT not connected, cannot publish GET_ITEMS");
        return false;
    }

    if (!messageId) {
        static uint32_t msgCounter = 0;
        char msgIdBuffer[32];
        snprintf(msgIdBuffer, sizeof(msgIdBuffer), "ITEMS_%04lu", msgCounter++);
        messageId = msgIdBuffer;
    }

    // Use JsonDocument
    JsonDocument doc;
    doc["Message_ID"] = messageId;
    doc["Device_ID"]  = _deviceId;
    doc["Action"]     = "GET_ITEMS";
    if (nfcUid && strlen(nfcUid) > 0) doc["NFC_UID"] = nfcUid;
    doc["Msg_Type"]   = "request";
    doc["Auction_Name"] = auctionName;
    doc["DateTime"]   = millis();

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';  // ✅ CRITICAL: Null-terminate for MQTT publish

    Serial.print("📤 Sending GET_ITEMS request (");
    Serial.print(n);
    Serial.print(" bytes): ");
    Serial.println(buffer);

    // Use null-terminated string version
    bool result = _mqttClient.publish(_reqTopic, buffer, false, 1);
    
    if (!result) {
        Serial.println("❌ Failed to publish GET_ITEMS request");
        Serial.print("MQTT state: ");
        Serial.println(_mqttClient.lastError());
    }
    
    return result;
}

void ItemsMQTT::onAction(const char* action, ItemsHandler handler) {
    if (handlerCount < 20) {
        handlers[handlerCount++] = {action, handler};
    } else {
        Serial.println("⚠️ Max items handlers reached");
    }
}

void ItemsMQTT::handleMessage(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, _resTopic) != 0) return; // Only handle subscribed response topic

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
        Serial.print("❌ JSON parse failed: "); 
        Serial.println(err.c_str());
        return;
    }

    const char* action = doc["Action"] | "";
    if (strcmp(action, "GET_ITEMS") != 0) return; // Filter: only process GET_ITEMS

    const char* status = doc["Status"] | "FAILED";
    const char* messageId = doc["Message_ID"] | "";
    const char* auctionName = doc["Auction_ID"] | "";
    const char* auctionMode = doc["Auction_Mode"] | "";
    const char* auctionStatus = doc["Auction_Status"] | "";

    Serial.print("🔹 GET_ITEMS Response - Auction_Name: "); Serial.println(auctionName);
    Serial.print("Status: "); Serial.println(status);

    if (strcmp(status, "SUCCESS") == 0) {
        strlcpy(lastResponse.Message_ID, messageId, sizeof(lastResponse.Message_ID));
        strlcpy(lastResponse.Auction_Name, auctionName, sizeof(lastResponse.Auction_Name));
        strlcpy(lastResponse.Auction_Mode, auctionMode, sizeof(lastResponse.Auction_Mode));
        strlcpy(lastResponse.Auction_Status, auctionStatus, sizeof(lastResponse.Auction_Status));
        strlcpy(lastResponse.Status, status, sizeof(lastResponse.Status));
        lastResponse.Items_Count = doc["Items_Count"] | 0;
        lastResponse.Items.clear();

        if (doc["Items"].is<JsonArray>()) {
            JsonArray itemsArray = doc["Items"];
            for (JsonObject it : itemsArray) {
                Item item;
                strlcpy(item.Item_ID, it["Item_ID"] | "", sizeof(item.Item_ID));
                strlcpy(item.Name, it["Name"] | "", sizeof(item.Name));
                strlcpy(item.Status, it["Status"] | "", sizeof(item.Status));
                strlcpy(item.Currency, it["Currency"] | "", sizeof(item.Currency));
                strlcpy(item.End_DateTime, it["End_DateTime"] | "", sizeof(item.End_DateTime));
                item.Remaining_Seconds = it["Remaining_Seconds"] | 0;

                if (strcmp(auctionMode, "ENGLISH") == 0 || strcmp(auctionMode, "OPEN") == 0 || strcmp(auctionMode, "English auction") == 0 || strcmp(auctionMode, "Progressive elimination") == 0) {
                    item.Current_Price = it["Current_Price"] | 0.0;
                    item.Next_Min_Bid = it["Next_Min_Bid"] | 0.0;
                } else if (strcmp(auctionMode, "CLOSED") == 0 || strcmp(auctionMode, "TENDER") == 0 || strcmp(auctionMode, "Sealed bid auction") == 0) {
                    item.Your_Bid_Submitted = it["Your_Bid_Submitted"] | false;
                }

                lastResponse.Items.push_back(item);
            }
        }
    }

    // Call user-registered handlers
    for (int i = 0; i < handlerCount; i++) {
        if (strcmp(handlers[i].action, action) == 0) {
            handlers[i].handler(doc);
            return;
        }
    }
}