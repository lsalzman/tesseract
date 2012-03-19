* Installing and Running Sauerbraten

*nix versions of Sauerbraten clients and standalone servers.
The clients function identical to the win32 client, see config.html for more
information about command-line options if necessary.

Please run "sauerbraten_unix" from the root Sauerbraten dir (NOT from inside the "bin_unix"
directory) to launch these, or set the SAUER_DIR variable at the top of the "sauerbraten_unix" 
script to an absolute path to allow it to be run from any location. Note that the "sauerbraten_unix" 
script is set up to write any files (saved maps, configs, etc.) into the user's home 
directory at "~/.sauerbraten".

Clients will need the following dynamic link libraries present:
* libGL (OpenGL)
* SDL (>= 1.2.10)
* SDL_image
* SDL_mixer
* libpng
* libjpeg
* zlib

If native binaries for your platform are not included, then try the following:
1) Ensure you have the DEVELOPMENT VERSIONS of the above libraries installed.
2) Change to the src/ directory and type "make install".
3) Re-run the "sauerbraten_unix" script from the root Sauerbraten directory if it succeeded.

The servers (bin_unix/linux_server or bin_unix/native_server) should need no libs 
other than libstdc++ and zlib. Note that for the server to see the "server-init.cfg", 
it must be run from whichever directory "server-init.cfg" is in. If you run a server
with the "sauerbraten_unix -d" command, this happens automatically. However, if you
wish to run the standalone servers instead, then you may need to write an appropriate
wrapper script to change to the appropriate data directory before running the standalone
server binary, as described below in the packaging guide.





* Packaging Guide for Unix-like Operating System Developers/Maintainers

If you are packaging Sauerbraten up for redistribution in a Linux distribution or other 
similar OS, please avoid using the "sauerbraten_unix" script in its default/unmodified form.
You should at least set the SAUER_DATA variable to appropriately find the common Sauerbraten
data files, or better yet replace it with a more appropriate way of starting Sauerbraten using
the script as a basis. If the distribution happens to place program binaries in a specific
directory separate from data files, such as "/usr/bin", then much of the lines within the script
that deal with finding an appropriate binary can be removed, as they only exist to help people
running from the original Sauerbraten tarball. An example run script is shown further
below.

Also, please note, that if your distribution chooses to package the binaries and game data
as separate packages due to whatever licensing rules or cross-architecture package sharing, 
that client binaries from newer Sauerbraten editions are NOT compatible with older versions of 
game data, on whole, nor obviously compatible with newer versions of game data. Game data, as a 
whole, and client binaries are only roughly compatible within individual Sauerbraten editions,
though patches to individual Sauerbraten editions generally always retain compatibility with
the game data.

For those writing custom Sauerbraten run scripts for packaging, they should adhere to the following
guidelines:

Sauerbraten finds all game files relative to the current directory from which Sauerbraten is run, 
so any script running Sauerbraten should set its current directory to where the Sauerbraten data 
files are located before it runs the Sauerbraten client binaries. No silly symlinking tricks should 
be at all necessary.

When running the Sauerbraten client, one command-line switch should ALWAYS be supplied to
the client binary. This is "-q${HOME}/.sauerbraten", which will instruct Sauerbraten to
write any user private files such as saved maps and configurations to a private ".sauerbraten" 
directory within each user's home directory. Sauerbraten will automatically create this
directory and any subdirectories for each user running it, so do not pre-create this directory 
or install any symlinks within it - as some Linux distribution packages have erroneously done. 
All command-line switches supplied to the Sauerbraten run script should be passed to the 
Sauerbraten client after the "-q${HOME}/.sauerbraten" switch.

A simple script such as the following (with directory/file names set as appropriate) would 
ultimately suffice for the client:

#!/bin/sh
SAUER_DATA=/usr/share/games/sauerbraten
SAUER_BIN=/usr/bin/sauerbraten_client
SAUER_OPTIONS="-q${HOME}/.sauerbraten"

cd ${SAUER_DATA}
exec ${SAUER_BIN} ${SAUER_OPTIONS} "$@"

A simple script for the server, which assumes a global default "server-init.cfg" in SAUER_DATA,
but allows per-user overriding via the home directory, might be:

#!/bin/sh
SAUER_DATA=/usr/share/games/sauerbraten
SAUER_SERV_BIN=/usr/bin/sauerbraten_server
SAUER_SERV_OPTIONS="-q${HOME}/.sauerbraten"

cd ${SAUER_DATA}
exec ${SAUER_SERV_BIN} ${SAUER_SERV_OPTIONS} "$@"

With respect to libraries, make sure that you do not link Sauerbraten against any other ENet package
than the one that comes included with the Sauerbraten, as it may be different from the official ENet
releases and might fail to compile or communicate properly.

