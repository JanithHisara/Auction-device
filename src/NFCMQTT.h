// #ifndef NFC_MQTT_H
// #define NFC_MQTT_H

// #include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include <Arduino.h>

// typedef void (*NFCHandler)(JsonDocument& doc);

// struct NFCAccess {
//     bool Granted;
//     String User_ID;
//     String Role;
//     int Reason; // if failed
// };

// struct NFCResponse {
//     String Message_ID;
//     String NFC_UID;
//     String Status;
//     NFCAccess Access;
//     String DateTime;
// };

// class NFCMQTT {
// public:
//     NFCMQTT(PubSubClient& client);

//     void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
//     void loop();

//     bool checkAccess(const char* nfcUid, const char* messageId = nullptr);
//     void onAction(const char* action, NFCHandler handler);
//     void handleMessage(char* topic, byte* payload, unsigned int length);

//     NFCResponse lastResponse;

// private:
//     PubSubClient& _mqttClient;
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
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <functional>

// Define NFCHandler type
typedef std::function<void(JsonDocument&)> NFCHandler;

struct NFCAccessInfo {
    bool Granted;
    String User_ID;
    String User_Name;
    String Role;
    int Reason;
};

struct NFCResponse {
    String Message_ID;
    String NFC_UID;
    String Status;
    NFCAccessInfo Access;
};

class NFCMQTT {
public:
    NFCMQTT(PubSubClient& client);
    
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
    
    PubSubClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    const char* _deviceId;
    
    // Store pending UID
    String _pendingUid = "";
    
    // Callback pointers
    void (*closePopupCallback)() = NULL;
    void (*navigateToItemCallback)(const char*, const char*, const char*) = NULL;
};

#endif