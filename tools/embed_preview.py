#!/usr/bin/env python3
"""Embed preview.html into src/webpage.cpp as webpage_part1/part2.
Splits the preview into two roughly equally sized parts to match the project's pattern.
"""
import os
import sys
from datetime import datetime

# Determine project root robustly — PlatformIO may execute scripts without __file__ defined
if '__file__' in globals():
    base_dir = os.path.dirname(__file__)
    ROOT = os.path.abspath(os.path.join(base_dir, '..'))
else:
    # When run by PlatformIO, cwd is already the project root
    ROOT = os.getcwd()
PREVIEW = os.path.join(ROOT, 'preview.html')
TARGET = os.path.join(ROOT, 'src', 'webpage.cpp')
BACKUP = TARGET + '.bak.' + datetime.now().strftime('%Y%m%d%H%M%S')

if not os.path.exists(PREVIEW):
    print('preview.html not found at', PREVIEW, '- skipping embed (not fatal).')
    sys.exit(0)
if not os.path.exists(TARGET):
    print('target src/webpage.cpp not found at', TARGET, '- skipping embed (not fatal).')
    sys.exit(0)

with open(PREVIEW, 'r', encoding='utf-8') as f:
    content = f.read()

# Ensure a UI version comment exists at top
if '<!-- UI version:' not in content:
    content = '<!-- UI version: {} -->\n'.format(datetime.now().strftime('%Y-%m-%d')) + content

# Split content near midpoint on a newline boundary
mid = len(content) // 2
split_at = content.rfind('\n', 0, mid)
if split_at == -1:
    split_at = mid
part1 = content[:split_at].rstrip() + '\n'
part2 = content[split_at:].lstrip() + '\n'

with open(TARGET, 'r', encoding='utf-8') as f:
    target_text = f.read()

# Backup
with open(BACKUP, 'w', encoding='utf-8') as f:
    f.write(target_text)
print('Backup written to', BACKUP)

# Replacement helper
def replace_part(text, name, new_html):
    start_marker = f'extern const char {name}[] PROGMEM = R"rawliteral('
    end_marker = ')rawliteral";'
    si = text.find(start_marker)
    if si == -1:
        raise ValueError(f'start marker for {name} not found')
    si_end = si + len(start_marker)
    ei = text.find(end_marker, si_end)
    if ei == -1:
        raise ValueError(f'end marker for {name} not found')
    new_block = start_marker + '\n' + new_html + '\n' + end_marker
    return text[:si] + new_block + text[ei+len(end_marker):]

# Perform replacements
try:
    updated = replace_part(target_text, 'webpage_part1', part1)
    updated = replace_part(updated, 'webpage_part2', part2)
except Exception as e:
    print('Error replacing parts:', e)
    sys.exit(2)

with open(TARGET, 'w', encoding='utf-8') as f:
    f.write(updated)

print('Embedded preview.html into src/webpage.cpp successfully.')
print('Please run `pio run -e esp32s3` to build and `pio run --target upload -e esp32s3` to upload.')
