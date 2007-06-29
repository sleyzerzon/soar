#ifndef PORTABILITY_WINDOWS_H
#define PORTABILITY_WINDOWS_H

/* This file contains code specific to the windows platforms */

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0400		// This is required since our target is NT4+

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h> // Check for memory leaks
#endif

#include <windows.h>
#include <process.h>
#include <winsock2.h>
#include <direct.h>
#include <Lmcons.h> // for UNLEN constant
#include <malloc.h>
#include <conio.h>
#include <sys/types.h>

// A copy of this file exists in shared. The project exists in Core/pcre
#include <pcreposix.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif

// Map certain functions depending on the OS
#ifndef getcwd // defined in crtdbg.h
#define getcwd _getcwd
#endif
#define chdir _chdir
#define strcasecmp _stricmp
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#ifndef strdup
#define strdup _strdup
#endif

// Use named pipes instead of sockets for same-machine interprocess communication
#define ENABLE_NAMED_PIPES

// FIXME: This is for gSKI. We do need to address the whole sleep issue though.
// Sleep on windows is in milliseconds, hence the multiplication by 1000
#define sys_sleep( seconds )    Sleep( seconds * 1000 )

// This maps some constants to values that can be used on any platform
#ifndef __STDC__  // FIXME: what does this mean?

#define NET_CLOSESOCKET		closesocket
#define ERROR_NUMBER		GetLastError()

#define NET_EWOULDBLOCK		WSAEWOULDBLOCK
#define NET_ENETDOWN		WSAENETDOWN
#define NET_EFAULT			WSAEFAULT
#define NET_ENOTCONN		WSAENOTCONN
#define NET_EINTR			WSAEINTR
#define NET_EINPROGRESS		WSAEINPROGRESS
#define NET_ENETRESET		WSAENETRESET
#define NET_ENOTSOCK		WSAENOTSOCK
#define NET_EOPNOTSUPP		WSAEOPNOTSUPP
#define NET_ESHUTDOWN		WSAESHUTDOWN
#define NET_WOULDBLOCK		WSAEWOULDBLOCK
#define NET_EMSGSIZE		WSAEMSGSIZE
#define NET_EINVAL			WSAEINVAL
#define NET_ECONNABORTED	WSAECONNABORTED
#define NET_ETIMEDOUT		WSAETIMEDOUT
#define NET_ECONNRESET		WSAECONNRESET
#define NET_NOTINITIALISED	WSANOTINITIALISED

#define NET_SD_BOTH			SD_BOTH

#endif // not __STDC__

#endif // PORTABILITY_WINDOWS_H