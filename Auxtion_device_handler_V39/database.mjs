import { createClient } from "@supabase/supabase-js";

// Initialize Supabase client
const SUPABASE_URL = process.env.SUPABASE_URL;
const SUPABASE_KEY = process.env.SUPABASE_KEY;
export const supabase = createClient(SUPABASE_URL, SUPABASE_KEY);

export async function hasUserBiddedOnGem(gemId, userId) {
  if (!gemId || !userId) return null;
  try {
    const { data, error } = await supabase
      .from("bids")
      .select("bid_amount")
      .eq("gem_id", gemId)
      .eq("user_id", userId)
      .order("created_at", { ascending: false })
      .limit(1);
    if (!error && data && data.length > 0) return data[0].bid_amount;
    return null;
  } catch (e) {
    return null;
  }
}

// Fetch auctions (Returns all LIVE auctions as required for initial screen)
export async function fetchAuctions() {
  const { data: auctions, error } = await supabase
    .from("auctions")
    .select("*")
    .eq("status", "live")
    .order("created_at", { ascending: true });

  if (error) throw error;
  return auctions;
}

// Fetch NFC cards with user details
export async function fetchNFCCards() {
  const { data: nfcCards, error } = await supabase
    .from("nfc_cards")
    .select(`
      nfc_uid,
      user_id,
      is_active,
      users:user_id (
        display_name
      )
    `);

  if (error) throw error;
  return nfcCards;
}

// Fetch active items for auction
export async function fetchActiveItems(auctionUuid) {
  const { data: gems, error } = await supabase
    .from("gems")
    .select(`
      id,
      name,
      starting_price,
      min_bid_increment,
      status,
      end_time,
      round_end_time,
      current_price,
      auction_id,
      created_at
    `)
    .eq("auction_id", auctionUuid)
    .eq("status", "active")
    .order("created_at", { ascending: true });

  if (error) throw error;
  return gems;
}

// Helper to verify that a registration record has an approved/active status and is NOT rejected or pending
function isValidRegistration(row) {
  if (!row) return false;
  
  const statusFields = ["status", "registration_status", "approval_status", "state", "user_status", "participation_status"];
  const rejectedStatuses = ["rejected", "denied", "cancelled", "declined", "inactive", "revoked", "disabled", "banned", "pending", "unapproved", "blocked", "eliminated"];

  for (const field of statusFields) {
    if (row[field] !== undefined && row[field] !== null) {
      const val = String(row[field]).trim().toLowerCase();
      if (rejectedStatuses.includes(val)) {
        console.log(`🚫 Excluded registration record due to ${field} = '${row[field]}'`);
        return false;
      }
    }
  }
  return true;
}

// NEW: Filter active gems to ONLY those registered for this user (supports gem-level & auction-level registration)
export async function filterUserRegisteredGems(auctionUuid, userId, allActiveItems) {
  if (!allActiveItems || allActiveItems.length === 0) {
    console.log(`❌ No active items found in auction`);
    return [];
  }

  // If no userId, return all items (device session might not have user)
  if (!userId) {
    console.log(`⚠️ No userId provided - returning all ${allActiveItems.length} active items`);
    return allActiveItems;
  }

  try {
    console.log(`🔒 Checking registered gems for user ${userId} in auction ${auctionUuid}`);

    // 1. Check gem-specific registrations (e.g., gem_participants, gem_registrations, user_gems)
    const gemIds = allActiveItems.map(item => item.id || item.gem_id);
    const tablesToCheckGems = ["gem_participants", "gem_registrations", "user_gems", "registered_gems"];
    
    for (const table of tablesToCheckGems) {
      try {
        const { data, error } = await supabase
          .from(table)
          .select("*")
          .eq("user_id", userId)
          .in("gem_id", gemIds);

        if (!error && data && data.length > 0) {
          const validRows = data.filter(r => isValidRegistration(r));
          if (validRows.length > 0) {
            const regGemIds = new Set(validRows.map(r => r.gem_id));
            const filtered = allActiveItems.filter(item => regGemIds.has(item.id));
            console.log(`✅ Found ${filtered.length} valid registered gems for user in table '${table}'`);
            if (filtered.length > 0) return filtered;
          }
        }
      } catch (e) {
        // Table might not exist in schema, proceed to next table check
      }
    }

    // 2. Check auction-level registrations (e.g., auction_participants, auction_registrations, registrations)
    const tablesToCheckAuction = ["auction_participants", "auction_registrations", "registrations", "user_auctions"];
    
    for (const table of tablesToCheckAuction) {
      try {
        const { data, error } = await supabase
          .from(table)
          .select("*")
          .eq("auction_id", auctionUuid)
          .eq("user_id", userId);

        if (!error && data && data.length > 0) {
          const validRows = data.filter(r => isValidRegistration(r));
          if (validRows.length > 0) {
            console.log(`✅ User verified with valid status in auction table '${table}'. Returning all ${allActiveItems.length} active gems for this auction.`);
            return allActiveItems;
          } else {
            console.log(`🚫 User registration in table '${table}' is REJECTED or PENDING! Blocking items display.`);
            return [];
          }
        }
      } catch (e) {
        // Table might not exist, proceed
      }
    }

    // 3. FALLBACK: No registration table found for this auction/user — return all items
    // This allows the device to work even if no explicit registration table exists.
    // Access was already verified via CHECK_ACCESS (NFC scan), so we trust the session.
    console.log(`⚠️ No registration table found for user ${userId} in auction ${auctionUuid}. Falling back to all ${allActiveItems.length} active items (access was verified via NFC).`);
    return allActiveItems;

  } catch (error) {
    console.error("❌ Error during user registration verification:", error.message);
    // On error, still return all items so device isn't stuck
    return allActiveItems;
  }
}

