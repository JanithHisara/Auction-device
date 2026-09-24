import { handler } from "./index.mjs";

const event = {
  requestContext: { http: { method: "POST" } },
  body: JSON.stringify({ type: "UPDATE", table: "auctions" })
};

async function run() {
  try {
    await handler(event);
    console.log("Mock handler execution completed.");
  } catch (e) {
    console.error("Handler threw error:", e);
  }
}

run();
