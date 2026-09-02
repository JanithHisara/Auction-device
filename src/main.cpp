#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <logo.h>
#include <MQTTClient.h>
#include <LVGLScreen.h>
#include <tft_init.h>
#include <Header_template.h>
#include <SD_Manager.h>
#include <Hardware.h>
#include <Preferences.h>
#include <apmode.h>
#include <Button.h>
#include <AuctionScreen.h>
#include <AuctionMQTT.h>
#include <NFCMQTT.h>
#include <ItemsMQTT.h>
#include <ItemScreen.h>
#include <BidMQTT.h>
#include <Spinner.h>
#include <vector>
#include <Secret.h>

// ------------------ PINS ------------------
#define Latch_ON 4
#define Latch_ISR 14
#define NFC_ON 12
#define buzzer 33
#define pin_LCD 2
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   21
#define TFT_RST  22


#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define BTN1 35   // SW20 (Confirm)
#define BTN2 34   // SW22 (AP Mode Only)
#define BTN3 36   // SW17 (Next Page)
#define BTN4 27   // SW18 (Previous Page)
#define BTN5 32   // SW21 (Confirm)
#define BTN_CANCEL 25 // SW19 (Cancel)

#define SDA_PIN 16
#define SCL_PIN 17
#define POWER_PIN 12
#define MAX17048_ADDR 0x36

#define MAX_FILES 20
#define MAX_RECORDS 100
//SD Card
#define SD_MISO 19
#define SD_CS   13
#define SD_BTN  15

#define LOGO_WIDTH 240
#define LOGO_HEIGHT 80

// ------------------ MQTT ------------------
String AUCTION_REQ_TOPIC = "";
String AUCTION_RES_TOPIC = "";
const char* awsEndpoint = "a1m322vfibs32e-ats.iot.ap-south-1.amazonaws.com";

// ------------------ GLOBALS ------------------
String deviceId = "";
String clientId;
String firmwareVersion = "1.0.0";
String hardwareVersion = "1.0";

Spinner spinner;
IPAddress ip;
WiFiClientSecure net;
MQTTClient mqttClient(4096);
// AP Mode
APMode apMode("AuctionHub", "12345678");
Preferences preferences;


// Keypad
const uint8_t rowPins[4] = {0,1,2,3};
const uint8_t colPins[4] = {4,5,6,7};
//const uint8_t ledPins[] = {8,9,10,11,12,13,14,15}; // MCP pins for LEDs
char keys[4][4] = {
  {'A','1','2','3'},
  {'B','4','5','6'},
  {'C','7','8','9'},
  {'D','*','0','#'}
};

// TFT + LVGL
TFT_init tft(TFT_MOSI, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST, 240, 320);
SD_Manager sd(TFT_SCLK, SD_MISO, TFT_MOSI, SD_CS, SD_BTN);

// Hardware objects
Adafruit_MCP23X17 mcp;
KeypadManager keypad(&mcp, rowPins, colPins, keys);
NFCManager nfc(-1,-1);

// ------------------ NFC FORMATTER ------------------
String formatNfcUid(uint8_t* uid, uint8_t uidLen) {
    if (uidLen == 4) {
        uint32_t dec_uid = ((uint32_t)uid[3] << 24) | ((uint32_t)uid[2] << 16) | ((uint32_t)uid[1] << 8) | uid[0];
        return String(dec_uid);
    } else {
        String uidStr = formatNfcUid(uid, uidLen);
        return uidStr;
    }
}

BatteryManager battery(MAX17048_ADDR, 3.6, 4.2);
//LEDManager leds(&mcp, ledPins, 8);

// Buttons
Button btnOK2(BTN1);     // SW20 (Confirm)
Button btnAPMode(BTN2);  // SW22 (AP Mode Only)
Button btnRight(BTN4);   // SW17 (Next Page - Pin 36)
Button btnOK(BTN5);      // SW21 (Confirm)
Button btnLeft(BTN3);    // SW18 (Previous Page - Pin 27)
Button btnCancel(BTN_CANCEL); // SW19 (Cancel)

// MQTT Objects
AuctionMQTT auction(mqttClient);
NFCMQTT nfcMqtt(mqttClient);
ItemsMQTT items(mqttClient);
BidMQTT bid(mqttClient);

// ------------------ STATE ------------------
bool lastState = LOW;
volatile bool wifiConnected = false;
volatile bool mqttConnected = false;
bool auction_requested = false;
bool apModeActive = false;
bool auctionDataLoaded = false;
int auction_index = 0;   // for navigating auctions
int item_index = 0;      // for navigating items

// ------------------ TIMERS ------------------
unsigned long startupTime;
unsigned long lastBatteryUpdate = 0;
unsigned long lastTimeUpdate = 0;
unsigned long lastKeyTime = 0;
const unsigned long KEY_DEBOUNCE_MS = 200;
char lastKey = 0;
const unsigned long BATTERY_INTERVAL = 5000;
const unsigned long TIME_INTERVAL = 1000;
unsigned long lastItemsRequest = 0;
unsigned long lastAuctionRequest = 0;
const unsigned long ITEMS_INTERVAL = 3000; // 3 seconds for real-time live synchronization
const char* currentAuctionID = nullptr;
const char* currentAuctionStatus = nullptr;
const unsigned long keyDebounceDelay = 200; // ms
unsigned long lastNfcCheckTime = 0;
const unsigned long NFC_CHECK_INTERVAL = 3000; // 3 seconds
String expectedNfcUid = ""; // Store the expected UID for the current user
String pinInput = "";
const int PIN_LENGTH = 4;
String expectedPin = "4567";
int currentBox = 0;
bool pinValidated = false;

static lv_obj_t* pin_card = nullptr;
static lv_obj_t* pin_boxes[4] = {nullptr};
static lv_obj_t* pin_message = nullptr;
// ------------------ UI STATE MACHINE ------------------
enum UIState {
    UI_AP_MODE,
    UI_AUCTION,
    UI_PIN_INPUT,
    UI_WAITING_NFC,
    UI_LOADING_ITEMS,
    UI_ITEMS,
    UI_BID_WAIT_NFC
};
UIState currentUI = UI_AUCTION;

struct UserSession {
    String uid;
    String userId;
    String userName; 
    String role;
    bool granted;
    bool active;
};
struct ItemsBuffer {
    JsonArray itemsArrayCopy;
    char auctionID[32];
    char auctionMode[16];
};

ItemsBuffer itemsBuffer;
volatile bool items_ready_for_display = false;

enum NfcAuthState {
    NFC_IDLE,
    NFC_WAIT_FOR_CARD,
    NFC_WAIT_FOR_RESPONSE
};
UserSession currentUser;
NfcAuthState nfcState = NFC_IDLE;

enum BidUIState {
    BID_UI_FILE_LIST,
    BID_UI_RECORDS
};
BidUIState bidUIState = BID_UI_FILE_LIST;

String scannedUid = "";
unsigned long nfcStartTime = 0;
const unsigned long NFC_TIMEOUT = 10000; // 10s
String pendingNFCUid = "";

