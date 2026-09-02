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
void SD_Manager::listBidFiles(String* files, int maxFiles, int& fileCount)
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
                files[fileCount++] = name;
            }
        }
        entry.close();
    }
    root.close();
}

// Read all records from selected bid file
void SD_Manager::readBidFile(const String& filename, String* records, int maxRecords, int& recordCount)
{
    recordCount = 0;

    if (!_sdReady) return;

    if (!SD.exists(filename)) {
        Serial.println("File not found: " + filename);
        return;
    }

    File file = SD.open(filename);

    if (!file) {
        Serial.println("Failed to open: " + filename);
        return;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        if (recordCount < maxRecords) {
            records[recordCount++] = line;
        } else {
            break;
        }
    }

    file.close();
}

// bool SD_Manager::updateBidStatus(const char* auctionId,
//                                  const char* itemId,
//                                  const char* newStatus)
// {
//     if (!_sdReady) return false;

//     // Open today's CSV
//     struct tm timeinfo;
//     if (!getLocalTime(&timeinfo)) return false;

//     char filename[40];
//     snprintf(filename, sizeof(filename), "/bids_%04d-%02d-%02d.csv",
//              timeinfo.tm_year + 1900,
//              timeinfo.tm_mon + 1,
//              timeinfo.tm_mday);

//     if (!SD.exists(filename)) return false;

//     File file = SD.open(filename, FILE_READ);
//     File temp = SD.open("/temp.csv", FILE_WRITE);
//     if (!file || !temp) return false;

//     while (file.available()) {
//         String line = file.readStringUntil('\n');
//         line.trim();
//         if (line.length() == 0) continue;

//         // Match auctionId & itemId in the CSV line
//         if (line.startsWith(String(auctionId) + "," + String(itemId) + ",")) {
//             int lastComma = line.lastIndexOf(',');
//             if (lastComma > 0) {
//                 line = line.substring(0, lastComma + 1) + String(newStatus);
//             }
//         }

//         temp.println(line);
//     }

//     file.close();
//     temp.close();

//     SD.remove(filename);
//     SD.rename("/temp.csv", filename);

//     saveLog(("Bid updated: " + String(auctionId) + "," + String(itemId) + " -> " + String(newStatus)).c_str());

//     return true;
// }