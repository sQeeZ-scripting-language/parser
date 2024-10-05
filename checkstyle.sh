#!/bin/bash

function format_file {
  local file=$1
  clang-format -i "$file"
}

echo "Applying clang-format to all .cpp and .hpp files..."
for file in $(find . -name '*.cpp' -o -name '*.hpp'); do
  format_file "$file"
done

echo "All files have been formatted."
