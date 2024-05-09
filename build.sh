#!/bin/bash

DF_DIR=../deadfrog-lib/src
g++ -I$DF_DIR -o grapher.exe -g -D_WIN32 -Os \
    main.cpp \
    $DF_DIR/df_bitmap.cpp \
    $DF_DIR/df_bmp.cpp \
    $DF_DIR/df_colour.cpp \
    $DF_DIR/df_common.cpp \
    $DF_DIR/df_font.cpp \
    $DF_DIR/df_message_dialog.cpp \
    $DF_DIR/fonts/df_mono.cpp \
    $DF_DIR/fonts/df_prop.cpp \
    $DF_DIR/df_time.cpp \
    $DF_DIR/df_window.cpp -ldwmapi -lgdi32
