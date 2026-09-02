#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define MAX_REG_HANDLERS 5

struct RegisterResponse {
    String Message_ID;
    String Protocol_Version;
    String Status;
    String MAC_Address;
    String Device_ID;
    int Heartbeat_Interval;
    String DateTime;
};

typedef void (*RegisterHandler)(JsonDocument& doc);

struct RegisterHandlerEntry {
    const char* action;
    RegisterHandler handler;
};

class RegisterMQTT {
public:
    RegisterMQTT(PubSubClient& client);

    void begin(const char* reqTopic, const char* resTopic, const char* macAddress,
               const char* firmwareVersion, const char* hardwareVersion, int bootCount);

    void loop();

    bool publishRequest(const char* messageId = nullptr);

    void onAction(const char* action, RegisterHandler handler);

    void handleMessage(char* topic, byte* payload, unsigned int length);

    RegisterResponse lastResponse;

private:
    PubSubClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    const char* _macAddress;
    const char* _firmwareVersion;
    const char* _hardwareVersion;
    int _bootCount;

    RegisterHandlerEntry handlers[MAX_REG_HANDLERS];
    int handlerCount = 0;
};
