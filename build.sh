#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra"
CURSES_FLAGS="-lcurses"
READLINE_FLAGS="-lreadline"

TARGET="shellcan"
SRC="shellcan.c"

gcc $CFLAGS $CURSES_FLAGS $READLINE_FLAGS -o $TARGET $SRC