// Add these global variables for selected auction
String selectedAuctionId = "";  // Store the currently selected auction ID
String selectedAuctionMode = ""; // Store the auction mode
String selectedAuctionName = ""; // Store the auction name

// Add flag to track if item screen is initialized
bool item_screen_initialized = false;

// Add inactivity timer variable
unsigned long lastActivity = 0;
String localBidValue = "";

// ------------------ FUNCTION PROTOTYPES ------------------
void updateBattery();
void updateTime();
void startNTP();
bool connectMQTT();
void setupMQTTCallbacks();
void initMQTTHandlers();
void show_custom_loading_timeout(const char* message, uint32_t timeout_ms);
void show_warning_timeout(const char* message, uint32_t timeout_ms);
void hide_custom_loading();
void show_refresh_popup(const char* message);
void hide_refresh_popup();
void updateInputs();
void updateSystem();
void handleAuctionState();
void handleWaitingNFCState();
//void handleLoadingItemsState();
void checkNfcAvailabilityAndUid();
void handleItemsState();
void handleBidWaitNFCState();
void handleBidPopupInput();
void changeState(UIState newState);
void resetAuctionSelection();
const char* get_current_user_name();
void update_pin_display();
void show_pin_ui();
void hide_pin_ui();
void verify_and_proceed();
//void connectAWS();

// ------------------ FUNCTION IMPLEMENTATIONS ------------------
void sendNormalMessage(const char* msg) {
  if (mqttClient.publish(AUCTION_REQ_TOPIC.c_str(), msg, false, 1)) {
    Serial.print("Normal message sent: ");
    Serial.println(msg);
  } else {
    Serial.println("Failed to send normal message!");
  }
}
// Update battery percentage on display
bool lowBatteryWarningShown = false;
void updateBattery() {
    if (lastBatteryUpdate == 0 || millis() - lastBatteryUpdate > BATTERY_INTERVAL) {
        lastBatteryUpdate = millis();
        float soc = battery.readSOC();
        float voltage = battery.readVoltage();
        
        uint8_t pct = (uint8_t)battery.readPercent();
        set_battery_percent(pct);
        
        Serial.print("Battery Voltage: "); Serial.print(voltage);
        Serial.print("V, Raw SOC: "); Serial.print(soc);
        Serial.print("%, Displayed Pct: "); Serial.print(pct); Serial.println("%");

        if (soc <= 20.0 && voltage > 1.0) {
            Serial.println("Battery SOC is <= 20%! Shutting down...");
            show_custom_loading("Battery Low!\nPowering off...");
            lv_timer_handler();
            
            ledcSetup(0, 2000, 8);
            ledcAttachPin(buzzer, 0);
            ledcWriteTone(0, 1500);
            delay(60);
            ledcWriteTone(0, 800);
            delay(100);
            ledcWriteTone(0, 0);
            ledcDetachPin(buzzer);
            pinMode(buzzer, INPUT); 
            
            delay(2000); 
            digitalWrite(Latch_ON, LOW);
            while(true) delay(100);
        }

        if (soc < 30.0 && soc > 20.0) {
            set_battery_color(lv_color_hex(0xFF0000));
            if (!lowBatteryWarningShown && millis() > 5000) {
                show_warning_timeout("Low Battery!", 3000);
                lowBatteryWarningShown = true;
            }
        } else if (soc >= 30.0) {
            set_battery_color(lv_color_white());
            lowBatteryWarningShown = false;
        }
    }
}
void readNFC() {
    digitalWrite(NFC_ON,HIGH);
    if (nfcState != NFC_WAIT_FOR_CARD) return;

    static unsigned long lastNfcScan = 0;
    if (millis() - lastNfcScan < 200) return;
    lastNfcScan = millis();

    uint8_t uid[7];
    uint8_t uidLen;

    if (nfc.readUID(uid, &uidLen)) {

        String uidStr = formatNfcUid(uid, uidLen);

        Serial.println("Scanned UID: " + uidStr);

        // NFC FOR BID CONFIRMATION
        if (currentUI == UI_BID_WAIT_NFC) {
            if (!currentUser.granted || uidStr != currentUser.uid) {
                Serial.println("❌ NFC card does not match the registered auction participant!");
                show_custom_loading_timeout("Unauthorized Card!", 2000);
                nfcState = NFC_IDLE;
                currentUI = UI_ITEMS;
                return;
            }
            Serial.println("NFC verified for bid");
            // Store UID inside library
            set_nfc_authenticated(true,
                                currentUser.userId.c_str(),
                                currentUser.role.c_str());
            // Now show waiting response
            show_custom_loading("Submitting bid...");

            // NOW submit bid
            confirm_bid();
            nfcState = NFC_IDLE;
            currentUI = UI_ITEMS;
            return;
        }
        // NORMAL AUCTION AUTH FLOW - THIS IS WHERE NFC PUBLISH HAPPENS
        if (currentUI == UI_WAITING_NFC) {
            show_custom_loading("Verifying...");
            
            // ✅ PUBLISH NFC CHECK REQUEST HERE
            String msgId = "NFC_" + String(millis());
            // Store the UID for later use
            scannedUid = uidStr;            
            // Publish the NFC check request using your NFCMQTT library with selectedAuctionId
            if (nfcMqtt.checkAccess(uidStr.c_str(), selectedAuctionName.c_str(), msgId.c_str())) {
                Serial.println("✅ NFC check request published for UID: " + uidStr + " in Auction: " + selectedAuctionName);
                nfcState = NFC_WAIT_FOR_RESPONSE;
                nfcStartTime = millis();
            } else {
                Serial.println("❌ Failed to publish NFC check request");
                show_custom_loading("NFC Publish Failed");
                lv_timer_t* t = lv_timer_create([](lv_timer_t* timer){
                    if (currentUI == UI_WAITING_NFC) {
                        show_custom_loading("Scan NFC Card...");
                        nfcState = NFC_WAIT_FOR_CARD;
                        nfcStartTime = millis();
                    }
                    lv_timer_del(timer);
                }, 2000, nullptr);
                lv_timer_set_repeat_count(t, 1);
            }
        }
    }
}

// Update time on display
void startNTP() {
    configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("NTP started");
}

void updateTime() {
    if (millis() - lastTimeUpdate < TIME_INTERVAL) return;
    lastTimeUpdate = millis();

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char buf[20];
        snprintf(buf, sizeof(buf), "%02d:%02d %02d/%02d",
                 timeinfo.tm_hour, timeinfo.tm_min,
                 timeinfo.tm_mday, timeinfo.tm_mon + 1);
        set_date_time(buf);
    }
}
const char* get_current_user_name() {
    return currentUser.userName.c_str();
}
extern const char* get_current_user_name();

