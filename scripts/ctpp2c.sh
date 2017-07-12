#!/usr/bin/env bash


ctpp2c=$1
SOURCE=$(pwd)/$2
DEST=$3

$ctpp2c $SOURCE $DEST
