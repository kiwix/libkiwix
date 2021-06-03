#!/usr/bin/bash

files=(
"include/library.h"
"include/common/stringTools.h"
"include/common/pathTools.h"
"include/common/otherTools.h"
"include/common/regexTools.h"
"include/common/networkTools.h"
"include/common/archiveTools.h"
"include/manager.h"
"include/reader.h"
"include/kiwix.h"
"include/xapianSearcher.h"
"include/searcher.h"
"src/library.cpp"
"src/android/kiwix.cpp"
"src/android/org/kiwix/kiwixlib/JNIKiwixBool.java"
"src/android/org/kiwix/kiwixlib/JNIKiwix.java"
"src/android/org/kiwix/kiwixlib/JNIKiwixString.java"
"src/android/org/kiwix/kiwixlib/JNIKiwixInt.java"
"src/searcher.cpp"
"src/common/pathTools.cpp"
"src/common/regexTools.cpp"
"src/common/otherTools.cpp"
"src/common/archiveTools.cpp"
"src/common/networkTools.cpp"
"src/common/stringTools.cpp"
"src/xapianSearcher.cpp"
"src/manager.cpp"
"src/reader.cpp"
)

for i in "${files[@]}"
do
  echo $i
  clang-format -i -style=file $i
done
