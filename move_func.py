import re

with open("src/main.cpp", "r", encoding="utf-8") as f:
    c = f.read()

# Extract processI2CQueue
process_match = re.search(r"void processI2CQueue\(\) \{.*?\}\n\}\n", c, re.DOTALL)
if process_match:
    func_text = process_match.group(0)
    c = c.replace(func_text, "")
    # Insert it before void setup()
    c = c.replace("void setup() {", func_text + "\nvoid setup() {")

with open("src/main.cpp", "w", encoding="utf-8") as f:
    f.write(c)
