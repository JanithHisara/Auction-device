import { fetchAllGemsSummary, fetchRegisteredUsersSummary } from "./database.mjs";

// Helper to convert DB UTC timestamp into local Sri Lanka (UTC+05:30) website time YYYY-MM-DDTHH:mm:ss for accurate display on device screen
export function toLocalISOString(dateStr) {
  if (!dateStr) return "";
  try {
    const d = new Date(dateStr);
    if (isNaN(d.getTime())) return String(dateStr);
    // Adjust by +5 hours 30 minutes (330 mins * 60000 ms) for exact website local time
    const localTime = new Date(d.getTime() + (5 * 60 + 30) * 60000);
    return localTime.toISOString().replace(".000Z", "").replace("Z", "");
  } catch (e) {
    return String(dateStr);
  }
}

// Cache structures
let auctionCache = {
  lastUpdated: null,
  formattedToUuid: {},
  uuidToFormatted: {},
  auctions: [],
  auctionsList: []
};

let itemsCache = {
  lastUpdated: null,
  byAuction: {}
};

let nfcCache = {
  lastUpdated: null,
  uidToUserDetails: {} // Stores: { user_id, is_active, display_name }
};

let deviceSessions = {}; // Stores Device_ID => { nfc_uid, user_id, user_name, timestamp }
let lastMessageTime = Date.now();

// Cache TTL
const CACHE_TTL = 1000; // 1 second for near real-time synchronization
const ITEMS_CACHE_TTL = 1000; // 1 second for live rounds, activated gems, and finished rounds
const NFC_CACHE_TTL = 5 * 60 * 1000;
const IDLE_TIMEOUT = 2 * 60 * 1000;

// Helper functions
function formatAuctionId(index) {
  const number = (index + 1).toString().padStart(3, '0');
  return `AUC${number}`;
}

export function mapAuctionMode(auction) {
  if (!auction) return "UNKNOWN";
  const rawType = typeof auction === 'string' ? auction : (auction.auction_type || auction.type || auction.mode || auction.auction_mode || auction.bidding_type || auction.bidding_mode || auction.category);
  if (!rawType) return "UNKNOWN";
  const modeStr = String(rawType).toUpperCase().replace(/_/g, " ").trim();
  
  if (modeStr.includes("INCREMENTAL APPROVAL")) {
    return "Progressive elimination";
  } else if (modeStr.includes("PROGRESSIVE ELIMINATION")) {
    return "English auction";
  } else if (modeStr.includes("TENDERBASE") || modeStr.includes("TENDER")) {
    return "Sealed bid auction";
  }
  
  return modeStr;
}

function isIdle() {
  return (Date.now() - lastMessageTime) > IDLE_TIMEOUT;
}

// Export functions
export function updateLastMessageTime() {
  lastMessageTime = Date.now();
}

export function setDeviceSession(deviceId, nfcUid, userId, userName) {
  if (!deviceId || deviceId === "unknown") return;
  deviceSessions[deviceId] = {
    nfc_uid: nfcUid,
    user_id: userId,
    user_name: userName,
    timestamp: Date.now()
  };
  console.log(`💾 Recorded active session for device ${deviceId}: User ${userName} (${userId})`);
}

export function getDeviceSession(deviceId) {
  return deviceSessions[deviceId] || null;
}

export async function updateAuctionCache(fetchAuctionsFn) {
  console.log("   Fetching fresh auctions, gem counts, and registered user counts...");
  const [auctions, allGems, userCounts] = await Promise.all([
    fetchAuctionsFn(),
    fetchAllGemsSummary(),
    fetchRegisteredUsersSummary()
  ]);

  // Group gems by auction_id to compute total Items count and active Available count
  const countMap = {};
  if (allGems && allGems.length > 0) {
    allGems.forEach(g => {
      const aId = g.auction_id;
      if (!countMap[aId]) countMap[aId] = { total: 0, available: 0 };
      countMap[aId].total++;
      if (g.status === "active") {
        countMap[aId].available++;
      }
    });
  }

  const formattedToUuid = {};
  const uuidToFormatted = {};
  const auctionsList = [];

  auctions.forEach((auction, index) => {
    const formattedId = formatAuctionId(index);
    formattedToUuid[formattedId] = auction.id;
    uuidToFormatted[auction.id] = formattedId;

    const counts = countMap[auction.id] || { total: 0, available: 0 };
    const regUsers = userCounts[auction.id] || 0;

    auctionsList.push({
      Auction_ID: formattedId,
      Name: auction.name,
      Auction_Mode: mapAuctionMode(auction),
      Auction_Status: auction.status?.toUpperCase() || "LIVE",
      Start_DateTime: toLocalISOString(auction.auction_start),
      End_DateTime: toLocalISOString(auction.auction_end),
      Items_Count: counts.total,
      Registered_Count: regUsers
    });
  });

  auctionCache = {
    lastUpdated: Date.now(),
    formattedToUuid,
    uuidToFormatted,
    auctions,
    auctionsList
  };

  console.log(`✅ Auction cache updated with ${auctions.length} auctions`);
}

export async function updateNFCCache(fetchNFCCardsFn) {
  console.log("   Fetching fresh NFC cards with user details...");
  const nfcCards = await fetchNFCCardsFn();
  const uidToUserDetails = {};

  nfcCards.forEach(card => {
    const displayName = card.users?.display_name || "Unknown User";
    uidToUserDetails[card.nfc_uid] = {
      user_id: card.user_id,
      is_active: card.is_active,
      display_name: displayName
    };
  });

  nfcCache = {
    lastUpdated: Date.now(),
    uidToUserDetails
  };

  console.log(`✅ NFC cache updated with ${nfcCards.length} cards`);
}