// Connect MQTT
bool connectMQTT() {

    if (!wifiConnected) return false;

    static bool awsConfigured = false;

    if (!awsConfigured) {

        net.setCACert(AWS_ROOT_CA);
        net.setCertificate(AWS_CERT_CRT);
        net.setPrivateKey(AWS_PRIVATE_KEY);

        net.setTimeout(15000);
        net.setHandshakeTimeout(15000);

        mqttClient.begin(awsEndpoint, 8883, net);
        // mqttClient.setBufferSize(4096); // Increased for auction messages
        //mqttClient.setKeepAlive(60); 
        

        awsConfigured = true;
    }

    clientId = "AuctionHub_" + WiFi.macAddress();

    Serial.print("Connecting to AWS MQTT as: ");
    Serial.println(clientId);

    if (mqttClient.connect(clientId.c_str())) {

        Serial.println("✅ AWS MQTT Connected");

        // Subscribe to device-specific topic
        mqttClient.subscribe(AUCTION_RES_TOPIC.c_str(), 1);
        Serial.println("Subscribed to specific: " + AUCTION_RES_TOPIC);
        
        // Always independently subscribe to the broadcast topic!
        mqttClient.subscribe("auction/broadcast", 1);
        Serial.println("Subscribed to global broadcast: auction/broadcast");
        
        if (true) {
            Serial.println("Subscribed to specific: " + AUCTION_RES_TOPIC);

            delay(100);
            
            // ✅ PUBLISH AUCTION REQUEST HERE - AFTER CONNECTION IS CONFIRMED
            Serial.println("Publishing initial GET_AUCTION request...");
            String msgId = "INIT_" + String(millis());
            if (auction.publishRequest("GET_AUCTION", msgId.c_str())) {
                Serial.println("✅ GET_AUCTION published successfully");
                auction_requested = true;
            } else {
                Serial.println("❌ Failed to publish GET_AUCTION");
            }
        }

        mqttConnected = true;
        return true;
    }

    Serial.print("❌ MQTT Failed rc=");
    Serial.println(mqttClient.lastError());

    mqttConnected = false;
    return false;
}

// ------------------ MQTT CALLBACK ------------------
void mqttCallback(MQTTClient *client, char topic[], char payload[], int length) {
    if (strcmp(topic, "auction/broadcast") == 0) {
        String msgId = "SYNC_" + String(millis());
        
        if (currentUI == UI_AUCTION) {
            // If in auction list menu, refresh the auctions
            auction.publishRequest("GET_AUCTION", msgId.c_str());
        } 
        else if ((currentUI == UI_ITEMS || currentUI == UI_BID_WAIT_NFC) && selectedAuctionId.length() > 0) {
            // If inside a specific auction, refresh only the items for that auction!
            items.publishRequest(selectedAuctionName.c_str(), msgId.c_str(), currentUser.uid.c_str());
        }
        
        return;
    }

    auction.handleMessage(topic, (byte*)payload, (unsigned int)length);
    nfcMqtt.handleMessage(topic, (byte*)payload, (unsigned int)length);
    items.handleMessage(topic, (byte*)payload, (unsigned int)length);
    bid.handleMessage(topic, (byte*)payload, (unsigned int)length); // Comment out bid handling for now
}

