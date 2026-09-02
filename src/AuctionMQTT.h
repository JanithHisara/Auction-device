#ifndef AUCTION_MQTT_H
#define AUCTION_MQTT_H

#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <Arduino.h>

typedef void (*ActionHandler)(JsonDocument& doc);

// Use the same struct name everywhere
struct Auction {
    char Auction_ID[64];
    char Name[64];
    char Auction_Mode[32];
    char Auction_Status[32];
    char Start_DateTime[32];
    char End_DateTime[32];
    int Items_Count;
    int Registered_Count;
};

struct AuctionResponse {
    char Message_ID[64];
    char Status[32];
    std::vector<Auction> Auctions;  // Now using Auction consistently
};

struct HandlerEntry {
    const char* action;
    ActionHandler handler;
};

class AuctionMQTT {
public:
    AuctionMQTT(MQTTClient& mqttClient);

    void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
    void loop();

    bool publishRequest(const char* action, const char* messageId = nullptr);
    void onAction(const char* action, ActionHandler handler);
    void handleMessage(char* topic, byte* payload, unsigned int length);

    AuctionResponse lastResponse;  // stores last GET_AUCTION response

private:
    MQTTClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    const char* _deviceId;

    HandlerEntry handlers[10];
    int handlerCount = 0;
};

#endif