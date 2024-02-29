@echo off

IF NOT EXIST build mkdir build
pushd build
cl -DDEBUG=1 -Zi ..\source\win32_aberration.cpp user32.lib Gdi32.lib
popd
