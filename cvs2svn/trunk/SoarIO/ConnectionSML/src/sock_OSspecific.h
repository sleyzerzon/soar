/////////////////////////////////////////////////////////////////
// OSSpecific class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// The parts of the socket code that are specific to the operating system.
// 
/////////////////////////////////////////////////////////////////

#ifndef CT_OS_SPECIFIC_H
#define CT_OS_SPECIFIC_H

#ifdef HAVE_CONFIG_H
#include "config.h"

#ifndef HAVE_STRCASECMP
#error "missing required strcasecmp function"
#endif // HAVE_STRCASECMP

#endif // HAVE_CONFIG_H

#include "sock_SocketHeader.h"	// For SOCKET definition

namespace sock {

bool InitializeOperatingSystemSocketLibrary() ;
bool TerminateOperatingSystemSocketLibrary() ;
bool MakeSocketNonBlocking(SOCKET hSock) ;
bool SleepMillisecs(long msecs) ;

} // Namespace

// Map certain functions depending on the OS
#ifdef _WIN32
#define STRICMP	   stricmp
#define VSNSPRINTF _vsnprintf
#else
#define STRICMP    strcasecmp
#define VSNSPRINTF vsnprintf
#endif

#endif // CT_OS_SPECIFIC_H

