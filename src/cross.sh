#!/bin/sh
#make CXX=i586-mingw32msvc-g++ STRIP=i586-mingw32msvc-strip WINDRES=i586-mingw32msvc-windres PLATFORM=MINGW $*
make CXX=i686-w64-mingw32-g++ STRIP=i686-w64-mingw32-strip WINDRES=i686-w64-mingw32-windres PLATFORM=MINGW $*