// Function to check single NFC card directly from database
async function checkSingleNFCCardInDB(nfcUid, fetchNFCCardsFn) {
  console.log(`   Checking NFC UID directly in database: ${nfcUid}`);
  try {
    const supabase = (await import('./database.mjs')).supabase;
    const { data: card, error } = await supabase
      .from("nfc_cards")
      .select(`
        nfc_uid,
        user_id,
        is_active,
        users:user_id (
          display_name
        )
      `)
      .eq("nfc_uid", nfcUid)
      .single();

    if (error || !card) {
      console.log(`❌ NFC UID not found in DB: ${nfcUid}`);
      return null;
    }

    const displayName = card.users?.display_name || "Unknown User";
    const cardDetails = {
      user_id: card.user_id,
      is_active: card.is_active,
      display_name: displayName
    };

    console.log(`✅ Found NFC card in DB: ${displayName} (${card.user_id})`);
    nfcCache.uidToUserDetails[nfcUid] = cardDetails;
    return cardDetails;
  } catch (error) {
    console.error("❌ Error checking NFC card in DB:", error);
    return null;
  }
}

// Smart NFC check with immediate DB lookup if not in cache
export async function checkNFCCard(nfcUid, fetchNFCCardsFn) {
  console.log(`   Checking NFC UID: ${nfcUid}`);
  try {
    const cache = await getNFCCache(fetchNFCCardsFn);
    let cardDetails = cache.uidToUserDetails[nfcUid];

    if (!cardDetails) {
      console.log(`⚠️ NFC UID ${nfcUid} not in cache, checking database directly...`);
      cardDetails = await checkSingleNFCCardInDB(nfcUid, fetchNFCCardsFn);
      if (!cardDetails) {
        console.log(`❌ NFC UID not found in database: ${nfcUid}`);
        return { valid: false, reason: 1 };
      }
    } else {
      console.log(`✅ NFC UID found in cache: ${cardDetails.display_name}`);
    }

    if (!cardDetails.is_active) {
      console.log(`❌ NFC UID inactive: ${nfcUid}`);
      return { valid: false, reason: 2 };
    }

    console.log(`✅ NFC UID is valid. User: ${cardDetails.display_name} (${cardDetails.user_id})`);
    return {
      valid: true,
      user_id: cardDetails.user_id,
      user_name: cardDetails.display_name
    };
  } catch (error) {
    console.error("❌ Error checking NFC:", error);
    return { valid: false, reason: 500 };
  }
}

export async function getAuctionCache(fetchAuctionsFn) {
  const now = Date.now();
  const cacheAge = now - (auctionCache.lastUpdated || 0);

  if (isIdle()) {
    console.log(`⏱️ System idle, using existing auction cache`);
    return auctionCache;
  }

  if (!auctionCache.lastUpdated || cacheAge > CACHE_TTL) {
    console.log(`⚠️ Auction cache age: ${Math.round(cacheAge / 1000)}s, refreshing...`);
    await updateAuctionCache(fetchAuctionsFn);
  }
  return auctionCache;
}

export async function getNFCCache(fetchNFCCardsFn) {
  const now = Date.now();
  const cacheAge = now - (nfcCache.lastUpdated || 0);

  if (isIdle()) {
    console.log(`⏱️ System idle, using existing NFC cache`);
    return nfcCache;
  }

  if (!nfcCache.lastUpdated || cacheAge > NFC_CACHE_TTL) {
    console.log(`⚠️ NFC cache age: ${Math.round(cacheAge / 1000)}s, refreshing...`);
    await updateNFCCache(fetchNFCCardsFn);
  }
  return nfcCache;
}

export async function getActiveItems(auctionUuid, formattedAuctionId, fetchActiveItemsFn) {
  const now = Date.now();
  const auctionItemCache = itemsCache.byAuction[auctionUuid];

  if (auctionItemCache) {
    auctionItemCache.lastAccessed = now;
  }

  if (auctionItemCache && (now - auctionItemCache.lastUpdated) < ITEMS_CACHE_TTL && !isIdle()) {
    console.log(`   Using cached items for auction ${formattedAuctionId}`);
    return auctionItemCache;
  }

  if (isIdle() && auctionItemCache) {
    console.log(`⏱️ System idle, using expired cache for auction ${formattedAuctionId}`);
    return auctionItemCache;
  }

  console.log(`   Fetching fresh active items for auction ${formattedAuctionId}...`);
  const gems = await fetchActiveItemsFn(auctionUuid);
  const formattedToUuid = {};
  const uuidToFormatted = {};
  const activeItems = [];

  gems.forEach((gem, index) => {
    const itemNumber = (index + 1).toString().padStart(3, '0');
    const formattedId = `ITEM${itemNumber}`;
    formattedToUuid[formattedId] = gem.id;
    uuidToFormatted[gem.id] = formattedId;
    activeItems.push({ ...gem, formattedId });
  });

  itemsCache.byAuction[auctionUuid] = {
    lastUpdated: now,
    lastAccessed: now,
    activeItems,
    formattedToUuid,
    uuidToFormatted,
    count: activeItems.length
  };
  itemsCache.lastUpdated = now;

  console.log(`✅ Cached ${activeItems.length} active items`);
  return itemsCache.byAuction[auctionUuid];
}

export function invalidateItemCache(auctionUuid) {
  if (auctionUuid && itemsCache.byAuction[auctionUuid]) {
    console.log(`   Invalidating item cache for auction ${auctionUuid}`);
    itemsCache.byAuction[auctionUuid].lastUpdated = 0;
  }
}
