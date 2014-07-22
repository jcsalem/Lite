// Standard Compiler macros
// _WIN32  - Windows 32-bit or 64-bit
//
// For a full list, see http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// pr http://sourceforge.net/p/predef/wiki/OperatingSystems/
//

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

//----------------------------------------------------------------------------
// Feature Settings
//----------------------------------------------------------------------------

// Uncomment this to disable SFML.
// Instructions for compiling with SFML are in the README.txt file.
//#define HAS_SFML 0

//----------------------------------------------------------------------------
// OS-specific defines
//----------------------------------------------------------------------------

#ifdef _WIN32  // Windows 32 or 64 bit
#define OS_WINDOWS
#endif

#ifdef __APPLE__ // MacOS
#define OS_MAC
#endif

#ifdef __AVR__
#define OS_ARDUINO
#endif

#ifdef __linux
#define OS_LINUX
#endif

//----------------------------------------------------------------------------
// Feature defaults
//----------------------------------------------------------------------------

//#ifndef HAS_SFML
//#ifndef __arm__
//#define HAS_SFML 1

//#ifdef OS_WINDOWS
//// Force static linking on Windows
//#define SFML_STATIC
//#endif

//#endif // __arm__
//#endif // HAS_SFML

#ifndef HAS_GPIO
#ifndef OS_WINDOWS
#ifndef OS_MAC
#define HAS_GPIO 1
#endif
#endif
#endif 

//----------------------------------------------------------------------------
// Windows Specific
//----------------------------------------------------------------------------
// MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
// MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
// MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
// MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
// MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
// MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
// MSVC++ 7.0  _MSC_VER == 1300 (Visual Studio .Net)
// MSVC++ 6.0  _MSC_VER == 1200 (Visual C++ 6.0)
// MSVC++ 5.0  _MSC_VER == 1100
#ifdef _MSC_VER
// Disable spurious "possible loss of data" and truncation from double to float warnings
#pragma warning (disable: 4244 4267 4305)
#endif // _MSC_VER

// Windows versions:
//*** Windows 2000:         WINVER is 0x0500
//*** Windows 2003/XP:      WINVER is 0x0501
//*** Windows 2008/Vista:   WINVER is 0x0600
//*** Windows 7:            WINVER is 0x0700

// Force minimal Windows.h (if it's later included)
#ifdef OS_WINDOWS
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#endif

//----------------------------------------------------------------------------
// Other compiler info
//----------------------------------------------------------------------------

// Intel Compiler: __INTEL_COMPILER
// GCC: __GNUC__

#endif // CONFIG_H_INCLUDED
