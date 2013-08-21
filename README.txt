Codeblocks Compilation Notes
There are two global variables that must be set every time: extralibs and fileext. CodeBlocks will prompt at the beginning of the session. Look up the appropriate value from the table below. If an entry is blank, enter a space instead.
             extralibs         fileext
Windows                         .exe
M            -lncurses     
Linux        -lncurses                         ???
RaspPi       -lncurses                         ???

Mac Notes
Compiler Search paths:
 /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/include
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/include/c++/4.2.1
Library Search path:
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/lib

I deleted this library search path as it didn't appear useful. It generated compiler warnings because those libraries weren't compiled with 64 bit x86 in mind.
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/lib/gcc/i686-apple-darwin11/4.2.1