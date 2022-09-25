#!/usr/bin/bash

# Print usage() if no argument given
if [ $# -eq 0 ]
then
    echo "./format_code.sh MY_DIR_TO_FORMAT"
    exit 1
fi

# Retrieve directory to parse
DIR=$1

# Apply formating to all *.cpp and *.h files
for FILE in `find "$DIR" -name '*.h' -o -name '*.cpp'`
do
  echo $FILE
  clang-format -i -style=file "$FILE"
done
