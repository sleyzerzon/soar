#ifndef PORTABILITY_H
#define PORTABILITY_H

/* portability.h
 * Author: Jonathan Voigt (voigtjr@gmail.com)
 * Date: April 2007
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>

#ifdef SCONS_POSIX
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#endif // SCONS_POSIX

#include <assert.h>
#include <time.h>

#ifdef SCONS_NT
#include <windows.h>
#include <direct.h>
//
// Visual Studio 2005 requires these:
#define getcwd _getcwd
#define chdir _chdir

#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif

#endif // PORTABILITY_H
