import re

with open("src/OTAUpdate.h", "r", encoding="utf-8") as f:
    c = f.read()

c = re.sub(
    r"#define CURRENT_FIRMWARE_VERSION \d+\.\d+", 
    "#define CURRENT_FIRMWARE_VERSION 4.5", 
    c
)

with open("src/OTAUpdate.h", "w", encoding="utf-8") as f:
    f.write(c)
