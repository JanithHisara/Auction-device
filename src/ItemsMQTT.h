#ifndef ITEMS_MQTT_H
#define ITEMS_MQTT_H

#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <string>

struct Item {
    char Item_ID[64];
    char Name[64];
    char Status[32];
    double Current_Price = 0.0;
    char Currency[8];
    double Next_Min_Bid = 0.0;   // Only for English auction
    char End_DateTime[32];
    uint32_t Remaining_Seconds = 0;
    bool Your_Bid_Submitted = false; // Only for Closed auction
};

struct GetItemsResponse {
    char Message_ID[64];
    char Auction_Name[64];
    char Auction_Mode[32];
    char Auction_Status[32];
    char Status[32];
    uint32_t Items_Count = 0;
    std::vector<Item> Items;
};

typedef void (*ItemsHandler)(JsonDocument& doc);

class ItemsMQTT {
public:
    ItemsMQTT(MQTTClient& client);

    void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
    void loop();

    bool publishRequest(const char* auctionName, const char* messageId = nullptr, const char* nfcUid = nullptr);
    void onAction(const char* action, ItemsHandler handler);
    void handleMessage(char* topic, byte* payload, unsigned int length);

    // Last parsed response template
    GetItemsResponse lastResponse;

private:
    MQTTClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    const char* _deviceId;

    struct HandlerEntry {
        const char* action;
        ItemsHandler handler;
    };
    HandlerEntry handlers[20];
    int handlerCount = 0;
};

#endif
