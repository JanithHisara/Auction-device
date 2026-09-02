import { IoTDataPlaneClient, PublishCommand } from "@aws-sdk/client-iot-data-plane";
import {
  supabase,
  fetchAuctions,
  fetchNFCCards,
  fetchActiveItems,
  fetchGemById,
  filterUserRegisteredGems,
  isUserRegisteredForAuction,
  placeBid as placeBidDB,
  hasUserBiddedOnGem
} from "./database.mjs";
import {
  updateLastMessageTime,
  getAuctionCache,
  getActiveItems,
  invalidateItemCache,
  checkNFCCard,
  setDeviceSession,
  getDeviceSession,
  toLocalISOString,
  mapAuctionMode
} from "./cache.mjs";

// Initialize IoT client
const client = new IoTDataPlaneClient({
  endpoint: "https://a1m322vfibs32e-ats.iot.ap-south-1.amazonaws.com"
});



// Helper function to calculate remaining seconds
function calculateRemainingSeconds(endTime) {
  if (!endTime) return 0;
  const end = new Date(endTime).getTime();
  const now = new Date().getTime();
  return Math.max(0, Math.floor((end - now) / 1000));
}

// Function to get gem UUID using cached active items
async function getGemUuid(formattedAuctionId, formattedItemId) {
  console.log(`   Looking up gem: Auction ${formattedAuctionId}, Item ${formattedItemId}`);
  try {
    const auctionCache = await getAuctionCache(fetchAuctions);
    const auctionUuid = auctionCache.formattedToUuid[formattedAuctionId];

    if (!auctionUuid) {
      console.log(`❌ Auction ID not found: ${formattedAuctionId}`);
      return null;
    }

    const itemsData = await getActiveItems(auctionUuid, formattedAuctionId, fetchActiveItems);

    if (!itemsData || itemsData.count === 0) {
      console.log(`❌ No active items for auction: ${formattedAuctionId}`);
      return null;
    }

    const gemUuid = itemsData.formattedToUuid[formattedItemId];

    if (!gemUuid) {
      console.log(`❌ Item ${formattedItemId} not found in active items`);
      return null;
    }

    console.log(`✅ Found: ${formattedItemId} -> ${gemUuid}`);
    return gemUuid;
  } catch (error) {
    console.error("❌ Error getting gem UUID:", error);
    return null;
  }
}

// Function to get formatted items for response with UPPERCASE field names
async function getAuctionItems(formattedAuctionId, userId = null) {
  console.log(`   Fetching items for auction: ${formattedAuctionId}`);
  try {
    const cache = await getAuctionCache(fetchAuctions);
    const auctionUuid = cache.formattedToUuid[formattedAuctionId];

    if (!auctionUuid) {
      console.log(`❌ Auction ID not found: ${formattedAuctionId}`);
      return null;
    }

    const itemsData = await getActiveItems(auctionUuid, formattedAuctionId, fetchActiveItems);

    if (!itemsData || itemsData.count === 0) {
      return {
        Auction_ID: formattedAuctionId,
        Auction_Mode: "OPEN",
        Auction_Status: "LIVE",
        Items: [],
        Items_Count: 0
      };
    }

    const auction = cache.auctions.find(a => a.id === auctionUuid);
    const auctionMode = mapAuctionMode(auction?.auction_type);

    const formattedItems = itemsData.activeItems.map((gem) => {
      const currentPrice = gem.current_price || gem.starting_price || 0;
      const baseItem = {
        Item_ID: gem.formattedId,
        Name: gem.name,
        Status: gem.status?.toUpperCase() || "LIVE",
        Current_Price: parseFloat(currentPrice),
        Currency: "$",
        End_DateTime: toLocalISOString(gem.end_time),
        Remaining_Seconds: calculateRemainingSeconds(gem.end_time)
      };

      if (auctionMode === "OPEN") {
        const increment = gem.min_bid_increment || 0;
        const nextBid = currentPrice + increment;
        return {
          ...baseItem,
          Next_Min_Bid: parseFloat(nextBid)
        };
      } else if (auctionMode === "ENGLISH") {
        return (async () => {
          let userBidAmount = null;
          if (userId) {
            userBidAmount = await hasUserBiddedOnGem(gem.id, userId);
          }
          return {
            ...baseItem,
            Current_Price: parseFloat(currentPrice),
            Next_Min_Bid: userBidAmount !== null ? parseFloat(userBidAmount) : 0, 
            Your_Bid_Submitted: userBidAmount !== null
          };
        })();
      } else {
        return (async () => {
          let userBidAmount = null;
          if (userId) {
            userBidAmount = await hasUserBiddedOnGem(gem.id, userId);
          }
          return {
            ...baseItem,
            Current_Price: parseFloat(currentPrice), // Initial Price
            Next_Min_Bid: userBidAmount !== null ? parseFloat(userBidAmount) : 0, // Abuse Next_Min_Bid to store user's bid!
            Your_Bid_Submitted: userBidAmount !== null
          };
        })();
      }
    });

    const resolvedItems = await Promise.all(formattedItems);

    return {
      Auction_ID: formattedAuctionId,
      Auction_Mode: auctionMode,
      Auction_Status: auction?.status?.toUpperCase() || "LIVE",
      Items: resolvedItems,
      Items_Count: resolvedItems.length
    };
  } catch (error) {
    console.error("❌ Error fetching gems:", error);
    throw error;
  }
}

