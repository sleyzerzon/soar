#ifndef _WINX_SYS_TIMEX_H_
#define _WINX_SYS_TIMEX_H_

#include <features.h>

#ifdef _TIMEVAL_DEFINED
# define _STRUCT_TIMEVAL     1
#endif /* _TIMEVAL_DEFINED */
#include <bits/types.h>
#define __need_time_t
#include <time.h>
#define __need_timeval
#include <bits/time.h>

#ifdef _TIMEVAL_DEFINED /* also in winsock[2].h */
# undef __TIMEVAL__ 
# define __TIMEVAL__ 1
# undef _STRUCT_TIMEVAL
# define _STRUCT_TIMEVAL     1
#endif /* _TIMEVAL_DEFINED */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __USE_GNU
/* Macros for converting between `struct timeval' and `struct timespec'.  */
# define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
        (ts)->tv_sec = (tv)->tv_sec;                                    \
        (ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
# define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
        (tv)->tv_sec = (ts)->tv_sec;                                    \
        (tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}
#endif

#ifdef __USE_BSD
/* Structure crudely representing a timezone.
   This is obsolete and should never be used.  */
struct timezone
  {
    int tz_minuteswest;       /* Minutes west of GMT.  */
    int tz_dsttime;      /* Nonzero if DST is ever in effect.  */
  };

typedef struct timezone *__restrict __timezone_ptr_t;
#else
typedef void *__restrict __timezone_ptr_t;
#endif

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.
   NOTE: This form of timezone information is obsolete.
   Use the functions and variables declared in <time.h> instead.  */
extern int gettimeofday (struct timeval *__restrict __tv,
                __timezone_ptr_t __tz) __THROW;

extern int getntptimeofday (struct timespec *__restrict __tp,
                __timezone_ptr_t __tz) __THROW;

#ifdef __USE_BSD
/* Set the current time of day and timezone information.
   This call is restricted to the super-user.  */
extern int settimeofday (__const struct timeval *__tv,
                __const struct timezone *__tz) __THROW;
extern int setntptimeofday (__const struct timespec *__tp,
                __const struct timezone *__tz) __THROW;
#endif

///* Values for the first argument to `getitimer' and `setitimer'.  */
//enum __itimer_which
//  {
//    /* Timers run in real time.  */
//    ITIMER_REAL = 0,
//#define ITIMER_REAL ITIMER_REAL
//    /* Timers run only when the process is executing.  */
//    ITIMER_VIRTUAL = 1,
//#define ITIMER_VIRTUAL ITIMER_VIRTUAL
//    /* Timers run when the process is executing and when
//       the system is executing on behalf of the process.  */
//    ITIMER_PROF = 2
//#define ITIMER_PROF ITIMER_PROF
//  };

/* Type of the second argument to `getitimer' and
   the second and third arguments `setitimer'.  */
struct itimerval
  {
    /* Value to put into `it_value' when the timer expires.  */
    struct timeval it_interval;
    /* Time to the next timer expiration.  */
    struct timeval it_value;
  };

#if defined __USE_GNU && !defined __cplusplus
/* Use the nicer parameter type only in GNU mode and not for C++ since the
   strict C++ rules prevent the automatic promotion.  */
typedef enum __itimer_which __itimer_which_t;
#else
typedef int __itimer_which_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /*_WINX_SYS_TIMEX_H_*/
