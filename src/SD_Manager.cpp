#include "SD_Manager.h"

SD_Manager::SD_Manager(int sclk, int miso, int mosi, int cs, int powerPin)
{
    _sclk = sclk;
    _miso = miso;
    _mosi = mosi;
    _cs = cs;
    _powerPin = powerPin;
    _sdReady = false;
}

bool SD_Manager::begin()
{
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, HIGH);
    delay(300);

    SPI.begin(_sclk, _miso, _mosi);

    if (!SD.begin(_cs)) {
        Serial.println("❌ SD Mount Failed");
        return false;
    }

    _sdReady = true;

    Serial.println("✅ SD Mounted");

    uint64_t size = SD.cardSize() / (1024 * 1024);
    Serial.print("Size: ");
    Serial.print(size);
    Serial.println(" MB");

    return true;
}

void SD_Manager::writeTestFile()
{
    if (!_sdReady) return;

    File file = SD.open("/test.txt", FILE_WRITE);

    if (!file) {
        Serial.println("❌ Write open failed");
        return;
    }

    file.println("ESP32 + LVGL SD Test OK");
    file.println("Auction terminal storage test");

    file.close();
    Serial.println("✅ File written");
}

void SD_Manager::readTestFile()
{
    if (!_sdReady) return;

    File file = SD.open("/test.txt");

    if (!file) {
        Serial.println("❌ Read open failed");
        return;
    }

    Serial.println("Reading file:");

    while (file.available()) {
        Serial.write(file.read());
    }

    Serial.println("\n--- END ---");
    file.close();
}

bool SD_Manager::isReady()
{
    return _sdReady;
}

bool SD_Manager::saveBidRecord(const char* auctionId,
                               const char* itemId,
                               const char* userId,
                               const char* nfcUid,
                               float bidValue,
                               const char* status)
{
    if (!_sdReady) {
        Serial.println("SD not ready");
        return false;
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get time");
        return false;
    }

    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Generate daily filename
    char filename[40];
    snprintf(filename, sizeof(filename), "/bids_%04d-%02d-%02d.csv",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday);

    bool newFile = !SD.exists(filename);
    File file = SD.open(filename, FILE_APPEND);

    if (!file) {
        Serial.println("❌ Failed to open " + String(filename));
        return false;
    }

    // Write CSV header if new file
    if (newFile) {
        file.println("timestamp,auction_id,item_id,user_id,nfc_uid,bid_value,status");
    }

    file.print(timestamp); file.print(",");
    file.print(auctionId); file.print(",");
    file.print(itemId); file.print(",");
    file.print(userId); file.print(",");
    file.print(nfcUid); file.print(",");
    file.print(bidValue); file.print(",");
    file.println(status);

    file.close();

    Serial.println("✅ Bid saved to " + String(filename));
    return true;
}

// List all bid files like "bids_YYYY-MM-DD.csv"
void SD_Manager::listBidFiles(char files[][64], int maxFiles, int& fileCount)
{
    fileCount = 0;

    if (!_sdReady) return;

    File root = SD.open("/");

    if (!root) {
        Serial.println("Failed to open root");
        return;
    }

    File entry;
    while ((entry = root.openNextFile())) {
        String name = entry.name();

        if (!entry.isDirectory() && name.startsWith("bids_") && name.endsWith(".csv")) {
            if (fileCount < maxFiles) {
                strlcpy(files[fileCount++], name.c_str(), 64);
            }
        }
        entry.close();
    }
    root.close();
}

// Read all records from selected bid file
void SD_Manager::readBidFile(const char* filename, char records[][256], int maxRecords, int& recordCount)
{
    recordCount = 0;

    if (!_sdReady) return;

    if (!SD.exists(filename)) {
        Serial.printf("File not found: %s\n", filename);
        return;
    }

    File file = SD.open(filename);

    if (!file) {
        Serial.printf("Failed to open: %s\n", filename);
        return;
    }

    while (file.available()) {
        char line[256];
        int i = 0;
        while (file.available() && i < 255) {
            char c = file.read();
            if (c == '\n') break;
            line[i++] = c;
        }
        line[i] = '\0';
        // line.trim() replaced by manual strip if necessary, or assume csv is clean
        if (strlen(line) == 0) continue;

        if (recordCount < maxRecords) {
            strlcpy(records[recordCount++], line, 256);
        } else {
            break;
        }
    }

    file.close();
}