// Function to place bid
async function placeBid(bidData, auctionUuid, auctionMode) {
  console.log("   Placing bid:", JSON.stringify(bidData, null, 2));
  try {
    const bidResult = await placeBidDB(bidData, auctionMode);
    invalidateItemCache(auctionUuid);
    console.log(`✅ Bid placed successfully for gem ${bidData.gem_id}`);
    return { success: true, bid: bidResult };
  } catch (error) {
    console.error("❌ Error placing bid:", error);
    return { success: false, error: error.message };
  }
}

// Main handler
export const handler = async (event) => {
  // ---------- SUPABASE WEBHOOK HANDLER ----------
  // If the event comes from an HTTP request (Function URL), it will have a 'body'
  if (event.requestContext && event.body) {
    try {
      const webhookPayload = typeof event.body === 'string' ? JSON.parse(event.body) : event.body;
      console.log("Received Webhook from Supabase:", webhookPayload);
      
      // We want to tell the device to refresh!
      const syncResponse = {
        Action: "SYNC_DB",
        Msg_Type: "response",
        DateTime: new Date().toISOString()
      };
      
      const command = new PublishCommand({
        topic: "auction/broadcast",
        payload: Buffer.from(JSON.stringify(syncResponse)),
        qos: 1
      });
      
      await client.send(command);
      console.log("Broadcasted SYNC_DB to all devices on auction/broadcast");
      
      return { statusCode: 200, body: "Webhook processed" };
    } catch (e) {
      console.error("Webhook processing failed", e);
      return { statusCode: 500, body: "Error" };
    }
  }

  updateLastMessageTime();
  console.log("   Received:", JSON.stringify(event));

  const deviceId = event.Device_ID || "unknown";
  const action = event.Action || "UNKNOWN";
  const messageId = event.Message_ID || "";

  let response = {
    Message_ID: messageId,
    Device_ID: deviceId,
    Action: action,
    Msg_Type: "response",
    DateTime: new Date().toISOString(),
  };

  try {
    // ---------- GET AUCTION ----------
    // All live auctions continue to be shown on the device before NFC scan
    if (action === "GET_AUCTION") {
      console.log("   Processing GET_AUCTION request");
      const cache = await getAuctionCache(fetchAuctions);
      response.Auctions = cache.auctionsList;
      response.Status = "SUCCESS";
    }
    // ---------- CHECK ACCESS ----------
    else if (action === "CHECK_ACCESS") {
      const nfcUid = event.NFC_UID || "";
      
      if (!nfcUid) {
        response.Status = "FAILED";
        response.Reason = 3;
        response.NFC_UID = "";
      } else {
        const result = await checkNFCCard(nfcUid, fetchNFCCards);
        response.NFC_UID = nfcUid;

        if (result.valid) {
          const formattedAuctionId = event.Auction_ID || "";
          let isRegistered = true;
          
          if (formattedAuctionId) {
            const auctionCache = await getAuctionCache(fetchAuctions);
            const auctionUuid = auctionCache.formattedToUuid[formattedAuctionId];
            if (auctionUuid) {
              isRegistered = await isUserRegisteredForAuction(auctionUuid, result.user_id);
            }
          }

          if (isRegistered) {
            response.Access = {
              Granted: true,
              User_ID: result.user_id,
              User_Name: result.user_name,
              Role: "Bidder"
            };
            response.Status = "SUCCESS";
            // Cache session for this device so GET_ITEMS knows the logged-in user if needed
            setDeviceSession(deviceId, nfcUid, result.user_id, result.user_name);
          } else {
            response.Status = "FAILED";
            response.Reason = 4; // Not registered for this auction
          }
        } else {
          response.Status = "FAILED";
          response.Reason = result.reason;
        }
      }
    }
    // ---------- GET ITEMS ----------
    else if (action === "GET_ITEMS") {
      const formattedAuctionId = event.Auction_ID || "";
      const session = getDeviceSession(deviceId);
        let userId = session?.user_id || event.User_ID || null;

        if (!userId && event.NFC_UID) {
          const cardInfo = await checkNFCCard(event.NFC_UID, fetchNFCCards);
          if (cardInfo && cardInfo.valid) {
            userId = cardInfo.user_id;
            setDeviceSession(deviceId, event.NFC_UID, userId, cardInfo.user_name);
          }
        }

      if (!formattedAuctionId) {
        response.Status = "FAILED";
        response.Reason = 3;
      } else {
        const itemsData = await getAuctionItems(formattedAuctionId, userId);

        if (!itemsData) {
            response.Auction_ID = formattedAuctionId;
            response.Auction_Mode = "OPEN";
            response.Auction_Status = "FINISHED";
            response.Items_Count = 0;
            response.Items = [];
            response.Status = "SUCCESS";
          } else {
          response.Auction_ID = itemsData.Auction_ID;
          response.Auction_Mode = itemsData.Auction_Mode;
          response.Auction_Status = itemsData.Auction_Status;
          response.Items_Count = itemsData.Items_Count;
          response.Items = itemsData.Items;
          response.Status = "SUCCESS";
        }
      }
    }
    // ---------- SUBMIT BID ----------
    else if (action === "SUBMIT_BID") {
      const nfcUid = event.NFC_UID || "";
      const formattedAuctionId = event.Auction_ID || "";
      const formattedItemId = event.Item_ID || "";
      const bidAmount = event.Bid_Amount || 0;
      const currency = event.Currency || "$";

      response.Auction_ID = formattedAuctionId;
      response.Item_ID = formattedItemId;
      response.NFC_UID = nfcUid;
      response.Currency = currency;

      const nfcResult = await checkNFCCard(nfcUid, fetchNFCCards);

      if (!nfcResult.valid) {
        response.Status = "FAILED";
        response.Bid_Status = "REJECTED";
        response.Reason = nfcResult.reason;
      } else {
        const auctionCache = await getAuctionCache(fetchAuctions);
        const auctionUuid = auctionCache.formattedToUuid[formattedAuctionId];
        
        // Ensure user is actually registered for this auction!
        const isRegistered = await isUserRegisteredForAuction(auctionUuid, nfcResult.user_id);
        if (!isRegistered) {
          response.Status = "FAILED";
          response.Bid_Status = "REJECTED";
          response.Reason = 4; // Not registered
        } else {
          const gemUuid = await getGemUuid(formattedAuctionId, formattedItemId);

        if (!gemUuid) {
          response.Status = "FAILED";
          response.Bid_Status = "REJECTED";
          response.Reason = 4;
        } else {
          // Check round end time
          const gemData = await fetchGemById(gemUuid);
          if (!gemData || !gemData.round_end_time) {
            response.Status = "FAILED";
            response.Bid_Status = "REJECTED";
            response.Reason = 6;
          } else {
            const now = new Date();
            const roundEnd = new Date(gemData.round_end_time);

            if (now > roundEnd) {
              response.Status = "FAILED";
              response.Bid_Status = "REJECTED";
              response.Reason = 7;
            } else {
              // ✅ ROUND ACTIVE -> allow bid
              const auction = auctionCache.auctions.find(a => a.id === auctionUuid);
                const auctionModeForBid = mapAuctionMode(auction);
                const bidResult = await placeBid({
                  gem_id: gemUuid,
                  user_id: nfcResult.user_id,
                  bid_amount: parseFloat(bidAmount)
                }, auctionUuid, auctionModeForBid);

              if (bidResult.success) {
                const auction = auctionCache.auctions.find(a => a.id === auctionUuid);
                response.Status = "SUCCESS";
                response.Bid_Status = "ACCEPTED";
                response.Current_Highest_Bid = parseFloat(bidAmount);
                response.Next_Min_Bid = parseFloat(bidAmount);
                response.Auction_Mode = mapAuctionMode(auction);
                response.Auction_Status = auction?.status?.toUpperCase() || "LIVE";
              } else if (bidResult.error === "DUPLICATE_BID") {
                response.Status = "FAILED";
                response.Bid_Status = "REJECTED";
                response.Reason = 8;
              } else if (bidResult.error === "ROUND_ENDED") {
                response.Status = "FAILED";
                response.Bid_Status = "REJECTED";
                response.Reason = 7;
              } else {
                response.Status = "FAILED";
                response.Bid_Status = "REJECTED";
                response.Reason = 500;
              }
            }
          }
        }
        }
      }
    }
    // ---------- Unknown Action ----------
    else {
      response.Status = "FAILED";
      response.Reason = 99;
    }
  } catch (error) {
    console.error("❌ Error:", error);
    response.Status = "FAILED";
    response.Reason = 500;
  }

  // Publish response
  const command = new PublishCommand({
    topic: `auction/${deviceId}/response`,
    payload: Buffer.from(JSON.stringify(response)),
    qos: 1
  });

  await client.send(command);
  console.log("✅ Response sent");
  return { result: "ok" };
};