// Check if user is registered for the auction (either auction-level OR gem-level for ANY gem in the auction, active or inactive)
export async function isUserRegisteredForAuction(auctionUuid, userId) {
  if (!userId || !auctionUuid) return false;
  try {
    console.log(`🔒 Checking if user ${userId} is registered for auction ${auctionUuid}`);

    // 1. Check auction-level registrations
    const tablesToCheckAuction = ["auction_participants", "auction_registrations", "registrations", "user_auctions"];
    for (const table of tablesToCheckAuction) {
      try {
        const { data, error } = await supabase
          .from(table)
          .select("*")
          .eq("auction_id", auctionUuid)
          .eq("user_id", userId);

        if (!error && data && data.length > 0) {
          const validRows = data.filter(r => isValidRegistration(r));
          if (validRows.length > 0) {
            console.log(`✅ User verified with valid status in auction table '${table}'`);
            return true;
          } else {
            console.log(`🚫 User registration in table '${table}' is REJECTED or PENDING! Denying access.`);
            return false;
          }
        }
      } catch (e) {}
    }

    // 2. Check gem-level registrations for ANY gem in this auction (even if not active yet)
    const { data: allGems, error: gemsErr } = await supabase
      .from("gems")
      .select("id")
      .eq("auction_id", auctionUuid);

    if (!gemsErr && allGems && allGems.length > 0) {
      const gemIds = allGems.map(g => g.id);
      const tablesToCheckGems = ["gem_participants", "gem_registrations", "user_gems", "registered_gems"];
      for (const table of tablesToCheckGems) {
        try {
          const { data, error } = await supabase
            .from(table)
            .select("*")
            .eq("user_id", userId)
            .in("gem_id", gemIds);

          if (!error && data && data.length > 0) {
            const validRows = data.filter(r => isValidRegistration(r));
            if (validRows.length > 0) {
              console.log(`✅ User verified in gem registration table '${table}' (${validRows.length} valid registered gems)`);
              return true;
            }
          }
        } catch (e) {}
      }
    }

    console.log(`❌ User ${userId} is NOT registered for auction ${auctionUuid} or any of its gems.`);
    return false;
  } catch (error) {
    console.error("❌ Error verifying user registration for auction:", error.message);
    return false;
  }
}

