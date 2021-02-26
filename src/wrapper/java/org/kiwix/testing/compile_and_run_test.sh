#!/usr/bin/bash

# This script compiles and runs the unit test to test the java wrapper.
# This is not integrated in meson because ... this is not so easy.

die()
{
  echo >&2 "!!! ERROR: $*"
  exit 1
}


KIWIX_LIB_JAR=$1
if [ -z $KIWIX_LIB_JAR ]
then
  die "You must give the path to the kiwixlib.jar as first argument"
fi

KIWIX_LIB_DIR=$2
if [ -z $KIWIX_LIB_DIR ]
then
  die "You must give the path to directory containing libkiwix.so as second argument"
fi

KIWIX_LIB_JAR=$(readlink -f "$KIWIX_LIB_JAR")
KIWIX_LIB_DIR=$(readlink -f "$KIWIX_LIB_DIR")
TEST_SOURCE_DIR=$(dirname "$(readlink -f $0)")

cd "$TEST_SOURCE_DIR"

javac -g -d . -s . -cp "junit-4.13.jar:$KIWIX_LIB_JAR" test.java \
  || die "Compilation failed"

java -Djava.library.path="$KIWIX_LIB_DIR" \
     -cp "junit-4.13.jar:hamcrest-core-1.3.jar:$KIWIX_LIB_JAR:." \
     org.junit.runner.JUnitCore test \
  || die "Unit test failed"
