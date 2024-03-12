@echo off

IF NOT EXIST build mkdir build
pushd build
cl -DDEBUG=1 -DPRINT_FPS=0 -Zi ..\source\win32_aberration.cpp user32.lib Gdi32.lib Winmm.lib
popd
