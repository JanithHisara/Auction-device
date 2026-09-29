#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <ArduinoJson.h>
extern void show_custom_loading(const char* message);
extern void hide_custom_loading();

// Define the current firmware version of the device
// CHANGE THIS TO 1.1 BEFORE COMPILING THE NEW FIRMWARE
#define CURRENT_FIRMWARE_VERSION 2.4

// URL to the version.json file on GitHub
// Example JSON file contents: {"version": 1.1, "url": "https://raw.githubusercontent.com/JanithHisara/Auction-update-repo/main/firmware.bin"}
const char* versionUrl = "https://raw.githubusercontent.com/JanithHisara/Auction-update-repo/main/version.json";

void performOTAUpdate(const char* binUrl) {
    WiFiClientSecure client;
    client.setInsecure(); // Do not verify SSL certificate for simplicity
    client.setTimeout(15000); // 15-second timeout for large downloads
    
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(15000);
    http.begin(client, binUrl);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        bool canBegin = Update.begin(contentLength);
        if (canBegin) {
            Serial.println("Begin OTA. This may take 1-2 minutes...");
            
            // Show updating UI
            show_custom_loading("Updating Device\nPlease Wait...");
            
            // Custom highly resilient download loop
            size_t written = 0;
            uint8_t buff[1024] = { 0 };
            WiFiClient *stream = http.getStreamPtr();
            uint32_t timeout = millis();
            bool writeError = false;
            
            while (http.connected() && (written < contentLength)) {
                size_t size = stream->available();
                if (size) {
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    size_t written_this_time = Update.write(buff, c);
                    if (written_this_time != c) {
                        Serial.println("Error: Flash write failed!");
                        writeError = true;
                        break;
                    }
                    written += c;
                    timeout = millis(); // Reset timeout on successful read
                } else {
                    if (millis() - timeout > 15000) {
                        Serial.println("Error: Network timeout during download!");
                        break;
                    }
                    delay(10); // Wait for more data to arrive
                }
            }

            if (written == contentLength && !writeError) {
                Serial.println("Written : " + String(written) + " successfully");
                if (Update.end()) {
                    Serial.println("OTA done!");
                    if (Update.isFinished()) {
                        Serial.println("Update successfully completed. Rebooting.");
                        show_custom_loading("Success!\nRebooting...");
                        delay(2000);
                        ESP.restart();
                    } else {
                        Serial.println("Update not finished? Something went wrong!");
                    }
                } else {
                    Serial.println("Error Occurred. Error #: " + String(Update.getError()));
                }
            } else {
                Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
            }
        } else {
            Serial.println("Not enough space to begin OTA");
        }
    } else {
        Serial.println("Cannot download firmware! HTTP code: " + String(httpCode));
    }
    http.end();
}

void checkGitHubForUpdates() {
    Serial.println("Checking GitHub for firmware updates...");
    show_custom_loading("Checking Updates...");
    
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10000);
    
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000);
    http.begin(client, versionUrl);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Remove UTF-8 BOM if present (GitHub raw text or Windows files often have this!)
        if (payload.startsWith("\xEF\xBB\xBF")) {
            payload = payload.substring(3);
        }
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            float latestVersion = doc["version"];
            String binUrl = doc["url"].as<String>();
            
            // Add a small epsilon to prevent floating point precision bugs (e.g. 1.300001 > 1.300000)
            if (latestVersion > (CURRENT_FIRMWARE_VERSION + 0.05)) {
                Serial.printf("New firmware found! Current: %.1f, Latest: %.1f\n", CURRENT_FIRMWARE_VERSION, latestVersion);
                
                // Show prominent update screen BEFORE releasing HTTP resources
                show_custom_loading("\xEF\x9B\x9A Device Updating...\nPlease Wait\nDo Not Power Off");

                // CRITICAL: We MUST free the HTTP client and SSL context before starting the OTA download!
                // Otherwise the ESP32 runs out of RAM for the second SSL connection and fails with HTTP -11!
                http.end();
                client.stop();
                
                performOTAUpdate(binUrl.c_str());
            } else {
                Serial.println("Device is up to date.");
                // Clear the loading screen so it can proceed
                hide_custom_loading();
            }
        } else {
            Serial.println("Failed to parse version JSON.");
            hide_custom_loading();
        }
    } else {
        Serial.println("Failed to check for updates. HTTP Code: " + String(httpCode));
        hide_custom_loading();
    }
    
    http.end();
}

#endif













