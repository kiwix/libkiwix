#!/usr/bin/env bash

set -e

BUILD_PATH=$(pwd)

echo "javac -d $BUILD_PATH/src/android $@"
javac -d $BUILD_PATH/src/android "$@"


cd $BUILD_PATH/src/android
echo "javah -jni org.kiwix.kiwixlib.JNIKiwix"
javah -jni org.kiwix.kiwixlib.JNIKiwix
cd $BUILD_PATH
