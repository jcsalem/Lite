Lite
A toolbox for addressable light art.

*** HOW TO RUN ***
+ Setting the LDEV environment variable +
Linux/Mac/MinGW: export LDEV='ck:172.24.22.51/1(72)'
Windows Command Prompt: set LDEV=ck:172.24.22.51/1

+ List of Commands +
Use --help for help on any command.

Ltool: The Multi-tool. Commands: set, all, rotate, rotwash, wash, bounce
Lsparkle: Sparkly lights
Lstarry: Stars
Lfirefly: Fireflies
ckinfo: Reports information on ethernet-attached ColorKinetics fixtures via Kinet protocol.
testmix: tests light mixing

*** CMAKE COMPILATION INSTRUCTIONS ***
+ For all platforms +
cd litedir
mkdir build
cd build
cmake -G "generator name" <options> ..
Then follow instructions below
+ Windows: CodeBlocks +
 Generator name: "CodeBlocks - MinGW Makefiles"
 Within CodeBlocks, open lite.cbp and compile
+ Windows: MinGW +
 [Generator name: "MinGW Makefiles"]
 Start MinGW shell
 cd buildir
 cmake -G "MinGW Makefiles" ..
 mingw32-make
+ Windows: Visual Studio +
 Generator name: "Visual Studio 12" or "Visual Studio 12 Win64"
 (Note that these builds don't include SFML which I found too complex to install.)
 Open lite.sln and compile
+ Linux or MacOS: Unix make +
 Generator name: "Unix Makefiles"
 make
+ Linux or MacOS: CodeBlocks +
 Generator name: "CodeBlocks - Unix Makefiles"
 Within CodeBlocks, open lite.cbp and compile

* CMAKE Command Line Options *
<options> can be
 -DCMAKE_BUILD_TYPE=Debug   This setting is sticky and enables Debug builds

* Linux and Raspberry Pi Notes *
1) Install requirements
     sudo apt-get install ncurses-dev cmake
2) IF WINDOWING DESIRED:
  2A) Install sfml 2.1 or later from the SFML web site.
       Copy files to /usr/local
  2B) Install dependencies
       sudo apt-get install libglew1.5
       sudo apt-get install libjpeg62
3)Build
  cd builddir  # A subdirectory of this one
  cmake -G "Unix Makefiles" ..
  make

* MacOS Notes *
Install SFML package according to instructions. Copy files to /usr/local and to frameworks

* Windows Notes *
Install SFML-2.1 directory at the same level as Lite.
Pick the right version for your compiler. For CodeBlocks/Ming I picked the 32-bit version.

* OBSOLETE: Mac Notes *
Install SFML package according to instructions. Copy files to /usr/local and to frameworks
Compiler Search paths:
 /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/include
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/include/c++/4.2.1
/usr/local/lib

Library Search path:
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/lib

I deleted this library search path as it didn't appear useful. It generated compiler warnings because those libraries weren't compiled with 64 bit x86 in mind.
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk/usr/lib/gcc/i686-apple-darwin11/4.2.1

* OBSOLETE: Codeblocks Compilation Notes *
There are two global variables that must be set every time: extralibs and fileext. CodeBlocks will prompt at the beginning of the session. Look up the appropriate value from the table below. If an entry is blank, enter a space instead.
            fileext    extralibs       sfmllibs
Windows      .exe      -lws2_32        -L../../SFML-2.1/lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s
Mac                    -lncurses       -framework SFML -lsfml-graphics -lsfml-window -lsfml-system
Linux                  -lrt -lncurses  -lsfml-graphics -lsfml-window -lsfml-system
RaspPi                 -lrt -lncurses

