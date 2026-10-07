import re

with open("src/main.cpp", "r", encoding="utf-8") as f:
    c = f.read()

c = re.sub(
    r'String firmwareVersion = ".*?";', 
    'String firmwareVersion = "4.5";', 
    c
)

with open("src/main.cpp", "w", encoding="utf-8") as f:
    f.write(c)
