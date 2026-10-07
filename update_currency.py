import re

with open('src/ItemScreen.cpp', 'r') as f:
    content = f.read()

# 1. Replace the default currency $ with Rs
content = content.replace('item["Currency"] | "$"', 'item["Currency"] | "Rs"')

# 2. Replace the format_price function
old_format = '''static void format_price(double price, const char* currency, char* buffer, size_t size) {
    if (currency && strlen(currency) > 0) {
        if (price >= 1000) snprintf(buffer, size, "%s %.0f", currency, price);
        else snprintf(buffer, size, "%s %.2f", currency, price);
    } else snprintf(buffer, size, "%.2f", price);
}'''

new_format = '''static void format_price(double price, const char* currency, char* buffer, size_t size) {
    const char* curr = (currency && strlen(currency) > 0) ? currency : "Rs";
    
    if (price >= 1000000000.0) {
        snprintf(buffer, size, "%s %.1fB", curr, price / 1000000000.0);
    } else if (price >= 1000000.0) {
        snprintf(buffer, size, "%s %.2fM", curr, price / 1000000.0);
    } else if (price >= 1000.0) {
        snprintf(buffer, size, "%s %.1fK", curr, price / 1000.0);
    } else {
        snprintf(buffer, size, "%s %.0f", curr, price);
    }
}'''

if old_format in content:
    content = content.replace(old_format, new_format)
    print("Replaced format_price")
else:
    print("Could not find old format_price block!")

with open('src/ItemScreen.cpp', 'w') as f:
    f.write(content)
