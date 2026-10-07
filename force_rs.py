import re

with open('src/ItemScreen.cpp', 'r') as f:
    content = f.read()

old_format = '''static void format_price(double price, const char* currency, char* buffer, size_t size) {
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

new_format = '''static void format_price(double price, const char* currency, char* buffer, size_t size) {
    // Force Rs regardless of what the server sends
    const char* curr = "Rs";
    
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

content = content.replace(old_format, new_format)

with open('src/ItemScreen.cpp', 'w') as f:
    f.write(content)
print("Forced Rs currency.")
