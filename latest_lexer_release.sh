#!/bin/bash

REPO="sQeeZ-scripting-language/lexer"
REPO_DIR="./lexer-lib"
INCLUDE_DIR="./include/lexer"

if [ -d "$INCLUDE_DIR" ]; then
  rm -rf "$INCLUDE_DIR"
fi
mkdir -p "$INCLUDE_DIR"
mkdir -p "$REPO_DIR"

LATEST_RELEASE=$(curl -s "https://api.github.com/repos/$REPO/releases/latest")

if [[ "$OSTYPE" == "darwin"* ]]; then
  ZIP_URL=$(echo "$LATEST_RELEASE" | grep -o "https://.*sQeeZ-Lexer-macos-.*\.zip" | head -n 1)
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  ZIP_URL=$(echo "$LATEST_RELEASE" | grep -o "https://.*sQeeZ-Lexer-linux-.*\.zip" | head -n 1)
else
  ZIP_URL=$(echo "$LATEST_RELEASE" | grep -o "https://.*sQeeZ-Lexer-windows-.*\.zip" | head -n 1)
fi

if [ -z "$ZIP_URL" ]; then
  echo "No matching asset found."
  exit 1
fi

TEMP_DIR=$(mktemp -d)

curl -L -o "$TEMP_DIR/sQeeZ-Lexer.zip" "$ZIP_URL"

unzip "$TEMP_DIR/sQeeZ-Lexer.zip" -d "$REPO_DIR"

mv "$REPO_DIR/"*/libsQeeZ-Lexer-Lib.a "$REPO_DIR/"
mv "$REPO_DIR/lexer/"* "$INCLUDE_DIR/"

rm -rf "$REPO_DIR/"*/Release/sQeeZ-Lexer-Node.node
rm -rf "$REPO_DIR/"*/sQeeZ-Lexer-Exe
rm -rf "$INCLUDE_DIR/node"
rm -rf "$TEMP_DIR"

find "$REPO_DIR" -type d -empty -delete

echo "Latest files have been downloaded and extracted to $REPO_DIR."