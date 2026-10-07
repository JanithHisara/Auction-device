import re

with open("src/main.cpp", "r", encoding="utf-8") as f:
    c = f.read()

c = c.replace("while(true) delay(100);", "esp_deep_sleep_start();")

with open("src/main.cpp", "w", encoding="utf-8") as f:
    f.write(c)
