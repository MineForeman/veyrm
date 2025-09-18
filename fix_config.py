#!/usr/bin/env python3

import re

# Read the file
with open('src/config.cpp', 'r') as f:
    content = f.read()

# Fix patterns where we have object variables being checked as if they're not objects
# Pattern: variablename.is_object() && variablename.as_object().contains("key")
# Should become: variablename.contains("key")

content = re.sub(r'(\w+)\.is_object\(\) && \1\.as_object\(\)\.contains\("([^"]+)"\)', r'\1.contains("\2")', content)

# Write back
with open('src/config.cpp', 'w') as f:
    f.write(content)

print("Fixed config.cpp")