@echo off
set watcom=d:\DOSMusic\watcom
set include=%watcom%\H
set lib=%watcom%\LIB386
set path=%path%;%watcom%\binw
wcc386 dem137.c
:wasm dem137.asm
wlink system dos4g file dem137
