// #ifndef SD_MANAGER_H
// #define SD_MANAGER_H

// #include <Arduino.h>
// #include <SPI.h>
// #include <SD.h>

// // ================= CONFIG STRUCT =================
// class SD_Manager {
// public:
//     SD_Manager(int sclk, int miso, int mosi, int cs, int powerPin);

//     bool begin();
//     void writeTestFile();
//     void readTestFile();
//     bool isReady();
//     bool saveBidRecord(const char* auctionId, const char* itemId, const char* userId, const char* nfcUid, float bidValue, const char* status);

// private:
//     int _sclk, _miso, _mosi, _cs, _powerPin;
//     bool _sdReady;
// };

// #endif


#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>

class SD_Manager
{
public:
    SD_Manager(int sclk, int miso, int mosi, int cs, int powerPin);

    bool begin();
    bool isReady();

    void writeTestFile();
    void readTestFile();
    bool saveLog(const char* message);

    // Save bid record (auto-create daily file)
    bool saveBidRecord(const char* auctionId,
                       const char* itemId,
                       const char* userId,
                       const char* nfcUid,
                       float bidValue,
                       const char* status);

    // ---------- New Functions for Bid Viewer ----------
    
    // Return list of bid files on SD
    void listBidFiles(char files[][64], int maxFiles, int& fileCount);

    // Read all lines of a specific bid file
    void readBidFile(const char* filename, char records[][256], int maxRecords, int& recordCount);

private:
    int _sclk;
    int _miso;
    int _mosi;
    int _cs;
    int _powerPin;

    bool _sdReady;

    void getTodayFileName(char* buffer, size_t maxLen);
};

#endif