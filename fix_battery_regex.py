import re

with open("src/main.cpp", "r", encoding="utf-8") as f:
    c = f.read()

# Replace updateBattery
c = re.sub(
    r"void updateBattery\(\) \{.*?\}\n\}", 
    "void updateBattery() {\n    // Battery is now updated via I2CQueue polling to prevent I2C bus collisions.\n}", 
    c, 
    flags=re.DOTALL
)

# Replace processI2CQueue
process_match = re.search(r"void processI2CQueue\(\) \{.*?if \(ev\.type == EVENT_BATTERY_UPDATE\) \{.*?\n        \} else if \(ev\.type == EVENT_KEYPAD_PRESS\)", c, re.DOTALL)
if process_match:
    old_code = process_match.group(0)
    new_code = """void processI2CQueue() {
    I2CEvent ev;
    while(xQueueReceive(i2cEventQueue, &ev, 0)) {
        if (ev.type == EVENT_BATTERY_UPDATE) {
            set_battery_percent(ev.data.battery.percentage);
            
            if (ev.data.battery.percentage <= 5 && ev.data.battery.voltage > 1.0) {
                Serial.println("Battery <= 5%! Shutting down...");
                show_custom_loading("Power Low!\\nShutting down...");
                lv_timer_handler();
                
                ledcSetup(0, 2000, 8);
                ledcAttachPin(buzzer, 0);
                ledcWriteTone(0, 1500);
                delay(60);
                ledcWriteTone(0, 800);
                delay(100);
                ledcWriteTone(0, 0);
                ledcDetachPin(buzzer);
                pinMode(buzzer, INPUT); 
                
                delay(2000); 
                digitalWrite(Latch_ON, LOW);
                while(true) delay(100);
            }

            if (ev.data.battery.percentage < 15 && ev.data.battery.percentage > 5) {
                set_battery_color(lv_color_hex(0xFF0000));
            } else if (ev.data.battery.percentage >= 15) {
                set_battery_color(lv_color_white());
            }
        } else if (ev.type == EVENT_KEYPAD_PRESS)"""
    c = c.replace(old_code, new_code)
else:
    print("Failed to match processI2CQueue")

with open("src/main.cpp", "w", encoding="utf-8") as f:
    f.write(c)
