with open('src/ItemScreen.cpp', 'r') as f:
    content = f.read()

bad_str = '''//     float String bid_str = String(bid_text);
        bid_str.replace(",", "");
        bid_amount = bid_str.toDouble();'''

fixed_str = '''//     float bid_amount = atof(bid_text);'''

content = content.replace(bad_str, fixed_str)

with open('src/ItemScreen.cpp', 'w') as f:
    f.write(content)
print("Fixed syntax error")
