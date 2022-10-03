#!/usr/bin/bash

# Compute 'src' path
SCRIPT_DIR=$(dirname "$0")
REPO_DIR=$(readlink -f "$SCRIPT_DIR"/..)
DIRS="src include"

# Apply formating to all *.cpp and *.h files
cd "$REPO_DIR"
for FILE in $(find $DIRS -name '*.h' -o -name '*.cpp')
do
  echo $FILE
  clang-format -i -style=file "$FILE"
done
