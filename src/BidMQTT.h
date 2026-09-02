#ifndef BID_MQTT_H
#define BID_MQTT_H

#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <string>

using std::string;
using std::vector;

// Callback type for handling bid responses
typedef void (*BidHandler)(JsonDocument& doc);

// Bid template for last response
struct BidResponse {
    string Message_ID;
    string Status;
    string Auction_Name;
    string Auction_Mode;
    string Auction_Status;
    string Item_ID;
    string NFC_UID;
    string Bid_Status;
    float Current_Highest_Bid;
    float Next_Min_Bid;
    string Currency;
    int Reason; // Only if FAILED
};

struct BidHandlerEntry {
    const char* action;
    BidHandler handler;
};

class BidMQTT {
public:
    BidMQTT(MQTTClient& mqttClient);

    void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
    void loop();

    // Publish a bid request
    bool submitBid(const char* auctionName, const char* itemId, const char* nfcUid,
                   float bidAmount, const char* currency, const char* messageId = nullptr);

    // Register a callback
    void onAction(const char* action, BidHandler handler);

    // Handle incoming MQTT messages
    void handleMessage(char* topic, byte* payload, unsigned int length);

    // Last parsed response
    BidResponse lastResponse;

private:
    MQTTClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    const char* _deviceId;

    static const int MAX_HANDLERS = 10;
    BidHandlerEntry handlers[MAX_HANDLERS];
    int handlerCount = 0;
};

#endif
