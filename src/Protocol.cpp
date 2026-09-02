#include "Protocol.h"

Protocol::Protocol(MQTTClient& mqttClient) : _mqttClient(mqttClient) {}

void Protocol::begin(const char* reqTopic, const char* resTopic, const char* deviceId) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    _deviceId = String(deviceId);
    // _mqttClient.setBufferSize(4096);
}

void Protocol::loop() {
    // Reserved for future use
}

bool Protocol::isMessageDuplicate(const char* messageId) {
    if (!messageId || strlen(messageId) == 0) return false;
    
    cleanupRecentMessages();
    unsigned long now = millis();
    String idStr = String(messageId);
    
    for (const auto& msg : _recentMessages) {
        if (msg.messageId == idStr) {
            Serial.print("⚠️ Duplicate message ignored: ");
            Serial.println(messageId);
            return true;
        }
    }
    
    if (_recentMessages.size() >= MAX_RECENT_MESSAGES) {
        _recentMessages.erase(_recentMessages.begin());
    }
    _recentMessages.push_back({idStr, now});
    
    return false;
}

void Protocol::cleanupRecentMessages() {
    unsigned long now = millis();
    _recentMessages.erase(
        std::remove_if(_recentMessages.begin(), _recentMessages.end(),
            [now](const MessageInfo& mi) { 
                return (now - mi.timestamp) > MESSAGE_TIMEOUT; 
            }),
        _recentMessages.end());
}

String Protocol::generateMessageId(const char* prefix) {
    static uint32_t counter = 0;
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s_%04lu_%06lu", 
             prefix ? prefix : "MSG", 
             (unsigned long)(millis() % 10000), 
             counter++);
    return String(buffer);
}

bool Protocol::publish(JsonDocument& doc) {
    if (!_mqttClient.connected()) {
        Serial.println("⚠️ MQTT not connected");
        return false;
    }
    
    if (!doc["Message_ID"].is<const char*>()) {
        doc["Message_ID"] = generateMessageId();
    }
    
    if (!doc["Device_ID"].is<const char*>()) {
        doc["Device_ID"] = _deviceId;
    }
    
    if (!doc["Msg_Type"].is<const char*>()) {
        doc["Msg_Type"] = "request";
    }
    
    if (!doc["DateTime"].is<unsigned long>()) {
        doc["DateTime"] = millis();
    }
    
    // Fix: Use a fixed buffer instead of variable-length array
    char buffer[512];
    size_t len = serializeJson(doc, buffer);
    
    if (len >= sizeof(buffer)) {
        Serial.println("⚠️ Message too large for buffer");
        return false;
    }
    
    Serial.print("📤 Publishing: ");
    Serial.println(buffer);
    
    return _mqttClient.publish(_reqTopic, buffer, false, 1);
}

bool Protocol::publishRequest(const char* action, const char* messageId) {
    JsonDocument doc;
    doc["Action"] = action;
    if (messageId) doc["Message_ID"] = messageId;
    return publish(doc);
}

bool Protocol::getAuctions(const char* messageId) {
    return publishRequest("GET_AUCTION", messageId);
}

bool Protocol::checkNFCAccess(const char* nfcUid, const char* auctionId, const char* messageId) {
    JsonDocument doc;
    doc["Action"] = "CHECK_ACCESS";
    doc["NFC_UID"] = nfcUid;
    if (auctionId && strlen(auctionId) > 0) doc["Auction_ID"] = auctionId;
    if (messageId) doc["Message_ID"] = messageId;
    return publish(doc);
}

bool Protocol::getItems(const char* auctionId, const char* messageId) {
    JsonDocument doc;
    doc["Action"] = "GET_ITEMS";
    doc["Auction_ID"] = auctionId;
    if (messageId) doc["Message_ID"] = messageId;
    return publish(doc);
}

bool Protocol::submitBid(const char* auctionId, const char* itemId, const char* nfcUid,
                        float bidAmount, const char* currency, const char* messageId) {
    JsonDocument doc;
    doc["Action"] = "SUBMIT_BID";
    doc["Auction_ID"] = auctionId;
    doc["Item_ID"] = itemId;
    doc["NFC_UID"] = nfcUid;
    doc["Bid_Amount"] = bidAmount;
    doc["Currency"] = currency;
    if (messageId) doc["Message_ID"] = messageId;
    return publish(doc);
}

void Protocol::onAuctions(AuctionsHandler handler) {
    _auctionsHandler = handler;
}

void Protocol::onNFCCheck(NFCHandler handler) {
    _nfcHandler = handler;
}

void Protocol::onItems(ItemsHandler handler) {
    _itemsHandler = handler;
}

