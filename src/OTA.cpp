// #include "OTA.h"

// bool OTAUpdater::downloadAndUpdate(const char* url, int updateType) {

//   HTTPClient http;
//   http.begin(url);

//   Serial.printf("\n[OTA] Connecting to: %s\n", url);
//   int httpCode = http.GET();

//   if (httpCode != HTTP_CODE_OK) {
//     Serial.printf("[OTA] HTTP Error: %d\n", httpCode);
//     http.end();
//     return false;
//   }

//   int contentLength = http.getSize();
//   WiFiClient * stream = http.getStreamPtr();

//   Serial.printf("[OTA] File Size = %d bytes\n", contentLength);

//   if (contentLength <= 0) {
//     Serial.println("[OTA] ERROR: Content length is 0!");
//     http.end();
//     return false;
//   }

//   // Begin update
//   if (!Update.begin(contentLength, updateType)) {
//     Serial.printf("[OTA] OTA Begin Error: %s\n", Update.errorString());
//     http.end();
//     return false;
//   }

//   Serial.println("[OTA] Downloading...");

//   uint8_t buf[2048];
//   int written = 0;

//   while (http.connected() && written < contentLength) {
//     size_t avail = stream->available();
//     if (avail) {
//       int readBytes = stream->readBytes(buf, min((int)avail, (int)sizeof(buf)));
//       Update.write(buf, readBytes);
//       written += readBytes;

//       int percent = (written * 100) / contentLength;
//       Serial.printf("Progress: %d%% (%d/%d bytes)\r", percent, written, contentLength);
//     }
//     delay(1);
//   }

//   Serial.println();

//   if (!Update.end(true)) {
//     Serial.printf("[OTA] OTA End Error: %s\n", Update.errorString());
//     http.end();
//     return false;
//   }

//   if (!Update.isFinished()) {
//     Serial.println("[OTA] ERROR: Update not finished!");
//     http.end();
//     return false;
//   }

//   Serial.println("[OTA] Update Successful!");

//   http.end();
//   return true;
// }

// bool OTAUpdater::updateFirmware() {
//   Serial.println("\n=== Updating FIRMWARE ===");
//   return downloadAndUpdate(firmware_url, U_FLASH);
// }

// bool OTAUpdater::updateLittleFS() {
//   Serial.println("\n=== Updating LittleFS ===");
//   return downloadAndUpdate(littlefs_url, U_SPIFFS);
// }

// void OTAUpdater::performOTA() {

//   Serial.println("Checking for OTA updates...\n");

//   // Step 1 → Update Firmware
//   if (!updateFirmware()) {
//     Serial.println("❌ Firmware update FAILED!");
//     return;
//   }

//   // Step 2 → Update LittleFS
//   if (!updateLittleFS()) {
//     Serial.println("❌ LittleFS update FAILED!");
//     return;
//   }

//   Serial.println("\n✅ ALL OTA Updates Completed Successfully!");
//   Serial.println("Rebooting in 2 seconds...");
//   delay(2000);
//   ESP.restart();
// }

