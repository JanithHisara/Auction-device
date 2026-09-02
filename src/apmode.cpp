#include "apmode.h"
#include <Preferences.h>
#include <ArduinoJson.h>

APMode::APMode(const char* ssid, const char* password)
: _ssid(ssid), _password(password), server(80) {}

void APMode::begin() {
    Serial.println("Starting Access Point...");
    WiFi.softAP(_ssid, _password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP Address: ");
    Serial.println(IP);

    // Mount LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed!");
        return;
    }

    // DNS for captive portal
    dnsServer.start(DNS_PORT, "*", IP);

    // Root route
    server.on("/", [this]() {
        if (!serveFile("/index.html"))
            server.send(500, "text/plain", "index.html not found");
    });

    // Serve files (CSS/JS/images)
    server.onNotFound([this]() {
        if (!serveFile(server.uri())) {
            server.sendHeader("Location", "/");
            server.send(302, "text/plain", "");
        }
    });

    // Scan WiFi networks
    server.on("/scan", [this]() {
        int n = WiFi.scanNetworks();
        String json = "[";
        for (int i = 0; i < n; i++) {
            if (i) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        }
        json += "]";
        server.send(200, "application/json", json);
    });
    
    // Save config
    server.on("/save", HTTP_POST, [this]() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    String body = server.arg("plain");
    Serial.println("Received Config: " + body);

    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        Serial.println("JSON parse failed");
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    const char* ssid = doc["ssid"];
    const char* password = doc["password"];
    const char* deviceId = doc["deviceId"];

    // Validate that we got all fields
    if (!ssid || !password || !deviceId) {
        server.send(400, "text/plain", "Missing fields");
        return;
    }

    Serial.printf("SSID: %s\n", ssid);
    Serial.printf("Password: %s\n", password);
    Serial.printf("DeviceID: %s\n", deviceId);

    // Save to Preferences
    Preferences preferences;
    preferences.begin("wifi-config", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("deviceId", deviceId);
    preferences.end();

    Serial.println("Config saved successfully");
    
    // Send response
    server.send(200, "text/plain", "Saved. Rebooting...");
    
    // Flush serial
    Serial.flush();
    
    // Give time for response to be sent
    delay(500);
    
    // Restart ESP32
    ESP.restart();
});

    server.on("/ota", HTTP_POST, [this]() {
    Serial.println("OTA endpoint hit");

        if (server.hasArg("update")) { // For form submission
            Serial.println("Update request received");
            Serial.println("Value: " + server.arg("update"));
        }
        // ota update code here
        server.send(200, "text/plain", "OK");
    });



    server.begin();
    Serial.println("AP Mode with LittleFS + Captive Portal started.");
}


bool APMode::serveFile(String path) {
    if (path.endsWith("/")) path += "index.html";

    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    if (path.endsWith(".css"))  contentType = "text/css";
    if (path.endsWith(".js"))   contentType = "application/javascript";
    if (path.endsWith(".png"))  contentType = "image/png";
    if (path.endsWith(".jpg"))  contentType = "image/jpeg";
    if (path.endsWith(".ico"))  contentType = "image/x-icon";

    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}


void APMode::handle() {
    dnsServer.processNextRequest();
    server.handleClient();
}
