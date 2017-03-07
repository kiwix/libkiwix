#!/usr/bin/env bash

set -e

BUILD_PATH=$(pwd)

javac -d $BUILD_PATH/src/android $1 $2 $3 $4

cd $BUILD_PATH/src/android
javah -jni org.kiwix.kiwixlib.JNIKiwix
cd $BUILD_PATH
