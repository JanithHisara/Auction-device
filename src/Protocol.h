#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <functional>

// Match Auction structure from AuctionScreen.h
struct Auction {
    String Auction_ID;
    String Name;
    String Auction_Mode;
    String Auction_Status;
    String Start_DateTime;
    String End_DateTime;
    int Items_Count;
    int Registered_Count;
    
    Auction() : Items_Count(0), Registered_Count(0) {}
};

// Match NFCAccess structure
struct NFCAccess {
    struct AccessInfo {
        bool Granted;
        String User_ID;
        String Role;
        int Reason;
    };
    
    String Message_ID;
    String NFC_UID;
    String Status;
    AccessInfo Access;
};

// Match Item structure
struct Item {
    String Item_ID;
    String Name;
    String Status;
    String Currency;
    String End_DateTime;
    int Remaining_Seconds;
    float Current_Price;
    float Next_Min_Bid;
    bool Your_Bid_Submitted;
    
    Item() : Remaining_Seconds(0), Current_Price(0.0), 
             Next_Min_Bid(0.0), Your_Bid_Submitted(false) {}
};

// Match BidResult structure
struct BidResult {
    String Message_ID;
    String Status;
    String Auction_ID;
    String Auction_Mode;
    String Auction_Status;
    String Item_ID;
    String NFC_UID;
    String Bid_Status;
    float Current_Highest_Bid;
    float Next_Min_Bid;
    String Currency;
    int Reason;
    
    BidResult() : Current_Highest_Bid(0.0), Next_Min_Bid(0.0), Reason(0) {}
};

// Action handler type
typedef void (*ActionHandler)(JsonDocument& doc);

// Specialized handler types
typedef std::function<void(const std::vector<Auction>&)> AuctionsHandler;
typedef std::function<void(const NFCAccess&)> NFCHandler;
typedef std::function<void(const std::vector<Item>&, const String&, const String&)> ItemsHandler;
typedef std::function<void(const BidResult&)> BidHandler;

class Protocol {
private:
    MQTTClient& _mqttClient;
    const char* _reqTopic;
    const char* _resTopic;
    String _deviceId;
    
    // Response data storage
    std::vector<Auction> _auctions;
    NFCAccess _lastNFCAccess;
    std::vector<Item> _items;
    BidResult _lastBidResult;
    
    // Generic action handlers
    static const int MAX_HANDLERS = 20;
    struct HandlerEntry {
        String action;
        ActionHandler handler;
    };
    HandlerEntry _handlers[MAX_HANDLERS];
    int _handlerCount = 0;
    
    // Specialized handlers
    AuctionsHandler _auctionsHandler = nullptr;
    NFCHandler _nfcHandler = nullptr;
    ItemsHandler _itemsHandler = nullptr;
    BidHandler _bidHandler = nullptr;
    
    // Message deduplication
    struct MessageInfo {
        String messageId;
        unsigned long timestamp;
    };
    std::vector<MessageInfo> _recentMessages;
    static const int MAX_RECENT_MESSAGES = 10;
    static const unsigned long MESSAGE_TIMEOUT = 2000;
    
    bool isMessageDuplicate(const char* messageId);
    void cleanupRecentMessages();
    
public:
    // Public response structures (for backward compatibility)
    struct {
        String Message_ID;
        String Status;
        std::vector<Auction> Auctions;
    } lastResponse;
    
    struct {
        String Message_ID;
        String NFC_UID;
        String Status;
        struct {
            bool Granted;
            String User_ID;
            String Role;
            int Reason;
        } Access;
    } lastNFCAccessResponse;
    
    struct {
        String Message_ID;
        String Status;
        String Auction_ID;
        String Auction_Mode;
        String Auction_Status;
        int Items_Count;
        std::vector<Item> Items;
    } lastItemsResponse;
    
    struct {
        String Message_ID;
        String Status;
        String Auction_ID;
        String Auction_Mode;
        String Auction_Status;
        String Item_ID;
        String NFC_UID;
        String Bid_Status;
        float Current_Highest_Bid;
        float Next_Min_Bid;
        String Currency;
        int Reason;
    } lastBidResponse;
    
    Protocol(MQTTClient& mqttClient);
    
    // Initialization
    void begin(const char* reqTopic, const char* resTopic, const char* deviceId);
    void loop();
    
    // Publish methods
    bool publish(JsonDocument& doc);
    bool publishRequest(const char* action, const char* messageId = nullptr);
    
    // ==================== AUCTION METHODS ====================
    bool getAuctions(const char* messageId = nullptr);
    void onAuctions(AuctionsHandler handler);
    const std::vector<Auction>& getAuctions() const { return _auctions; }
    
    // ==================== NFC METHODS ====================
    bool checkNFCAccess(const char* nfcUid, const char* auctionId = nullptr, const char* messageId = nullptr);
    void onNFCCheck(NFCHandler handler);
    const NFCAccess& getNFCAccess() const { return _lastNFCAccess; }
    
    // ==================== ITEMS METHODS ====================
    bool getItems(const char* auctionId, const char* messageId = nullptr);
    void onItems(ItemsHandler handler);
    const std::vector<Item>& getItems() const { return _items; }
    
    // ==================== BID METHODS ====================
    bool submitBid(const char* auctionId, const char* itemId, const char* nfcUid,
                   float bidAmount, const char* currency = "$", 
                   const char* messageId = nullptr);
    void onBidResult(BidHandler handler);
    const BidResult& getBidResult() const { return _lastBidResult; }
    
    // ==================== GENERIC ACTION HANDLER ====================
    void onAction(const char* action, ActionHandler handler);
    
    // ==================== MESSAGE HANDLER ====================
    void handleMessage(char* topic, byte* payload, unsigned int length);
    
    // ==================== UTILITY METHODS ====================
    static String generateMessageId(const char* prefix = "MSG");
};

#endif
