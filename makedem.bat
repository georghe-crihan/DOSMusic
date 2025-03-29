@echo off
set watcom=d:\Holms
set path=%path%;%watcom%\binw
wasm dem137.asm
wlink system dos4g file dem137