void Protocol::onBidResult(BidHandler handler) {
    _bidHandler = handler;
}

void Protocol::onAction(const char* action, ActionHandler handler) {
    if (_handlerCount < MAX_HANDLERS) {
        _handlers[_handlerCount++] = {String(action), handler};
    } else {
        Serial.println("⚠️ Maximum handlers reached");
    }
}

void Protocol::handleMessage(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, _resTopic) != 0 && strcmp(topic, "auction/broadcast") != 0) return;
    
    if (length > 8192) {
        Serial.println("❌ Message too large (>8KB)");
        return;
    }
    
    char* jsonBuffer = (char*)malloc(length + 1);
    if (!jsonBuffer) {
        Serial.println("❌ Memory allocation failed");
        return;
    }
    
    memcpy(jsonBuffer, payload, length);
    jsonBuffer[length] = '\0';
    
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (const char*)jsonBuffer);
    free(jsonBuffer);
    
    if (err) {
        Serial.print("❌ JSON parse failed: ");
        Serial.println(err.c_str());
        return;
    }
    
    const char* messageId = doc["Message_ID"] | "";
    if (isMessageDuplicate(messageId)) {
        return;
    }
    
    const char* action = doc["Action"] | "";
    if (strlen(action) == 0) {
        Serial.println("⚠️ JSON missing Action");
        return;
    }
    
    const char* status = doc["Status"] | "UNKNOWN";
    
    Serial.printf("📥 Action: %s, Status: %s, MsgID: %s\n", 
                  action, status, messageId);
    
    // Clear previous data
    _auctions.clear();
    _items.clear();
    
    // ========== HANDLE GET_AUCTION ==========
    if (strcmp(action, "GET_AUCTION") == 0) {
        lastResponse.Message_ID = messageId;
        lastResponse.Status = status;
        lastResponse.Auctions.clear();
        
        if (strcmp(status, "SUCCESS") == 0 && doc["Auctions"].is<JsonArray>()) {
            JsonArray auctions = doc["Auctions"].as<JsonArray>();
            for (JsonObject auc : auctions) {
                Auction a;
                a.Auction_ID = auc["Auction_ID"] | "";
                a.Name = auc["Name"] | "";
                a.Auction_Mode = auc["Auction_Mode"] | "";
                a.Auction_Status = auc["Auction_Status"] | "";
                a.Start_DateTime = auc["Start_DateTime"] | "";
                a.End_DateTime = auc["End_DateTime"] | "";
                a.Items_Count = auc["Items_Count"] | 0;
                a.Registered_Count = auc["Registered_Count"] | 0;
                _auctions.push_back(a);
                lastResponse.Auctions.push_back(a);
            }
            Serial.printf("✅ Received %d auctions\n", _auctions.size());
            
            if (_auctionsHandler) {
                _auctionsHandler(_auctions);
            }
        }
    }
    
    // ========== HANDLE CHECK_ACCESS ==========
    else if (strcmp(action, "CHECK_ACCESS") == 0) {
        lastNFCAccessResponse.Message_ID = messageId;
        lastNFCAccessResponse.NFC_UID = doc["NFC_UID"] | "";
        lastNFCAccessResponse.Status = status;
        
        _lastNFCAccess.Message_ID = messageId;
        _lastNFCAccess.NFC_UID = doc["NFC_UID"] | "";
        _lastNFCAccess.Status = status;
        _lastNFCAccess.Access.Granted = false;
        _lastNFCAccess.Access.User_ID = "";
        _lastNFCAccess.Access.Role = "";
        _lastNFCAccess.Access.Reason = 0;
        
        if (strcmp(status, "SUCCESS") == 0 && doc["Access"].is<JsonObject>()) {
            JsonObject access = doc["Access"];
            bool granted = access["Granted"] | false;
            _lastNFCAccess.Access.Granted = granted;
            _lastNFCAccess.Access.User_ID = access["User_ID"] | "";
            _lastNFCAccess.Access.Role = access["Role"] | "";
            
            lastNFCAccessResponse.Access.Granted = granted;
            lastNFCAccessResponse.Access.User_ID = _lastNFCAccess.Access.User_ID;
            lastNFCAccessResponse.Access.Role = _lastNFCAccess.Access.Role;
        } else {
            _lastNFCAccess.Access.Reason = doc["Reason"] | 0;
            lastNFCAccessResponse.Access.Reason = _lastNFCAccess.Access.Reason;
        }
        
        Serial.printf("📥 NFC Access - Granted: %s\n", 
                      _lastNFCAccess.Access.Granted ? "YES" : "NO");
        
        if (_nfcHandler) {
            _nfcHandler(_lastNFCAccess);
        }
    }
    
    // ========== HANDLE GET_ITEMS ==========
    else if (strcmp(action, "GET_ITEMS") == 0) {
        String auctionId = doc["Auction_ID"] | "";
        String auctionMode = doc["Auction_Mode"] | "";
        
        lastItemsResponse.Message_ID = messageId;
        lastItemsResponse.Status = status;
        lastItemsResponse.Auction_ID = auctionId;
        lastItemsResponse.Auction_Mode = auctionMode;
        lastItemsResponse.Auction_Status = doc["Auction_Status"] | "";
        lastItemsResponse.Items_Count = doc["Items_Count"] | 0;
        lastItemsResponse.Items.clear();
        _items.clear();
        
        if (strcmp(status, "SUCCESS") == 0 && doc["Items"].is<JsonArray>()) {
            JsonArray items = doc["Items"].as<JsonArray>();
            for (JsonObject it : items) {
                Item item;
                item.Item_ID = it["Item_ID"] | "";
                item.Name = it["Name"] | "";
                item.Status = it["Status"] | "";
                item.Currency = it["Currency"] | "";
                item.End_DateTime = it["End_DateTime"] | "";
                item.Remaining_Seconds = it["Remaining_Seconds"] | 0;
                
                if (auctionMode == "ENGLISH" || auctionMode == "OPEN" || auctionMode == "English auction" || auctionMode == "Progressive elimination") {
                    item.Current_Price = it["Current_Price"] | 0.0;
                    item.Next_Min_Bid = it["Next_Min_Bid"] | 0.0;
                } else if (auctionMode == "CLOSED" || auctionMode == "TENDER" || auctionMode == "Sealed bid auction") {
                    item.Your_Bid_Submitted = it["Your_Bid_Submitted"] | false;
                }
                
                _items.push_back(item);
                lastItemsResponse.Items.push_back(item);
            }
            
            Serial.printf("✅ Received %d items for auction %s\n", 
                          _items.size(), auctionId.c_str());
            
            if (_itemsHandler) {
                _itemsHandler(_items, auctionId, auctionMode);
            }
        }
    }
    
    // ========== HANDLE SUBMIT_BID ==========
    else if (strcmp(action, "SUBMIT_BID") == 0) {
        lastBidResponse.Message_ID = messageId;
        lastBidResponse.Status = status;
        lastBidResponse.Auction_ID = doc["Auction_ID"] | "";
        lastBidResponse.Auction_Mode = doc["Auction_Mode"] | "";
        lastBidResponse.Auction_Status = doc["Auction_Status"] | "";
        lastBidResponse.Item_ID = doc["Item_ID"] | "";
        lastBidResponse.NFC_UID = doc["NFC_UID"] | "";
        lastBidResponse.Bid_Status = doc["Bid_Status"] | "";
        lastBidResponse.Current_Highest_Bid = doc["Current_Highest_Bid"] | 0.0;
        lastBidResponse.Next_Min_Bid = doc["Next_Min_Bid"] | 0.0;
        lastBidResponse.Currency = doc["Currency"] | "";
        lastBidResponse.Reason = doc["Reason"] | 0;
        
        _lastBidResult.Message_ID = messageId;
        _lastBidResult.Status = status;
        _lastBidResult.Auction_ID = lastBidResponse.Auction_ID;
        _lastBidResult.Auction_Mode = lastBidResponse.Auction_Mode;
        _lastBidResult.Auction_Status = lastBidResponse.Auction_Status;
        _lastBidResult.Item_ID = lastBidResponse.Item_ID;
        _lastBidResult.NFC_UID = lastBidResponse.NFC_UID;
        _lastBidResult.Bid_Status = lastBidResponse.Bid_Status;
        _lastBidResult.Current_Highest_Bid = lastBidResponse.Current_Highest_Bid;
        _lastBidResult.Next_Min_Bid = lastBidResponse.Next_Min_Bid;
        _lastBidResult.Currency = lastBidResponse.Currency;
        _lastBidResult.Reason = lastBidResponse.Reason;
        
        Serial.printf("📥 Bid Result - Status: %s, BidStatus: %s\n", 
                      status, _lastBidResult.Bid_Status.c_str());
        
        if (_bidHandler) {
            _bidHandler(_lastBidResult);
        }
    }
    
    // ========== CALL REGISTERED HANDLERS ==========
    for (int i = 0; i < _handlerCount; i++) {
        if (_handlers[i].action == action) {
            _handlers[i].handler(doc);
            return;
        }
    }
}

