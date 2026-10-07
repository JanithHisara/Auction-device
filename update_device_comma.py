with open('src/ItemScreen.cpp', 'r') as f:
    content = f.read()

# 1. Update the parsing in lv_timer_t block
old_parse = 'bid_amount = atof(bid_text);'
new_parse = '''String bid_str = String(bid_text);
        bid_str.replace(",", "");
        bid_amount = bid_str.toDouble();'''
content = content.replace(old_parse, new_parse)

# 2. Update add_to_bid_textarea
old_add = '''void add_to_bid_textarea(const char* text) {
    // Only allow input for closed bid auctions
    if (current_mode != AUCTION_MODE_ENGLISH && 
        bid_textarea != nullptr && 
        bid_popup_active) {
        lv_textarea_add_text(bid_textarea, text);
    }
}'''

new_add = '''void add_to_bid_textarea(const char* text) {
    if (current_mode != AUCTION_MODE_ENGLISH && bid_textarea != nullptr && bid_popup_active) {
        String raw = String(lv_textarea_get_text(bid_textarea));
        raw.replace(",", "");
        raw += text;
        
        String formatted = "";
        int len = raw.length();
        for (int i = 0; i < len; i++) {
            formatted += raw[i];
            if ((len - i - 1) > 0 && (len - i - 1) % 3 == 0) {
                formatted += ",";
            }
        }
        lv_textarea_set_text(bid_textarea, formatted.c_str());
    }
}'''
content = content.replace(old_add, new_add)

# 3. Update backspace_bid_textarea
old_backspace = '''void backspace_bid_textarea() {
    // Only allow input for closed bid auctions
    if (current_mode != AUCTION_MODE_ENGLISH && 
        bid_textarea != nullptr && 
        bid_popup_active) {
        lv_textarea_del_char(bid_textarea);
    }
}'''

new_backspace = '''void backspace_bid_textarea() {
    if (current_mode != AUCTION_MODE_ENGLISH && bid_textarea != nullptr && bid_popup_active) {
        String raw = String(lv_textarea_get_text(bid_textarea));
        raw.replace(",", "");
        if (raw.length() > 0) {
            raw.remove(raw.length() - 1);
        }
        
        String formatted = "";
        int len = raw.length();
        for (int i = 0; i < len; i++) {
            formatted += raw[i];
            if ((len - i - 1) > 0 && (len - i - 1) % 3 == 0) {
                formatted += ",";
            }
        }
        lv_textarea_set_text(bid_textarea, formatted.c_str());
    }
}'''
content = content.replace(old_backspace, new_backspace)

with open('src/ItemScreen.cpp', 'w') as f:
    f.write(content)
print("Updated device sealed bid input parsing and formatting.")
