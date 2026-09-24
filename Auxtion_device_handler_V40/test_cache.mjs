import { supabase } from './database.mjs';
import { updateAuctionCache, getAuctionCache } from './cache.mjs';

async function fetchAuctions() {
    const { data: auctions, error } = await supabase
      .from("auctions")
      .select("*")
      .eq("status", "live")
      .order("created_at", { ascending: true });
    if (error) throw error;
    return auctions;
}

async function test() {
    await updateAuctionCache(fetchAuctions);
    const cache = await getAuctionCache(fetchAuctions);
    console.log(JSON.stringify(cache.auctionsList, null, 2));
}

test();