#!/usr/bin/bash

# Compute 'src' path
SCRIPT_DIR=$(dirname "$0")
REPO_DIR=$(readlink -f "$SCRIPT_DIR"/..)
DIRS="$REPO_DIR/src $REPO_DIR/include"

# Apply formating to all *.cpp and *.h files
for FILE in $(find $DIRS -name '*.h' -o -name '*.cpp')
do
  echo $FILE
  clang-format -i -style=file "$FILE"
done
