#!/usr/bin/bash

# This script compile the unit test to test the java wrapper.
# This is not integrated in meson because ... this is not so easy.


KIWIX_LIB_JAR=$1
if [ -z $KIWIX_LIB_JAR ]
then
  echo "You must give the path to the kiwixlib.jar as first argument"
  exit 1
fi

KIWIX_LIB_DIR=$2
if [ -z $KIWIX_LIB_DIR ]
then
  echo "You must give the path to directory containing libkiwix.so as second argument"
  exit 1
fi
TEST_SOURCE_DIR=$(dirname $(readlink -f $0))


javac -g -d . -s . -cp $TEST_SOURCE_DIR/junit-4.13.jar:$KIWIX_LIB_JAR $TEST_SOURCE_DIR/test.java

java -Djava.library.path=$KIWIX_LIB_DIR -cp $TEST_SOURCE_DIR/junit-4.13.jar:$TEST_SOURCE_DIR/hamcrest-core-1.3.jar:$KIWIX_LIB_JAR:. org.junit.runner.JUnitCore test

