#include "BidMQTT.h"

BidMQTT::BidMQTT(MQTTClient& mqttClient)
    : _mqttClient(mqttClient) {}

void BidMQTT::begin(const char* reqTopic, const char* resTopic, const char* deviceId) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    _deviceId = deviceId;

    // _mqttClient.setBufferSize(2048); // Enough for bid JSON
}

void BidMQTT::loop() {
    // Reserved for future tasks
}

bool BidMQTT::submitBid(const char* auctionName, const char* itemId, const char* nfcUid,
                        float bidAmount, const char* currency, const char* messageId) {

    if (!messageId) messageId = "MSG_2000";

    JsonDocument doc;
    doc["Message_ID"] = messageId;
    doc["Device_ID"]  = _deviceId;
    doc["Action"]     = "SUBMIT_BID";
    doc["Msg_Type"]   = "request";
    doc["Auction_Name"] = auctionName;
    doc["Item_ID"]    = itemId;
    doc["NFC_UID"]    = nfcUid;
    doc["Bid_Amount"] = bidAmount;
    doc["Currency"]   = currency;
    doc["DateTime"]   = millis(); // Replace with RTC if needed

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';  // ✅ CRITICAL: Null-terminate for MQTT publish

    Serial.print("📤 Sending Bid: ");
    Serial.println(buffer);

    return _mqttClient.publish(_reqTopic, buffer, false, 1);  // Removed length parameter
}

void BidMQTT::onAction(const char* action, BidHandler handler) {
    if (handlerCount < MAX_HANDLERS) {
        handlers[handlerCount++] = {action, handler};
    } else {
        Serial.println("⚠️ Max bid handlers reached");
    }
}

void BidMQTT::handleMessage(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, _resTopic) != 0) return; // Only handle bid response topic

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
    if (strcmp(action, "SUBMIT_BID") != 0) return; // Filter: only process SUBMIT_BID

    const char* status = doc["Status"] | "FAILED";
    const char* messageId = doc["Message_ID"] | "";
    const char* auctionName = doc["Auction_Name"] | "";
    const char* auctionMode = doc["Auction_Mode"] | "";
    const char* auctionStatus = doc["Auction_Status"] | "";
    const char* itemId = doc["Item_ID"] | "";
    const char* nfcUid = doc["NFC_UID"] | "";
    const char* bidStatus = doc["Bid_Status"] | "";
    float currentBid = doc["Current_Highest_Bid"] | 0.0;
    float nextBid    = doc["Next_Min_Bid"] | 0.0;
    const char* currency = doc["Currency"] | "";
    int reason = doc["Reason"] | 0;

    Serial.print("📥 Bid Action: "); Serial.println(action);
    Serial.print("Message_ID: "); Serial.println(messageId);
    Serial.print("Status: "); Serial.println(status);

    // Fill lastResponse
    strlcpy(lastResponse.Message_ID, messageId, sizeof(lastResponse.Message_ID));
    strlcpy(lastResponse.Status, status, sizeof(lastResponse.Status));
    strlcpy(lastResponse.Auction_Name, auctionName, sizeof(lastResponse.Auction_Name));
    strlcpy(lastResponse.Auction_Mode, auctionMode, sizeof(lastResponse.Auction_Mode));
    strlcpy(lastResponse.Auction_Status, auctionStatus, sizeof(lastResponse.Auction_Status));
    strlcpy(lastResponse.Item_ID, itemId, sizeof(lastResponse.Item_ID));
    strlcpy(lastResponse.NFC_UID, nfcUid, sizeof(lastResponse.NFC_UID));
    strlcpy(lastResponse.Bid_Status, bidStatus, sizeof(lastResponse.Bid_Status));
    lastResponse.Current_Highest_Bid = currentBid;
    lastResponse.Next_Min_Bid = nextBid;
    strlcpy(lastResponse.Currency, currency, sizeof(lastResponse.Currency));
    lastResponse.Reason = reason;

    // Call user-registered handlers
    for (int i = 0; i < handlerCount; i++) {
        if (strcmp(handlers[i].action, action) == 0) {
            handlers[i].handler(doc);
            return;
        }
    }
}