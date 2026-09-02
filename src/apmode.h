#ifndef APMODE_H
#define APMODE_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

#define DNS_PORT 53

class APMode {
public:
    APMode(const char* ssid, const char* password);
    void begin();
    void handle();

private:
    void handleRoot();
    bool serveFile(String path);

    const char* _ssid;
    const char* _password;

    WebServer server;
    DNSServer dnsServer;
};

#endif
