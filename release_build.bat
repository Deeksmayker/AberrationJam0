@echo off

IF NOT EXIST build mkdir build
pushd build
cl -DDEBUG=0 -DPRINT_FPS=0 -O2 ..\source\win32_aberration.cpp user32.lib Gdi32.lib
popd
