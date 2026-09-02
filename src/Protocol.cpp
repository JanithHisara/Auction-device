#include "Protocol.h"

Protocol::Protocol(MQTTClient& mqttClient) : _mqttClient(mqttClient) {}

void Protocol::begin(const char* reqTopic, const char* resTopic, const char* deviceId) {
    _reqTopic = reqTopic;
    _resTopic = resTopic;
    strlcpy(_deviceId, deviceId, sizeof(_deviceId));
    // _mqttClient.setBufferSize(4096);
}

void Protocol::loop() {
    // Reserved for future use
}

bool Protocol::isMessageDuplicate(const char* messageId) {
    if (!messageId || strlen(messageId) == 0) return false;
    
    cleanupRecentMessages();
    unsigned long now = millis();
    const char* idStr = messageId;
    
    for (const auto& msg : _recentMessages) {
        if (strcmp(msg.messageId, idStr) == 0) {
            Serial.print("⚠️ Duplicate message ignored: ");
            Serial.println(messageId);
            return true;
        }
    }
    
    if (_recentMessages.size() >= MAX_RECENT_MESSAGES) {
        _recentMessages.erase(_recentMessages.begin());
    }
    MessageInfo mi; strlcpy(mi.messageId, idStr, sizeof(mi.messageId)); mi.timestamp = now; _recentMessages.push_back(mi);
    
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

const char* Protocol::generateMessageId(char* outBuffer, size_t maxLen, const char* prefix) {
    static uint32_t counter = 0;
    snprintf(outBuffer, maxLen, "%s_%04lu_%06lu", 
             prefix ? prefix : "MSG", 
             (unsigned long)(millis() % 10000), 
             counter++);
    return outBuffer;
}

bool Protocol::publish(JsonDocument& doc) {
    if (!_mqttClient.connected()) {
        Serial.println("⚠️ MQTT not connected");
        return false;
    }
    
    if (!doc["Message_ID"].is<const char*>()) {
        char msgBuf[32];
        doc["Message_ID"] = generateMessageId(msgBuf, sizeof(msgBuf));
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
        HandlerEntry entry; strlcpy(entry.action, action, sizeof(entry.action)); entry.handler = handler; _handlers[_handlerCount++] = entry;
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
        strlcpy(lastResponse.Message_ID, messageId, sizeof(lastResponse.Message_ID));
        strlcpy(lastResponse.Status, status, sizeof(lastResponse.Status));
        lastResponse.Auctions.clear();
        
        if (strcmp(status, "SUCCESS") == 0 && doc["Auctions"].is<JsonArray>()) {
            JsonArray auctions = doc["Auctions"].as<JsonArray>();
            for (JsonObject auc : auctions) {
                Auction a;
                strlcpy(a.Auction_ID, auc["Auction_ID"] | "", sizeof(a.Auction_ID));
                strlcpy(a.Name, auc["Name"] | "", sizeof(a.Name));
                strlcpy(a.Auction_Mode, auc["Auction_Mode"] | "", sizeof(a.Auction_Mode));
                strlcpy(a.Auction_Status, auc["Auction_Status"] | "", sizeof(a.Auction_Status));
                strlcpy(a.Start_DateTime, auc["Start_DateTime"] | "", sizeof(a.Start_DateTime));
                strlcpy(a.End_DateTime, auc["End_DateTime"] | "", sizeof(a.End_DateTime));
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
        strlcpy(lastNFCAccessResponse.Message_ID, messageId, sizeof(lastNFCAccessResponse.Message_ID));
        strlcpy(lastNFCAccessResponse.NFC_UID, doc["NFC_UID"] | "", sizeof(lastNFCAccessResponse.NFC_UID));
        strlcpy(lastNFCAccessResponse.Status, status, sizeof(lastNFCAccessResponse.Status));
        
        strlcpy(_lastNFCAccess.Message_ID, messageId, sizeof(_lastNFCAccess.Message_ID));
        strlcpy(_lastNFCAccess.NFC_UID, doc["NFC_UID"] | "", sizeof(_lastNFCAccess.NFC_UID));
        strlcpy(_lastNFCAccess.Status, status, sizeof(_lastNFCAccess.Status));
        _lastNFCAccess.Access.Granted = false;
        _lastNFCAccess.Access.User_ID[0] = '\0';
        _lastNFCAccess.Access.Role[0] = '\0';
        _lastNFCAccess.Access.Reason = 0;
        
        if (strcmp(status, "SUCCESS") == 0 && doc["Access"].is<JsonObject>()) {
            JsonObject access = doc["Access"];
            bool granted = access["Granted"] | false;
            _lastNFCAccess.Access.Granted = granted;
            strlcpy(_lastNFCAccess.Access.User_ID, access["User_ID"] | "", sizeof(_lastNFCAccess.Access.User_ID));
            strlcpy(_lastNFCAccess.Access.Role, access["Role"] | "", sizeof(_lastNFCAccess.Access.Role));
            
            lastNFCAccessResponse.Access.Granted = granted;
            strlcpy(lastNFCAccessResponse.Access.User_ID, _lastNFCAccess.Access.User_ID, sizeof(lastNFCAccessResponse.Access.User_ID));
            strlcpy(lastNFCAccessResponse.Access.Role, _lastNFCAccess.Access.Role, sizeof(lastNFCAccessResponse.Access.Role));
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
        const char* auctionId = doc["Auction_ID"] | "";
        const char* auctionMode = doc["Auction_Mode"] | "";
        
        strlcpy(lastItemsResponse.Message_ID, messageId, sizeof(lastItemsResponse.Message_ID));
        strlcpy(lastItemsResponse.Status, status, sizeof(lastItemsResponse.Status));
        strlcpy(lastItemsResponse.Auction_ID, auctionId, sizeof(lastItemsResponse.Auction_ID));
        strlcpy(lastItemsResponse.Auction_Mode, auctionMode, sizeof(lastItemsResponse.Auction_Mode));
        strlcpy(lastItemsResponse.Auction_Status, doc["Auction_Status"] | "", sizeof(lastItemsResponse.Auction_Status));
        lastItemsResponse.Items_Count = doc["Items_Count"] | 0;
        lastItemsResponse.Items.clear();
        _items.clear();
        
        if (strcmp(status, "SUCCESS") == 0 && doc["Items"].is<JsonArray>()) {
            JsonArray items = doc["Items"].as<JsonArray>();
            for (JsonObject it : items) {
                Item item;
                strlcpy(item.Item_ID, it["Item_ID"] | "", sizeof(item.Item_ID));
                strlcpy(item.Name, it["Name"] | "", sizeof(item.Name));
                strlcpy(item.Status, it["Status"] | "", sizeof(item.Status));
                strlcpy(item.Currency, it["Currency"] | "", sizeof(item.Currency));
                strlcpy(item.End_DateTime, it["End_DateTime"] | "", sizeof(item.End_DateTime));
                item.Remaining_Seconds = it["Remaining_Seconds"] | 0;
                
                if (strcmp(auctionMode, "ENGLISH") == 0 || strcmp(auctionMode, "OPEN") == 0 || strcmp(auctionMode, "English auction") == 0 || strcmp(auctionMode, "Progressive elimination") == 0) {
                    item.Current_Price = it["Current_Price"] | 0.0;
                    item.Next_Min_Bid = it["Next_Min_Bid"] | 0.0;
                } else if (strcmp(auctionMode, "CLOSED") == 0 || strcmp(auctionMode, "TENDER") == 0 || strcmp(auctionMode, "Sealed bid auction") == 0) {
                    item.Your_Bid_Submitted = it["Your_Bid_Submitted"] | false;
                }
                
                _items.push_back(item);
                lastItemsResponse.Items.push_back(item);
            }
            
            Serial.printf("✅ Received %d items for auction %s\n", 
                          _items.size(), auctionId);
            
            if (_itemsHandler) {
                _itemsHandler(_items, auctionId, auctionMode);
            }
        }
    }
    
    // ========== HANDLE SUBMIT_BID ==========
    else if (strcmp(action, "SUBMIT_BID") == 0) {
        strlcpy(lastBidResponse.Message_ID, messageId, sizeof(lastBidResponse.Message_ID));
        strlcpy(lastBidResponse.Status, status, sizeof(lastBidResponse.Status));
        strlcpy(lastBidResponse.Auction_ID, doc["Auction_ID"] | "", sizeof(lastBidResponse.Auction_ID));
        strlcpy(lastBidResponse.Auction_Mode, doc["Auction_Mode"] | "", sizeof(lastBidResponse.Auction_Mode));
        strlcpy(lastBidResponse.Auction_Status, doc["Auction_Status"] | "", sizeof(lastBidResponse.Auction_Status));
        strlcpy(lastBidResponse.Item_ID, doc["Item_ID"] | "", sizeof(lastBidResponse.Item_ID));
        strlcpy(lastBidResponse.NFC_UID, doc["NFC_UID"] | "", sizeof(lastBidResponse.NFC_UID));
        strlcpy(lastBidResponse.Bid_Status, doc["Bid_Status"] | "", sizeof(lastBidResponse.Bid_Status));
        lastBidResponse.Current_Highest_Bid = doc["Current_Highest_Bid"] | 0.0;
        lastBidResponse.Next_Min_Bid = doc["Next_Min_Bid"] | 0.0;
        strlcpy(lastBidResponse.Currency, doc["Currency"] | "", sizeof(lastBidResponse.Currency));
        lastBidResponse.Reason = doc["Reason"] | 0;
        
        strlcpy(_lastBidResult.Message_ID, messageId, sizeof(_lastBidResult.Message_ID));
        strlcpy(_lastBidResult.Status, status, sizeof(_lastBidResult.Status));
        strlcpy(_lastBidResult.Auction_ID, lastBidResponse.Auction_ID, sizeof(_lastBidResult.Auction_ID));
        strlcpy(_lastBidResult.Auction_Mode, lastBidResponse.Auction_Mode, sizeof(_lastBidResult.Auction_Mode));
        strlcpy(_lastBidResult.Auction_Status, lastBidResponse.Auction_Status, sizeof(_lastBidResult.Auction_Status));
        strlcpy(_lastBidResult.Item_ID, lastBidResponse.Item_ID, sizeof(_lastBidResult.Item_ID));
        strlcpy(_lastBidResult.NFC_UID, lastBidResponse.NFC_UID, sizeof(_lastBidResult.NFC_UID));
        strlcpy(_lastBidResult.Bid_Status, lastBidResponse.Bid_Status, sizeof(_lastBidResult.Bid_Status));
        _lastBidResult.Current_Highest_Bid = lastBidResponse.Current_Highest_Bid;
        _lastBidResult.Next_Min_Bid = lastBidResponse.Next_Min_Bid;
        strlcpy(_lastBidResult.Currency, lastBidResponse.Currency, sizeof(_lastBidResult.Currency));
        _lastBidResult.Reason = lastBidResponse.Reason;
        
        Serial.printf("📥 Bid Result - Status: %s, BidStatus: %s\n", 
                      status, _lastBidResult.Bid_Status);
        
        if (_bidHandler) {
            _bidHandler(_lastBidResult);
        }
    }
    
    // ========== CALL REGISTERED HANDLERS ==========
    for (int i = 0; i < _handlerCount; i++) {
        if (strcmp(_handlers[i].action, action) == 0) {
            _handlers[i].handler(doc);
            return;
        }
    }
}

