#ifndef UPDATE_MQTT_H
#define UPDATE_MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <vector>

#define MAX_UPDATE_HANDLERS 10

typedef std::function<void(JsonDocument&)> UpdateHandler;

/* ---------- Handler Entry ---------- */
struct UpdateHandlerEntry {
    const char* action;
    UpdateHandler handler;
};

/* ---------- Response Template ---------- */
struct UpdateResponse {
    String Message_ID;
    String Status;
    bool Update_Available;
    String Latest_Firmware_Version;
    bool Mandatory;
    String Release_Date;
    long Firmware_Size_Bytes;
    String Download_URL;
    String DateTime;
};

class UpdateMQTT {
public:
    UpdateMQTT(PubSubClient& client);

    void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
    void loop();

    bool checkUpdate(
        const char* messageId,
        const char* mac,
        const char* fwVersion,
        const char* hwVersion,
        int bootCount
    );

    void onAction(const char* action, UpdateHandler handler);
    void handleMessage(char* topic, byte* payload, unsigned int length);

    UpdateResponse lastResponse;

private:
    PubSubClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    const char* _deviceId;

    UpdateHandlerEntry handlers[MAX_UPDATE_HANDLERS];
    int handlerCount = 0;
};

#endif