// ------------------ MQTT HANDLERS ------------------
void setupMQTTCallbacks() {
    auction.onAction("SYNC_DB", [](JsonDocument& doc){
        Serial.println(">>> DB Sync requested by broadcast! Requesting updated auctions...");
        show_refresh_popup("Database Syncing...");
        
        // Force the popup to stay on screen for 2 seconds so the user can see it!
        delay(2000); 
        
        String msgId = "SYNC_" + String(millis());
        auction.publishRequest("GET_AUCTION", msgId.c_str());
    });

    auction.onAction("GET_AUCTION", [](JsonDocument& doc){
        Serial.println("✅ Auctions received");       
                if (auction.lastResponse.Auctions.size() > 0) {
            update_auctions_from_mqtt(
                auction.lastResponse.Auctions.data(),
                auction.lastResponse.Auctions.size()
            ); 
            
            if (!auctionDataLoaded) {
                auctionDataLoaded = true;             
                hide_custom_loading();          
                hide_refresh_popup();
                show_auction_screen();
            } else {
                if (currentUI == UI_AUCTION) {
                    hide_refresh_popup();
                    refresh_display();
                }
            }
        } else {
            Serial.println("No auctions in response");
            clear_auction_data();
            auctionDataLoaded = false;
            show_custom_loading("No auctions available");
        }
    });

    nfcMqtt.onAction("CHECK_ACCESS", [](JsonDocument& doc){
        if (nfcState != NFC_WAIT_FOR_RESPONSE) return;

        if (nfcMqtt.lastResponse.Access.Granted) {
            Serial.println("Access Granted");
            Serial.println("User ID: " + nfcMqtt.lastResponse.Access.User_ID);
            Serial.println("User Name: " + nfcMqtt.lastResponse.Access.User_Name);
            Serial.println("Role: " + nfcMqtt.lastResponse.Access.Role);

            currentUser.uid     = nfcMqtt.lastResponse.NFC_UID;
            currentUser.userId  = nfcMqtt.lastResponse.Access.User_ID;
            currentUser.userName = nfcMqtt.lastResponse.Access.User_Name;
            currentUser.role    = nfcMqtt.lastResponse.Access.Role;
            currentUser.granted = true;
            currentUser.active  = true;
            

                                    if (selectedAuctionId.length() > 0) {
                String welcomeMsg = "Welcome " + currentUser.userName;
                show_custom_loading(welcomeMsg.c_str());
                currentUI = UI_LOADING_ITEMS; // Prevent button presses during the 1.5s delay
                
                lv_timer_t* t = lv_timer_create([](lv_timer_t* timer){
                    if (selectedAuctionId.length() > 0) {
                        Serial.println("Access granted - Loading items for auction: " + selectedAuctionId);
                        
                        hide_auction_screen();
                        show_item_screen();   // shows loading label
                        
                        String msgId = "GET_ITEMS_" + String(millis());
                        if (items.publishRequest(selectedAuctionName.c_str(), msgId.c_str())) {
                            Serial.println("o. GET_ITEMS request sent for auction: " + selectedAuctionId);
                        } else {
                            Serial.println("?O Failed to send GET_ITEMS request");
                            show_custom_loading_timeout("\uF071 Failed to load items", 2000);
                            currentUI = UI_AUCTION;
                        }
                    }
                    lv_timer_del(timer);
                }, 1500, nullptr);
                lv_timer_set_repeat_count(t, 1);
            }
        } else {
            Serial.println("Access Denied for UID: " + nfcMqtt.lastResponse.NFC_UID);
            if (currentUI == UI_WAITING_NFC) {
                show_custom_loading("\uF071 Access Denied");
                lv_timer_t* t = lv_timer_create([](lv_timer_t* timer){
                    if (currentUI == UI_WAITING_NFC) {
                        show_custom_loading("Scan NFC Card...");
                        nfcState = NFC_WAIT_FOR_CARD;
                        nfcStartTime = millis();
                    }
                    lv_timer_del(timer);
                }, 2000, nullptr);
                lv_timer_set_repeat_count(t, 1);
                return;
            } else {
                show_custom_loading_timeout("Access Denied", 2000);
                currentUI = UI_AUCTION;
            }
        }

        nfcState = NFC_IDLE;
    });

    items.onAction("GET_ITEMS", [](JsonDocument& doc){

        if (currentUI != UI_LOADING_ITEMS && currentUI != UI_ITEMS) {
            Serial.println("Ignoring items - UI not expecting them");
            return;
        }

        if (!doc["Items"].is<JsonArray>()) return;

        JsonArray itemsArray = doc["Items"].as<JsonArray>();

        // Debug: Print what we received
        Serial.print("📥 GET_ITEMS Response - Auction_ID: ");
        if (doc["Auction_Name"].is<const char*>()) {
            Serial.println(doc["Auction_Name"].as<const char*>());
        } else {
            Serial.println("NOT FOUND");
        }
        
        if (doc["Status"].is<const char*>()) {
            Serial.print("Status: ");
            Serial.println(doc["Status"].as<const char*>());
        }

        // Check if Auction_ID exists in response (with underscore)
        if (!doc["Auction_Name"].is<const char*>()) {
            Serial.println("ERROR: No Auction_ID in response");
            return;
        }

        const char* responseAuctionName = doc["Auction_Name"];

        // Validate auction ID match with the selected one
        Serial.print("Selected Auction ID: ");
        Serial.println(selectedAuctionId);
        Serial.print("Response Auction ID: ");
        Serial.println(responseAuctionName);
        
        if (selectedAuctionName != responseAuctionName) {
            Serial.println("Items for wrong auction pushed by AWS. Ignoring...");
            return;
        }
        auctionDataLoaded=true;
        Serial.println("Auction ID MATCH! Loading items...");

        String modeUpper = selectedAuctionMode;
        modeUpper.toUpperCase();
        bool isClosedBid = (modeUpper.indexOf("CLOSED") >= 0 || modeUpper.indexOf("TENDER") >= 0 || modeUpper.indexOf("FIXED") >= 0 || modeUpper.indexOf("SEALED") >= 0);

        set_auction_details(
            selectedAuctionName.c_str(),
            isClosedBid ? AUCTION_MODE_CLOSED_BID : AUCTION_MODE_ENGLISH
        );

        set_current_auction_id(selectedAuctionId.c_str());

        const char* auctionStatus = "";
        if (doc["Auction_Status"].is<const char*>()) {
            auctionStatus = doc["Auction_Status"].as<const char*>();
        }
        load_items_from_mqtt(itemsArray, selectedAuctionMode.c_str(), auctionStatus);
        hide_custom_loading(); // Clear any loading banner from refresh
        hide_refresh_popup();
        if (currentUI == UI_LOADING_ITEMS) {
            item_index = 0;
            current_index = item_index; // reset item navigation index only when entering auction for the first time
        } else {
            int loaded_count = itemsArray.size();
            if (current_index >= loaded_count) {
                current_index = (loaded_count > 0) ? (loaded_count - 1) : 0;
            }
            item_index = current_index;
        }
        currentUI = UI_ITEMS;

        Serial.println("Items loaded successfully");
    });
    bid.onAction("SUBMIT_BID", [](JsonDocument& doc){

        Serial.println("📥 Bid response received");

        const char* status = doc["Status"] | "FAILED";

        // Let library handle popup + UI updates
        handle_bid_response(
            status,
            doc["Bid_Status"] | "",
            doc["Current_Highest_Bid"] | 0.0,
            doc["Reason"] | 0
        );

        // Reset NFC after processing
        set_nfc_authenticated(false, "", "");
        localBidValue = "";
            if (strcmp(status, "SUCCESS") == 0) {
        hide_auction_screen();
        hide_item_screen();
        //show_auction_screen();
       // show_pin_ui();
        resetAuctionSelection();
        changeState(UI_WAITING_NFC);
    } else {
        hide_auction_screen();
        Serial.println("Bid failed, staying on items screen");
    }
    });
}
// Add this function in main.cpp (accessible globally)
const char* get_current_nfc_uid() {
    return currentUser.uid.c_str();
}
void initMQTTHandlers() {
    mqttClient.onMessageAdvanced(mqttCallback);
   // mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
   // mqttClient.setBufferSize(2048);

    auction.begin(AUCTION_REQ_TOPIC.c_str(), AUCTION_RES_TOPIC.c_str(), deviceId.c_str());
    nfcMqtt.begin(AUCTION_REQ_TOPIC.c_str(), AUCTION_RES_TOPIC.c_str(), deviceId.c_str());
    items.begin(AUCTION_REQ_TOPIC.c_str(), AUCTION_RES_TOPIC.c_str(), deviceId.c_str());
    bid.begin(AUCTION_REQ_TOPIC.c_str(), AUCTION_RES_TOPIC.c_str(), deviceId.c_str()); // Comment out bid

    setupMQTTCallbacks();
}
void show_ap_mode_message() {
    // Hide any existing loading indicators
    if(loading_label) {
        lv_label_set_text(loading_label, "AP Mode Activate");
        lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Hide main UI elements
    lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
    // Ensure item screen is hidden
    hide_item_screen();
}
void show_custom_loading_timeout(const char* message, uint32_t timeout_ms) {
    if(loading_label) {
        lv_label_set_text(loading_label, message);
        lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);

    lv_timer_t* t = lv_timer_create([](lv_timer_t* timer){
        if (loading_label) {
            lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        if (currentUI == UI_AUCTION) {
            show_auction_screen();
            refresh_display();
        } else if (currentUI == UI_WAITING_NFC) {
            if (loading_label) {
                lv_label_set_text(loading_label, "Scan NFC Card...");
                lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
            }
        } else if (currentUI == UI_ITEMS) {
            // item_main_card was never hidden by us, so just hiding loading_label reveals it!
            // Do NOT call show_item_screen() as it forces the "Loading Items..." state.
        }

        lv_timer_del(timer);
    }, timeout_ms, nullptr);

    lv_timer_set_repeat_count(t, 1);
}

static lv_obj_t* refresh_popup_box = nullptr;
static unsigned long refresh_popup_time = 0;

void show_warning_timeout(const char* message, uint32_t timeout_ms) {
    if(loading_label) {
        lv_obj_set_style_text_color(loading_label, lv_color_hex(0xFF0000), 0);
        lv_label_set_text(loading_label, message);
        lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);

    lv_timer_t* t = lv_timer_create([](lv_timer_t* timer){
        if (loading_label) {
            lv_obj_set_style_text_color(loading_label, lv_color_white(), 0);
            lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        if (currentUI == UI_AUCTION) {
            show_auction_screen();
            refresh_display();
        } else if (currentUI == UI_WAITING_NFC) {
            if (loading_label) {
                lv_label_set_text(loading_label, "Scan NFC Card...");
                lv_obj_clear_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
            }
        } else if (currentUI == UI_ITEMS) {
            // item_main_card was never hidden by us, so just hiding loading_label reveals it!
            // Do NOT call show_item_screen() as it forces the "Loading Items..." state.
        }

        lv_timer_del(timer);
    }, timeout_ms, nullptr);
    lv_timer_set_repeat_count(t, 1);
}

void show_refresh_popup(const char* message) {
    if (refresh_popup_box != nullptr) {
        lv_obj_del(refresh_popup_box);
        refresh_popup_box = nullptr;
    }
    refresh_popup_time = millis();
    
    // Create a small popup window in front of all screens on top layer
    refresh_popup_box = lv_obj_create(lv_layer_top());
    lv_obj_set_size(refresh_popup_box, 200, 70);
    lv_obj_center(refresh_popup_box);
    lv_obj_set_style_bg_color(refresh_popup_box, lv_color_hex(0x2B2B36), 0);
    lv_obj_set_style_border_color(refresh_popup_box, lv_color_hex(0x00A859), 0);
    lv_obj_set_style_border_width(refresh_popup_box, 2, 0);
    lv_obj_set_style_radius(refresh_popup_box, 10, 0);
    lv_obj_set_style_pad_all(refresh_popup_box, 10, 0);

    lv_obj_t* label = lv_label_create(refresh_popup_box);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label, message);
    lv_obj_center(label);

    lv_timer_handler();
}

void hide_refresh_popup() {
    if (refresh_popup_box != nullptr) {
        lv_obj_del(refresh_popup_box);
        refresh_popup_box = nullptr;
    }
}

void hide_custom_loading() {
    if(loading_label) {
        lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    }
}

// ------------------ NETWORK TASK (Core1) ------------------
//TaskHandle_t networkTaskHandle = NULL;
unsigned long lastWifiReconnectAttempt = 0;
unsigned long lastMqttReconnectAttempt = 0;
static bool ntpStarted = false;
void handleNetwork() {
    unsigned long now = millis();
    // --- Wi-Fi handling ---
    if (WiFi.status() != WL_CONNECTED) {
        if (wifiConnected) {
            wifiConnected = false;
            mqttConnected = false;
            set_wifi_connected(false);
            set_mqtt_connected(false);
        }

        if (now - lastWifiReconnectAttempt > 10000) {
            lastWifiReconnectAttempt = now;
            set_wifi_connected(false);
            set_mqtt_connected(false);
            Serial.println("Reconnecting WiFi...");
            WiFi.reconnect();
        }

        return; // Skip MQTT until Wi-Fi is connected
    }
    // Wi-Fi is connected
    if (!wifiConnected) {
        wifiConnected = true;
        set_wifi_connected(true);
        Serial.println("WiFi Connected");

        if (!ntpStarted) {
            startNTP();
            ntpStarted = true;
        }

        lastMqttReconnectAttempt = 0; // reset MQTT reconnect timer
    }

    // --- MQTT handling ---
    if (!mqttClient.connected()) {
        if (now - lastMqttReconnectAttempt > 5000) {
            lastMqttReconnectAttempt = now;
            Serial.println("Connecting MQTT...");
            mqttConnected = connectMQTT();
            set_mqtt_connected(mqttConnected);
        }
    } else {
        mqttConnected = true;
        set_mqtt_connected(true);
        mqttClient.loop(); // keep connection alive
    }
}

// Update handlePinState with better debouncing
void handlePinState() {
    static unsigned long lastKeyTime = 0;
    static char lastKeyProcessed = 0;
    char key = keypad.scan();
    
    // Handle keypad input
    if (key != '\0') {
        // Simple debouncing for keypad
        if (millis() - lastKeyTime < 300) {
            if (key == lastKeyProcessed) return;
        }
        
        lastKeyTime = millis();
        lastKeyProcessed = key;
        
        // Number input
        if (key >= '0' && key <= '9' && pinInput.length() < PIN_LENGTH) {
            pinInput += key;
            Serial.print("PIN Input: ");
            Serial.println(pinInput);
            
            if (currentBox < PIN_LENGTH - 1) {
                currentBox++;
            }
            update_pin_display();
            
            if (pin_message) {
                lv_label_set_text(pin_message, " ");
            }
        }
        // Backspace
        else if (key == '*' && pinInput.length() > 0) {
            pinInput.remove(pinInput.length() - 1);  
            
            if (currentBox > 0) {
                currentBox--;
            }  
            update_pin_display();     
            
            if (pin_message) {
                lv_label_set_text(pin_message, " ");
            }
        }
        // Clear all
        else if (key == 'C' || key == 'c') {
            pinInput = "";
            currentBox = 0;
            update_pin_display();
            Serial.println("PIN Cleared");
            
            if (pin_message) {
                lv_label_set_text(pin_message, " ");
            }
        }
        // Submit with # key removed (SW16 does nothing)
    }
    // Handle physical OK button (separate from keypad)
    static unsigned long lastOKTime = 0;
    if ((btnOK.pressed() || btnOK2.pressed()) && (millis() - lastOKTime > 500)) {
        lastOKTime = millis();
        Serial.println("OK button pressed in PIN state");
        
        if (pinInput.length() == PIN_LENGTH) {
            Serial.println("PIN complete, verifying...");
            verify_and_proceed();
        } else if (pin_message) {
            lv_label_set_text(pin_message, "Enter all 4 digits!");
            lv_obj_set_style_text_color(pin_message, lv_color_hex(0xE57373), 0);
        }
    }
    // Cancel with navigation buttons
    static unsigned long lastNavTime = 0;
    if ((btnCancel.pressed() || btnLeft.pressed() || btnRight.pressed()) && 
        (millis() - lastNavTime > 500)) {
        lastNavTime = millis();
        Serial.println("Cancel PIN");
        hide_pin_ui();
        changeState(UI_AUCTION);
        show_custom_loading_timeout("Cancelled", 1000);
    }
}

TaskHandle_t PowerTask;
void powerMonitorTask(void * parameter) {
    unsigned long pressStartTime = 0;
    while (true) {
        if (millis() > 1000) { // Give it 1 second to boot before checking
            if (digitalRead(Latch_ISR) == HIGH) {
                if (pressStartTime == 0) {
                    pressStartTime = millis();
                } else if (millis() - pressStartTime >= 3000) {
                                        Serial.println("3s long press detected (Parallel Task): Shutting down device...");
                    // Brief descending shutdown tone
                    ledcSetup(0, 2000, 8);
                    ledcAttachPin(buzzer, 0);
                    ledcWriteTone(0, 1500);
                    delay(60);
                    ledcWriteTone(0, 800);
                    delay(100);
                    ledcWriteTone(0, 0);
                    ledcDetachPin(buzzer);
                    pinMode(buzzer, INPUT); // Float buzzer
                    
                    digitalWrite(Latch_ON, LOW);
                    while(true) delay(100); // Block until power dies
                }
            } else {
                pressStartTime = 0;
            }
        }
        delay(50); // Yield to watchdogs
    }
}

void setup() {
    pinMode(Latch_ON,OUTPUT);digitalWrite(Latch_ON,HIGH);
    pinMode(Latch_ISR, INPUT); 
    
    // Start the parallel power monitoring task on Core 0 immediately
    xTaskCreatePinnedToCore(powerMonitorTask, "PowerTask", 2048, NULL, 1, &PowerTask, 0);
    pinMode(NFC_ON,OUTPUT);digitalWrite(NFC_ON,LOW);
    pinMode(pin_LCD, OUTPUT);digitalWrite(pin_LCD, HIGH);
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Auction Hub Starting ===");

    // Initialize WiFi to read the MAC address and set dynamic topics
    WiFi.mode(WIFI_STA);
    String mac = WiFi.macAddress();
    Serial.printf("\nDEVICE_ID:%s\n", mac.c_str());

    // Dynamically set device ID and MQTT topics
    deviceId = mac;
    AUCTION_REQ_TOPIC = "auction/" + mac + "/request";
    AUCTION_RES_TOPIC = "auction/" + mac + "/response";

    // Hardware init
    pinMode(POWER_PIN, OUTPUT); 
    digitalWrite(POWER_PIN, HIGH);
    Wire.begin(SDA_PIN, SCL_PIN);

    tft.begin();
    lvgl_init();
    tft.fillScreen(0x0000);

    // Show logo & play startup melody via PWM buzzer
    uint16_t x = (240 - 240) / 2;
    uint16_t y = (320 - 80) / 2;
    tft.drawImage(x, y, LOGO_WIDTH, LOGO_HEIGHT, myImage);

    // Play short startup beep
    int melody[] = { 523, 659, 784 }; // C5, E5, G5
    int noteDurations[] = { 12, 12, 8 };
    
    ledcSetup(0, 2000, 8);
    ledcAttachPin(buzzer, 0);
    
    for (int i = 0; i < 3; i++) {
        int noteDuration = 1000 / noteDurations[i];
        ledcWriteTone(0, melody[i]);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        ledcWriteTone(0, 0); // Brief pause between notes
    }
    
    ledcWrite(0, 0);       
    ledcDetachPin(buzzer);
    pinMode(buzzer, OUTPUT);
    digitalWrite(buzzer, LOW);
    delay(100);

    // Remaining delay to complete ~2.8s logo display
    delay(2500);

    if (!mcp.begin_I2C(0x20)) Serial.println("MCP not found!");
    keypad.begin(); 
    nfc.begin(); 
    battery.begin(); 
   // leds.begin();
    btnOK2.begin(); 
    btnAPMode.begin(); 
    btnRight.begin(); 
    btnLeft.begin(); 
    btnOK.begin();
    btnCancel.begin();

    // UI
    create_topbar(240, 320);
    set_wifi_connected(false);
    set_mqtt_connected(false);
    create_auction_display(content_area);
    if (content_area) {
        init_item_screen(content_area);
        hide_item_screen();
    }
    //sd.begin();
    // Check button hold for AP mode at boot (SW22 / BTN2)
    pinMode(BTN2, INPUT);
    unsigned long start = millis();
    while (millis() - start < 3000) {
        if (digitalRead(BTN2) == LOW) {
            apModeActive = true;
            break;
        }
        delay(10);
    }

    // Wi-Fi in normal mode
    if (!apModeActive) {
        preferences.begin("wifi-config", true);
        String ssid = preferences.getString("ssid", "");
        String pass = preferences.getString("password", "");
        preferences.end();

        if (ssid.length() > 0) {
            WiFi.begin(ssid.c_str(), pass.c_str());
            Serial.println("Connecting to WiFi: " + ssid);
        }
    }else{
        hide_auction_screen();
        hide_item_screen();
        hide_custom_loading();
        apMode.begin();
        show_ap_mode_message();
        currentUI = UI_AP_MODE;
    }
    //leds.begin();
   // leds.set(0, true);   // LED on pin 8
   // leds.set(1, true);   // LED on pin 9
  //  leds.set(2, true);   // LED on pin 11
   // leds.set(3, true);   // LED on pin 8
    //leds.set(4, true);   // LED on pin 9
    //leds.set(5, true);   // LED on pin 10
    initMQTTHandlers();
    Serial.println("System Ready");
    startupTime = millis();
}

// ------------------ LOOP ------------------
void loop() {
    
    lv_timer_handler();
    if (refresh_popup_box != nullptr && (millis() - refresh_popup_time > 1500)) {
        hide_refresh_popup();
    }
    if (apModeActive) {
        apMode.handle();
        currentUI = UI_AP_MODE;
        // Keep AP mode message visible, not loading
        if (loading_label && lv_obj_has_flag(loading_label, LV_OBJ_FLAG_HIDDEN)) {
            show_ap_mode_message();
        }
        delay(5);
        return;
    } 
    handleNetwork();  
    
    

    updateInputs();      // Buttons + keypad + NFC

    // Runtime AP mode switching disabled - button is now useless after startup per user request

    updateSystem();      // Battery + Time
    switch (currentUI) {
        case UI_AP_MODE:
            break;
        case UI_AUCTION:
            handleAuctionState();
            break;

        case UI_PIN_INPUT:
            handlePinState();
            break;

        case UI_WAITING_NFC:
            handleWaitingNFCState();
            break;

        case UI_LOADING_ITEMS:
            // handleLoadingItemsState();
            break;

        case UI_ITEMS:
            handleItemsState();
            break;

        case UI_BID_WAIT_NFC:
            handleBidWaitNFCState();
            break;
    }

    delay(5);
}

void updateInputs() {
    btnOK2.update();
    btnAPMode.update();
    btnRight.update();
    btnLeft.update();
    btnOK.update();
    btnCancel.update();
        // NFC only when needed
    if (currentUI == UI_WAITING_NFC ||
        currentUI == UI_BID_WAIT_NFC) {
        readNFC();
    }
    // Keypad only when popup active
    // if (currentUI == UI_ITEMS &&
    //     is_bid_popup_active()) {
    //     readKeypad();
    // }
}
void updateSystem() {
    updateBattery();
    updateTime();
}
void handleAuctionState() {
    // SW16 (# button) acts as Refresh for Auctions List
    static unsigned long lastRefreshTime = 0;
    char key = keypad.scan();
    if (key == '#' && (millis() - lastRefreshTime > 1000)) {
        lastRefreshTime = millis();
        Serial.println("🔄 SW16 (#) pressed: Refreshing auction list...");
        show_refresh_popup("Refreshing...");
        String msgId = "REFRESH_" + String(millis());
        if (mqttConnected) {
            auction.publishRequest("GET_AUCTION", msgId.c_str());
            Serial.println("✅ GET_AUCTION refresh request published");
        } else {
            Serial.println("⚠️ MQTT not connected, cannot publish refresh request");
            show_refresh_popup("MQTT Offline");
        }
        return;
    }

    if (!auctionDataLoaded) {
        return;   // Nothing works
    }
    if (btnRight.pressed()) {
        next_auction();
    }
    if (btnLeft.pressed()) {
        prev_auction();
    }
    if (btnOK.pressed() || btnOK2.pressed()) {
        const char* status = auction_list[current_index].status;
        
        if (strcmp(status, "LIVE") != 0) {
            show_custom_loading_timeout("Auction not live", 2000);
            return;
        }
        
        // ✅ STORE AUCTION DETAILS BEFORE SHOWING PIN
        selectedAuctionId = auction_list[current_index].id;
        selectedAuctionMode = auction_list[current_index].mode;
        selectedAuctionName = auction_list[current_index].name;
        
        Serial.print("Selected Auction: ");
        Serial.println(selectedAuctionId);
        
        show_pin_ui();
        changeState(UI_PIN_INPUT);

        // const char* status = auction_list[current_index].status;

        // if (strcmp(status, "LIVE") != 0) {
        //     show_custom_loading_timeout("Auction not live", 2000);
        //     return;
        // }

        // selectedAuctionId   = auction_list[current_index].id;
        // selectedAuctionMode = auction_list[current_index].mode;
        // selectedAuctionName = auction_list[current_index].name;

        // show_custom_loading("Scan NFC Card...");
        // //sendNormalMessage("Scan NFC Card");

        // nfcState = NFC_WAIT_FOR_CARD;
        // nfcStartTime = millis();

        // changeState(UI_WAITING_NFC);
    }
}
void handleWaitingNFCState() {
    static bool messageShown = false;
    // Show message only once when entering this state
    if (!messageShown && currentUI == UI_WAITING_NFC) {
        show_custom_loading("Scan NFC Card...");
        messageShown = true;
        nfcState = NFC_WAIT_FOR_CARD;
        nfcStartTime = millis();
    }
    
    // Do not timeout the NFC scanning screen - wait indefinitely for a card.
    // Buttons (cancel/left/right) are ignored in this state per user request.

    // Do not timeout the NFC scanning screen - wait indefinitely for a card.
    // The screen will continuously display "Scan NFC Card..." or "Access Denied"
    // until the user either scans a card or presses cancel.
}

void handleItemsState() {
    checkNfcAvailabilityAndUid();
    //sendNormalMessage("Display Items");
    if (btnLeft.pressed())                     handle_item_buttons(0);     
    if (btnRight.pressed())                    handle_item_buttons(1);
    if (btnOK.pressed() || btnOK2.pressed())   handle_item_buttons(2);
    if (btnCancel.pressed() && is_bid_popup_active()) cancel_bid();

    if (is_bid_popup_active()) {
        handleBidPopupInput();
        return;
    }

    // SW16 (# button) acts as Refresh for Items List (when bid popup is closed)
    static unsigned long lastItemRefreshTime = 0;
    char key = keypad.scan();
    if (key == '#' && (millis() - lastItemRefreshTime > 1000)) {
        lastItemRefreshTime = millis();
        Serial.println("🔄 SW16 (#) pressed: Refreshing items list...");
        show_refresh_popup("Refreshing...");
        String msgId = "GET_ITEMS_" + String(millis());
        if (mqttConnected && selectedAuctionId.length() > 0) {
            items.publishRequest(selectedAuctionName.c_str(), msgId.c_str(), currentUser.uid.c_str());
            Serial.println("✅ GET_ITEMS refresh request published");
        } else {
            Serial.println("⚠️ Cannot refresh items: MQTT disconnected or no auction selected");
            show_refresh_popup("Refresh Failed");
        }
    }
}
void handleBidWaitNFCState() {
    if (millis() - nfcStartTime > NFC_TIMEOUT) {
        show_custom_loading_timeout("NFC Timeout", 2000);
        changeState(UI_ITEMS);
        nfcState = NFC_IDLE;
    }
}
void handleBidPopupInput() {
    static unsigned long lastKeyTime = 0;
    const uint16_t keyDebounceDelay = 180;
    char key = keypad.scan();
    if (key == '\0') return;
    if (millis() - lastKeyTime < keyDebounceDelay) return;
    lastKeyTime = millis();
    Serial.print("Key: ");
    Serial.println(key);

    if (key >= '0' && key <= '9') {
        char txt[2] = {key, '\0'};
        add_to_bid_textarea(txt);
        localBidValue += key;
    }else if (key == '*') {
        if (localBidValue.length() > 0)
            localBidValue.remove(localBidValue.length() - 1);
        backspace_bid_textarea();
    }else if (key == 'C') {
        localBidValue = "";
        clear_bid_textarea();
    }
    // # key function removed (SW16 does nothing)
    if (btnCancel.pressed() || btnLeft.pressed()) {
        localBidValue = "";
        cancel_bid();
    }
}
void changeState(UIState newState) {
    if (currentUI == newState) return;
    Serial.print("UI State: ");
    Serial.print(currentUI);
    Serial.print(" -> ");
    Serial.println(newState);

    // --- AGGRESSIVE UI CLEANUP ---
    if (newState == UI_AUCTION) {
        hide_item_screen();
        hide_pin_ui();
        hide_custom_loading();
        hide_refresh_popup();
    } else if (newState == UI_WAITING_NFC) {
        hide_item_screen();
        hide_auction_screen();
        hide_pin_ui();
        hide_refresh_popup();
    } else if (newState == UI_ITEMS) {
        hide_auction_screen();
        hide_pin_ui();
        hide_custom_loading();
    }
    // -----------------------------

    currentUI = newState;
    if (newState == UI_WAITING_NFC) {
        show_custom_loading("Scan NFC Card...");
        nfcState = NFC_WAIT_FOR_CARD;
        nfcStartTime = millis();
    } else if (newState == UI_AUCTION) {
        show_auction_screen();
        refresh_display();
    }
}
void resetAuctionSelection() {
    //selectedAuctionId = "";
    //selectedAuctionMode = "";
    //selectedAuctionName = "";
    nfcState = NFC_IDLE;
}
void checkNfcAvailabilityAndUid() {
    if (!currentUser.active) return;
    if (millis() - lastNfcCheckTime < NFC_CHECK_INTERVAL) return;
    lastNfcCheckTime = millis();
    // Get current UID from NFC reader
    uint8_t uid[7];
    uint8_t uidLen;
    
    if (nfc.readUID(uid, &uidLen)) {
        // Convert UID to string
        String currentUid = formatNfcUid(uid, uidLen);
        // Check if UID matches the authenticated user
        if (currentUid != currentUser.uid) {
            Serial.println("❌ Wrong NFC card detected!");
            Serial.print("Expected: " + currentUser.uid);
            Serial.print(", Got: " + currentUid);
            cancel_bid();
            hide_item_screen();
            hide_auction_screen();
            hide_pin_ui();
            show_custom_loading_timeout("\uF071 Wrong NFC detected", 3000);
            resetAuctionSelection();
            changeState(UI_WAITING_NFC);
            
            // Clear user session
            currentUser.uid = "";
            currentUser.userId = "";
            currentUser.role = "";
            currentUser.granted = false;
            currentUser.active = false;    
        }
    } else {
        // No NFC card detected
        Serial.println("⚠️ No NFC card detected"); 
        
        cancel_bid();
        hide_item_screen();
        hide_auction_screen();
        hide_pin_ui();
        show_custom_loading("\uF071 No NFC detected");
        
        lv_timer_t* t = lv_timer_create([](lv_timer_t* timer){
            if (currentUI == UI_WAITING_NFC) {
                show_custom_loading("Scan NFC Card...");
            }
            lv_timer_del(timer);
        }, 3000, nullptr);
        lv_timer_set_repeat_count(t, 1);
        
        resetAuctionSelection();
        changeState(UI_WAITING_NFC);
        
        // Clear user session
        currentUser.uid = "";
        currentUser.userId = "";
        currentUser.role = "";
        currentUser.granted = false;
        currentUser.active = false;
    }
}


void show_pin_ui() {
    // Safety check - don't create if already exists
    if (pin_card) {
        Serial.println("PIN UI already exists, hiding first");
        hide_pin_ui();
        delay(100);
    }
    
    // Hide auction screen
    if (main_card) {
        lv_obj_add_flag(main_card, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Hide loading if visible
    if (loading_label) {
        lv_obj_add_flag(loading_label, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Create PIN card
    pin_card = lv_obj_create(content_area);
    if (!pin_card) {
        Serial.println("Failed to create PIN card!");
        return;
    }
    
    lv_obj_set_size(pin_card, 240, 180);
    lv_obj_center(pin_card);
    lv_obj_set_style_bg_color(pin_card, lv_color_hex(0x2B2B36), 0);
    lv_obj_set_style_border_width(pin_card, 1, 0);
    lv_obj_set_style_border_color(pin_card, lv_color_hex(0x3A3A4A), 0);
    lv_obj_set_style_radius(pin_card, 15, 0);
    lv_obj_set_style_shadow_width(pin_card, 8, 0);
    lv_obj_set_style_shadow_color(pin_card, lv_color_hex(0x888888), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(pin_card);
    if (title) {
        lv_label_set_text(title, " Enter 4-Digit PIN");
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    }
    
    // Container for PIN boxes
    lv_obj_t* box_container = lv_obj_create(pin_card);
    if (box_container) {
        lv_obj_set_size(box_container, 200, 70);
        lv_obj_align(box_container, LV_ALIGN_TOP_MID, 0, 55);
        lv_obj_set_style_bg_color(box_container, lv_color_hex(0x1E1E24), 0);
        lv_obj_set_style_border_width(box_container, 0, 0);
        lv_obj_set_style_pad_all(box_container, 0, 0);
        lv_obj_set_flex_flow(box_container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(box_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(box_container, 10, 0);
        lv_obj_set_style_pad_column(box_container, 10, 0);
        
        // Create 4 PIN boxes
        for (int i = 0; i < PIN_LENGTH; i++) {
            lv_obj_t* box = lv_obj_create(box_container);
            if (box) {
                lv_obj_set_size(box, 40, 40);
                lv_obj_set_style_border_color(box, lv_color_hex(0x3A3A4A), 0);
                lv_obj_set_style_border_width(box, 2, 0);
                lv_obj_set_style_radius(box, 10, 0);
                lv_obj_set_style_bg_color(box, lv_color_hex(0x2B2B36), 0);
                
                lv_obj_t* label = lv_label_create(box);
                if (label) {
                    lv_label_set_text(label, "○");
                    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
                    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
                    lv_obj_center(label);
                    pin_boxes[i] = label;
                }
            }
        }
    }
    // Message label
    pin_message = lv_label_create(pin_card);
    if (pin_message) {
        lv_label_set_text(pin_message, " ");
        lv_obj_set_style_text_color(pin_message, lv_color_hex(0xA0A0A0), 0);
        lv_obj_set_style_text_font(pin_message, &lv_font_montserrat_14, 0);
        lv_obj_align(pin_message, LV_ALIGN_BOTTOM_MID, 0, -12);
    }
    
    // Reset PIN variables
    pinInput = "";
    currentBox = 0;
    update_pin_display();
    
    // Force LVGL to refresh
    lv_timer_handler();
}

// Hide PIN UI
void hide_pin_ui() {
    // Safely delete PIN UI objects
    if (pin_card) {
        // Delete children first
        for (int i = 0; i < PIN_LENGTH; i++) {
            pin_boxes[i] = nullptr;
        }
        pin_message = nullptr;  
        // Delete the card
        lv_obj_del(pin_card);
        pin_card = nullptr;
    }
    // Small delay for LVGL to process deletions
    lv_timer_handler();
    delay(10);
    // Restore auction screen only if we're not in AP mode
    if (main_card && !apModeActive && currentUI != UI_WAITING_NFC) {
        lv_obj_clear_flag(main_card, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(arrow_up, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(arrow_down, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_indicator, LV_OBJ_FLAG_HIDDEN);
        update_arrow_visibility();
    }
}

// Update PIN display
void update_pin_display() {
    // Safety check - if no boxes exist, return
    if (!pin_boxes[0]) return;
    for (int i = 0; i < PIN_LENGTH; i++) {
        if (pin_boxes[i]) {
            if (i < pinInput.length()) {
                char digit[2] = {pinInput[i], '\0'};
                lv_label_set_text(pin_boxes[i], digit);
                lv_obj_set_style_text_color(pin_boxes[i], lv_color_hex(0x5C9ACF), 0);
                lv_obj_set_style_text_font(pin_boxes[i], &lv_font_montserrat_14, 0);
            } else {
                lv_label_set_text(pin_boxes[i], "○");
                lv_obj_set_style_text_color(pin_boxes[i], lv_color_hex(0x444444), 0);
                lv_obj_set_style_text_font(pin_boxes[i], &lv_font_montserrat_14, 0);
            }
        }
    }
    // Highlight current box
    for (int i = 0; i < PIN_LENGTH; i++) {
        if (pin_boxes[i]) {
            lv_obj_t* box = lv_obj_get_parent(pin_boxes[i]);
            if (box) {
                if (i == currentBox && pinInput.length() < PIN_LENGTH) {
                    lv_obj_set_style_border_color(box, lv_color_hex(0x5C9ACF), 0);
                    lv_obj_set_style_border_width(box, 3, 0);
                    lv_obj_set_style_bg_color(box, lv_color_hex(0x3B3B4A), 0);
                } else {
                    lv_obj_set_style_border_color(box, lv_color_hex(0x444444), 0);
                    lv_obj_set_style_border_width(box, 2, 0);
                    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B2B36), 0);
                }
            }
        }
    }
}

void verify_and_proceed() {
    Serial.print("Verifying PIN: ");
    Serial.println(pinInput);
    if (pinInput == expectedPin) {
        Serial.println("✅ PIN Correct!");
        // Hide PIN UI
        hide_pin_ui(); 
        hide_auction_screen();
        // Proceed to next step
        changeState(UI_WAITING_NFC);
    } else {
        Serial.println("❌ PIN Wrong!"); 
        // Just show error message
        if (pin_message) {
            lv_label_set_text(pin_message, "\uF071 WRONG PIN!");
            lv_obj_set_style_text_color(pin_message, lv_color_hex(0xE57373), 0);
        } 
        // Clear PIN
        pinInput = "";
        currentBox = 0;
        update_pin_display();
    }
}






