#!/usr/bin/env python3

import re
import os
import glob

def fix_boost_json_file(filepath):
    """Fix Boost.JSON API usage in a file"""
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    original_content = content

    # Fix json variable declarations to use boost::json::object
    # Pattern: json varname; (where we know it's used as object)
    content = re.sub(r'\bjson\s+(\w+);\s*\n(\s*)\1\["', r'boost::json::object \1;\n\2\1["', content)

    # Fix array creation
    content = re.sub(r'\bjson::array\(\)', r'boost::json::array()', content)
    content = re.sub(r'\bboost::json::value::array\(\)', r'boost::json::array()', content)

    # Fix object creation
    content = re.sub(r'\bjson::object\(\)', r'boost::json::object()', content)
    content = re.sub(r'\bboost::json::value::object\(\)', r'boost::json::object()', content)

    # Fix .dump() to boost::json::serialize()
    content = re.sub(r'(\w+)\.dump\((\d+)\)', r'boost::json::serialize(\1)', content)

    # Fix .contains() calls - need to check if it's an object first
    # This is a simple fix - more complex cases might need manual handling
    content = re.sub(r'(\w+)\.contains\("([^"]+)"\)', r'\1.is_object() && \1.as_object().contains("\2")', content)

    # Fix array access like json[index] to json.as_array()[index]
    # This is tricky - we'll need to be more careful

    # Fix object access like json["key"] to json.as_object().at("key") for reading
    # This is complex and might need manual handling

    # Save if changed
    if content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed: {filepath}")
        return True
    return False

def main():
    # Find all C++ files
    cpp_files = []
    for pattern in ['src/**/*.cpp', 'include/**/*.h', 'tests/**/*.cpp']:
        cpp_files.extend(glob.glob(pattern, recursive=True))

    fixed_count = 0
    for filepath in cpp_files:
        if fix_boost_json_file(filepath):
            fixed_count += 1

    print(f"Fixed {fixed_count} files")

if __name__ == '__main__':
    main()