#!/bin/sh
make CXX=i586-mingw32msvc-g++ STRIP=i586-mingw32msvc-strip WINDRES=i586-mingw32msvc-windres PLATFORM=MINGW $*
