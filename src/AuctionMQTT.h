#ifndef AUCTION_MQTT_H
#define AUCTION_MQTT_H

#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <Arduino.h>

typedef void (*ActionHandler)(JsonDocument& doc);

// Use the same struct name everywhere
struct Auction {
    String Auction_ID;
    String Name;
    String Auction_Mode;
    String Auction_Status;
    String Start_DateTime;
    String End_DateTime;
    int Items_Count;
    int Registered_Count;
};

struct AuctionResponse {
    String Message_ID;
    String Status;
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