// Place bid (Supports updating existing bid for closed/sealed bid auctions)
export async function placeBid(bidData, auctionMode = "OPEN") {
  const { data: elimData, error: elimError } = await supabase
    .from("gem_eliminations")
    .select("id")
    .eq("gem_id", bidData.gem_id)
    .eq("user_id", bidData.user_id)
    .limit(1);

  if (elimData && elimData.length > 0) {
    throw new Error("ELIMINATED");
  }

  // 1 Get gem
  const { data: gem, error: gemError } = await supabase
    .from("gems")
    .select("id, round_end_time")
    .eq("id", bidData.gem_id)
    .single();

  if (gemError || !gem) {
    throw new Error("INVALID_ITEM");
  }

  // 2 Check round end
  const now = new Date().getTime();
  const roundEnd = new Date(gem.round_end_time).getTime();
  if (now >= roundEnd) {
    throw new Error("ROUND_ENDED");
  }

  const isClosed = auctionMode.toUpperCase().includes("CLOSED") || 
                   auctionMode.toUpperCase().includes("SEALED") || 
                   auctionMode.toUpperCase().includes("TENDER");

  if (isClosed) {
    // For CLOSED auctions, check if they already have a bid
    const { data: existingBid, error: checkError } = await supabase
      .from("bids")
      .select("id")
      .eq("gem_id", bidData.gem_id)
      .eq("user_id", bidData.user_id)
      .order("created_at", { ascending: false })
      .limit(1);

    if (checkError) throw checkError;

    if (existingBid && existingBid.length > 0) {
      // UPDATE their existing bid!
      const { data, error } = await supabase
        .from("bids")
        .update({ 
          bid_amount: bidData.bid_amount, 
          created_at: new Date().toISOString() 
        })
        .eq("id", existingBid[0].id)
        .select();

      if (error) throw error;
      return data[0];
    }
  } else if (auctionMode.toUpperCase().includes("PROGRESSIVE")) {
    // For PROGRESSIVE ELIMINATION, allow multiple users to bid the same amount!
    // But we still want to prevent the SAME user from double-tapping.
    const { data: existingBid, error: checkError } = await supabase
      .from("bids")
      .select("id")
      .eq("gem_id", bidData.gem_id)
      .eq("user_id", bidData.user_id)
      .eq("bid_amount", bidData.bid_amount)
      .limit(1);

    if (checkError) throw checkError;

    if (existingBid && existingBid.length > 0) {
      throw new Error("DUPLICATE_BID");
    }
  } else {
    // For OPEN / ENGLISH auctions, prevent ANYONE from bidding an amount that has already been bid
    // This enforces the "only one user can own the round/price" logic
    const { data: existingBid, error: checkError } = await supabase
      .from("bids")
      .select("id")
      .eq("gem_id", bidData.gem_id)
      .eq("bid_amount", bidData.bid_amount)
      .limit(1);

    if (checkError) throw checkError;

    if (existingBid && existingBid.length > 0) {
      throw new Error("DUPLICATE_BID");
    }
  }

  // 4. Insert new bid if no update occurred
  const { data, error } = await supabase
    .from("bids")
    .insert([{
      gem_id: bidData.gem_id,
      user_id: bidData.user_id,
      bid_amount: bidData.bid_amount,
      created_at: new Date().toISOString()
    }])
    .select();

  if (error) throw error;


  return data[0];
}

export async function fetchGemById(gemId) {
  const { data, error } = await supabase
    .from("gems")
    .select("id, round_end_time")
    .eq("id", gemId)
    .single();

  if (error) throw error;
  return data;
}

// Fetch user by ID to get display name
export async function fetchUserById(userId) {
  const { data: user, error } = await supabase
    .from("users")
    .select("display_name")
    .eq("id", userId)
    .single();

  if (error) throw error;
  return user;
}

// Fetch summary of all gems to compute total Items_Count and active (available) count for each auction
export async function fetchAllGemsSummary() {
  const { data: gems, error } = await supabase
    .from("gems")
    .select("id, auction_id, status");

  if (error) {
    console.error("❌ Error fetching gems summary:", error);
    return [];
  }
  return gems || [];
}

// Fetch count of valid registered users per auction
export async function fetchRegisteredUsersSummary() {
  const userCounts = {};
  const addUsers = (rows, getAuctionId) => {
    if (!rows) return;
    rows.forEach(r => {
      if (isValidRegistration(r) && r.user_id) {
        const aId = getAuctionId(r);
        if (aId) {
          if (!userCounts[aId]) userCounts[aId] = new Set();
          userCounts[aId].add(r.user_id);
        }
      }
    });
  };

  const tablesToCheckAuction = ["auction_participants", "auction_registrations", "registrations", "user_auctions"];
  for (const table of tablesToCheckAuction) {
    try {
      const { data, error } = await supabase.from(table).select("*");
      if (!error && data) {
        addUsers(data, r => r.auction_id);
      }
    } catch (e) {
      // Ignore if table does not exist
    }
  }

  const tablesToCheckGems = ["gem_participants", "gem_registrations", "user_gems", "registered_gems"];
  for (const table of tablesToCheckGems) {
    try {
      const { data, error } = await supabase.from(table).select("*, gems(auction_id)");
      if (!error && data) {
        addUsers(data, r => r.auction_id || r.gems?.auction_id);
      }
    } catch (e) {
      // Ignore if table does not exist
    }
  }

  const counts = {};
  for (const [aId, set] of Object.entries(userCounts)) {
    counts[aId] = set.size;
  }
  return counts;
}

export async function getUserIdFromNfc(nfcUid) {
  if (!nfcUid) return null;
  try {
    const { data, error } = await supabase
      .from("nfc_cards")
      .select("user_id")
      .eq("uid_number", nfcUid)
      .single();
    if (!error && data) return data.user_id;
  } catch (e) { }
  return null;
}
