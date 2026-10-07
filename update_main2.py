import re

with open('src/main.cpp', 'r') as f:
    content = f.read()

battery_logic_old = '''        if (ev.type == EVENT_BATTERY_UPDATE) {
            set_battery_percent(ev.data.battery.percentage);
            // Optionally could store global soc/voltage if needed'''
battery_logic_new = '''        if (ev.type == EVENT_BATTERY_UPDATE) {
            set_battery_percent(ev.data.battery.percentage);
            
            // Low battery logic
            if (ev.data.battery.percentage <= 5 && !lowBatteryWarningShown) {
                show_warning_timeout("\\uF244 Low Battery!", 3000);
                lowBatteryWarningShown = true;
            } else if (ev.data.battery.percentage > 10) {
                lowBatteryWarningShown = false;
            }'''
            
content = content.replace(battery_logic_old, battery_logic_new)

with open('src/main.cpp', 'w') as f:
    f.write(content)
print("Added low battery logic back to processI2CQueue")
