* Codeblocks Compilation Notes *
There are two global variables that must be set every time: extralibs and fileext. CodeBlocks will prompt at the beginning of the session. Look up the appropriate value from the table below. If an entry is blank, enter a space instead.
             fileext     extralibs
Windows       .exe
Mac                       -lncurses -framework SFML -lsfml-graphics -lsfml-window -lsfml-system
Linux                     -lrt -lncurses -lsfml-graphics -lsfml-window -lsfml-system
RaspPi                    -lrt -lncurses

* Setting the LDEV environment variable *
Linux/Mac: export LDEV='ck:172.24.22.51/1(72)'
Windows: set LDEV=ck:172.24.22.51/1

* Linux and Raspberry Pi Notes *
1) sudo apt-get install ncurses-dev
2) Install sfml 2.1 or later from the SFML web site.
   Copy files to /usr/local
3) Install dependencies
sudo apt-get install libglew1.5
sudo apt-get install libjpeg62

* Mac Notes *
Install SFML package according to instructions. Copy files to /usr/local and to frameworks
Compiler Search paths:
 /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/include
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/include/c++/4.2.1
/usr/local/lib

Library Search path:
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/lib

I deleted this library search path as it didn't appear useful. It generated compiler warnings because those libraries weren't compiled with 64 bit x86 in mind.
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/lib/gcc/i686-apple-darwin11/4.2.1
