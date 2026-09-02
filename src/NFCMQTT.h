// #ifndef NFC_MQTT_H
// #define NFC_MQTT_H

// #include <MQTTClient.h>
// #include <ArduinoJson.h>
// #include <Arduino.h>

// typedef void (*NFCHandler)(JsonDocument& doc);

// struct NFCAccess {
//     bool Granted;
//     char User_ID[64];
//     char Role[32];
//     int Reason; // if failed
// };

// struct NFCResponse {
//     char Message_ID[64];
//     char NFC_UID[32];
//     char Status[32];
//     NFCAccess Access;
//     String DateTime;
// };

// class NFCMQTT {
// public:
//     NFCMQTT(MQTTClient& client);

//     void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
//     void loop();

//     bool checkAccess(const char* nfcUid, const char* messageId = nullptr);
//     void onAction(const char* action, NFCHandler handler);
//     void handleMessage(char* topic, byte* payload, unsigned int length);

//     NFCResponse lastResponse;

// private:
//     MQTTClient& _mqttClient;
//     const char* _reqTopic;
//     const char* _resTopic;
//     const char* _deviceId;

//     // private struct
//     struct HandlerEntry {
//         const char* action;
//         NFCHandler handler;
//     };

//     HandlerEntry handlers[100];
//     int handlerCount = 0;
// };

// #endif

#ifndef NFCMQTT_H
#define NFCMQTT_H

#include <Arduino.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <functional>

// Define NFCHandler type
typedef std::function<void(JsonDocument&)> NFCHandler;

struct NFCAccessInfo {
    bool Granted;
    char User_ID[64];
    char User_Name[64];
    char Role[32];
    int Reason;
};

struct NFCResponse {
    char Message_ID[64];
    char NFC_UID[32];
    char Status[32];
    NFCAccessInfo Access;
};

class NFCMQTT {
public:
    NFCMQTT(MQTTClient& client);
    
    void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
    void loop();
    bool checkAccess(const char* nfcUid, const char* auctionName = nullptr, const char* messageId = "MSG_0000");
    void onAction(const char* action, NFCHandler handler);
    void handleMessage(char* topic, byte* payload, unsigned int length);
    
    // Public access to last response
    NFCResponse lastResponse;

    // Callback setters
    void setPopupCloseCallback(void (*callback)()) {
        closePopupCallback = callback;
    }
    
    void setNavigationCallback(void (*callback)(const char* userName, const char* userRole, const char* userId)) {
        navigateToItemCallback = callback;
    }

private:
    static const int MAX_HANDLERS = 10;
    struct HandlerEntry {
        const char* action;
        NFCHandler handler;
    } handlers[MAX_HANDLERS];
    int handlerCount = 0;
    
    MQTTClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    const char* _deviceId;
    
    // Store pending UID
    char _pendingUid[32] = "";
    
    // Callback pointers
    void (*closePopupCallback)() = NULL;
    void (*navigateToItemCallback)(const char*, const char*, const char*) = NULL;
};

#endif