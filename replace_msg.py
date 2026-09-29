path = 'C:/Janith/Auction hub/AuxtionHub_Device/src/ItemScreen.cpp'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()

target = '''            if (stat == "ended" || stat == "completed" || stat == "finished") { 
                lv_label_set_text(item_loading_label, "Auction is finished"); 
            } else if (!stat.equalsIgnoreCase("live") && !stat.equalsIgnoreCase("open")) { 
                lv_label_set_text(item_loading_label, "Auction is not live"); 
            } else { 
                const char* userName = get_current_user_name();
                if (userName && strlen(userName) > 0) {
                    char loading_msg[128];
                    snprintf(loading_msg, sizeof(loading_msg), "%s, auction is live but items are not placed yet please wait", userName);
                    lv_label_set_text(item_loading_label, loading_msg);
                } else {
                    lv_label_set_text(item_loading_label, "Auction is live but items are not placed yet please wait"); 
                }
            }'''

replacement = '''            const char* userName = get_current_user_name();
            char loading_msg[128];
            bool hasUser = (userName && strlen(userName) > 0);

            if (stat == "ended" || stat == "completed" || stat == "finished") { 
                if (hasUser) {
                    snprintf(loading_msg, sizeof(loading_msg), "%s, the auction has finished. Thank you for participating!", userName);
                    lv_label_set_text(item_loading_label, loading_msg);
                } else {
                    lv_label_set_text(item_loading_label, "The auction has finished. Thank you for participating!");
                }
            } else if (!stat.equalsIgnoreCase("live") && !stat.equalsIgnoreCase("open")) { 
                if (hasUser) {
                    snprintf(loading_msg, sizeof(loading_msg), "%s, this auction is not live yet. Please wait.", userName);
                    lv_label_set_text(item_loading_label, loading_msg);
                } else {
                    lv_label_set_text(item_loading_label, "This auction is not live yet. Please wait.");
                }
            } else { 
                if (hasUser) {
                    snprintf(loading_msg, sizeof(loading_msg), "%s, auction is live but items are not placed yet. Please wait.", userName);
                    lv_label_set_text(item_loading_label, loading_msg);
                } else {
                    lv_label_set_text(item_loading_label, "Auction is live but items are not placed yet. Please wait."); 
                }
            }'''

if target in text:
    print('Found and replaced')
    text = text.replace(target, replacement)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(text)
else:
    print('Target not found!')
