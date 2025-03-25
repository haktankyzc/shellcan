#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra"
CURSES_FLAGS="-lcurses"

TARGET="shellcan"
SRC="shellcan.c"


gcc $CFLAGS $CURSES_FLAGS -o $TARGET $SRC
