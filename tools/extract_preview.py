#!/usr/bin/env python3
"""Extract webpage_part1 and webpage_part2 from src/webpage.cpp into preview.html for live editing."""
import os
import sys
import re

# Determine project root
if '__file__' in globals():
    base_dir = os.path.dirname(__file__)
    ROOT = os.path.abspath(os.path.join(base_dir, '..'))
else:
    ROOT = os.getcwd()

SOURCE = os.path.join(ROOT, 'src', 'webpage.cpp')
PREVIEW = os.path.join(ROOT, 'preview.html')

if not os.path.exists(SOURCE):
    print(f'ERROR: src/webpage.cpp not found at {SOURCE}')
    sys.exit(1)

with open(SOURCE, 'r', encoding='utf-8') as f:
    content = f.read()

def extract_part(text, name):
    """Extract content between R"rawliteral( and )rawliteral" for a given variable name."""
    pattern = rf'extern const char {name}\[\] PROGMEM = R"rawliteral\((.*?)\)rawliteral";'
    match = re.search(pattern, text, re.DOTALL)
    if not match:
        raise ValueError(f'Could not find {name} in webpage.cpp')
    return match.group(1).strip()

try:
    part1 = extract_part(content, 'webpage_part1')
    part2 = extract_part(content, 'webpage_part2')
except Exception as e:
    print(f'ERROR: {e}')
    sys.exit(1)

# Combine parts
full_html = part1 + '\n' + part2

# Write to preview.html
with open(PREVIEW, 'w', encoding='utf-8') as f:
    f.write(full_html)

print(f'✅ Extracted preview.html from webpage.cpp')
print(f'   Source: {SOURCE}')
print(f'   Output: {PREVIEW}')
print()
print('Now you can:')
print('  1. Open preview.html in VS Code')
print('  2. Right-click → "Show Preview" for live editing')
print('  3. When done, run: python3 tools/embed_preview.py')
