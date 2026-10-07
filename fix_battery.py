with open("src/main.cpp", "r", encoding="utf-8") as f:
    c = f.read()

# Replace updateBattery entirely
old_update_battery = """void updateBattery() {
    if (lastBatteryUpdate == 0 || millis() - lastBatteryUpdate > BATTERY_INTERVAL) {
        lastBatteryUpdate = millis();
        float soc = battery.readSOC();
        float voltage = battery.readVoltage();
        
        uint8_t pct = (uint8_t)battery.readPercent();
        set_battery_percent(pct);
        
        Serial.print("Battery Voltage: "); Serial.print(voltage);
        Serial.print("V, Raw SOC: "); Serial.print(soc);
        Serial.print("%, Displayed Pct: "); Serial.print(pct); Serial.println("%");

        if (soc <= 20.0 && voltage > 1.0) {
            Serial.println("Battery SOC is <= 20%! Shutting down...");
            show_custom_loading("Battery Low!\nPowering off...");
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

        if (soc < 30.0 && soc > 20.0) {
            set_battery_color(lv_color_hex(0xFF0000));
            if (!lowBatteryWarningShown && millis() > 5000) {
                show_warning_timeout("Low Battery!", 3000);
                lowBatteryWarningShown = true;
            }
        } else if (soc >= 30.0) {
            set_battery_color(lv_color_white());
            lowBatteryWarningShown = false;
        }
    }
}"""

new_update_battery = """void updateBattery() {
    // Battery is now updated via I2CQueue polling to prevent I2C bus collisions.
}"""
c = c.replace(old_update_battery, new_update_battery)

# Replace processI2CQueue logic
old_process_queue = """void processI2CQueue() {
    I2CEvent ev;
    while(xQueueReceive(i2cEventQueue, &ev, 0)) {
        if (ev.type == EVENT_BATTERY_UPDATE) {
            set_battery_percent(ev.data.battery.percentage);
            
            // Low battery logic
            if (ev.data.battery.percentage <= 5 && !lowBatteryWarningShown) {
                show_warning_timeout("\uF244 Low Battery!", 3000);
                lowBatteryWarningShown = true;
            } else if (ev.data.battery.percentage > 10) {
                lowBatteryWarningShown = false;
            }
        } else if (ev.type == EVENT_KEYPAD_PRESS) {"""

new_process_queue = """void processI2CQueue() {
    I2CEvent ev;
    while(xQueueReceive(i2cEventQueue, &ev, 0)) {
        if (ev.type == EVENT_BATTERY_UPDATE) {
            set_battery_percent(ev.data.battery.percentage);
            
            if (ev.data.battery.percentage <= 5 && ev.data.battery.voltage > 1.0) {
                Serial.println("Battery <= 5%! Shutting down...");
                show_custom_loading("Power Low!\nShutting down...");
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
        } else if (ev.type == EVENT_KEYPAD_PRESS) {"""

c = c.replace(old_process_queue, new_process_queue)

with open("src/main.cpp", "w", encoding="utf-8") as f:
    f.write(c)
