path = 'C:/Janith/Auction hub/AuxtionHub_Device/src/ItemScreen.cpp'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()

target = '''    item_loading_label = lv_label_create(parent);
    lv_label_set_text(item_loading_label, "Loading Items1...");
    lv_obj_center(item_loading_label);
    lv_obj_set_style_text_color(item_loading_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(item_loading_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);'''

replacement = '''    item_loading_label = lv_label_create(parent);
    lv_obj_set_width(item_loading_label, 280);
    lv_label_set_long_mode(item_loading_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(item_loading_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(item_loading_label, "Loading Items...");
    lv_obj_center(item_loading_label);
    lv_obj_set_style_text_color(item_loading_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(item_loading_label, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(item_loading_label, LV_OBJ_FLAG_HIDDEN);'''

if target in text:
    print('Found and replaced')
    text = text.replace(target, replacement)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(text)
else:
    print('Target not found!')
