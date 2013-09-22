// Standard Compiler macros
// _WIN32  - Windows 32-bit or 64-bit
//
// For a full list, see http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// pr http://sourceforge.net/p/predef/wiki/OperatingSystems/
//

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

// OS-specific ones
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
// Feature settings
//----------------------------------------------------------------------------

//#define HAS_SFML

#ifndef OS_WINDOWS
//#ifdef __arm__
#define HAS_GPIO
#endif


//----------------------------------------------------------------------------
// Windows Specific
//----------------------------------------------------------------------------
//*** Visual C++ 6.0			                _MSC_VER is 1200
//*** Visual Studio .NET (Visual C++ 7.0)		_MSC_VER is 1300
//*** Visual Studio .NET 2003 (Visual C++ 7.1)	_MSC_VER is 1310
//*** Visual Studio .NET 2005 (Visual C++ 8.0)	_MSC_VER is 1400
//
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
