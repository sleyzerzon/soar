/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  misc.h
 *
 * =======================================================================
 */

#ifndef MISC_H_
#define MISC_H_

#include <iomanip>
#include <sstream>
#include <string>
#include <cstdio>

// Conversion of value to string
template<class T> std::string& to_string( const T& x, std::string& dest )
{
	static std::ostringstream o;
	
	// get value into stream
	o << std::setprecision( 16 ) << x;
	
	dest.assign( o.str() );
	o.str("");
	return dest;
}

// Conversion from string to value
template <class T> bool from_string( T& val, const std::string& str )
{
	std::istringstream i( str );
	i >> val;
	return !i.fail();
}

// compares two numbers stored as void pointers
// used for qsort calls
template <class T>
T compare_num( const void *arg1, const void *arg2 )
{
    return *( (T *) arg1 ) - *( (T *) arg2 );
}

template <class T> bool from_string( T& val, const char* const pStr )
{
	return from_string( val, std::string( pStr ) );
}

template <class T> inline T cast_and_possibly_truncate( void* ptr )
{
	return static_cast<T>( reinterpret_cast<uintptr_t>( ptr ) );
}

// These functions have proven to be much faster than the c++ style ones above.
// TO
const size_t TO_C_STRING_BUFSIZE = 24; // uint64: 18446744073709551615 plus a few extra
inline const char* const to_c_string( const char& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hhi", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned char& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hhu", v ); 
	return buf; 
}
inline const char* const to_c_string( const short& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hd", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned short& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%hu", v ); 
	return buf; 
}
inline const char* const to_c_string( const int& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%d", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned int& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%u", v ); 
	return buf; 
}
inline const char* const to_c_string( const long int& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%ld", v ); 
	return buf; 
}
inline const char* const to_c_string( const unsigned long& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%lu", v ); 
	return buf; 
}
inline const char* const to_c_string( const float& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%f", v ); 
	return buf; 
}
inline const char* const to_c_string( const double& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%lf", v ); 
	return buf; 
}
inline const char* const to_c_string( const long double& v, char* buf ) 
{ 
	SNPRINTF( buf, TO_C_STRING_BUFSIZE, "%Lf", v ); 
	return buf; 
}

// FROM
inline bool from_c_string( char& v, const char* const str ) 
{ 
	return sscanf( str, "%hhd", &v ) == 1; 
}
inline bool from_c_string( unsigned char& v, const char* const str ) 
{ 
	return sscanf( str, "%hhu", &v ) == 1; 
}
inline bool from_c_string( short& v, const char* const str ) 
{ 
	return sscanf( str, "%hd", &v ) == 1; 
}
inline bool from_c_string( unsigned short& v, const char* const str ) 
{ 
	return sscanf( str, "%hu", &v ) == 1; 
}
inline bool from_c_string( int& v, const char* const str ) 
{ 
	//v = atoi(str);
	return sscanf( str, "%d", &v ) == 1; 
}
inline bool from_c_string( unsigned int& v, const char* const str ) 
{ 
	return sscanf( str, "%u", &v ) == 1; 
}
inline bool from_c_string( long& v, const char* const str ) 
{ 
	//v = atol(str);
	return sscanf( str, "%ld", &v ) == 1; 
}
inline bool from_c_string( unsigned long& v, const char* const str ) 
{ 
	//v = strtoul(str, NULL, 10);
	return sscanf( str, "%lu", &v ) == 1; 
}
inline bool from_c_string( float& v, const char* const str ) 
{
	//v = strtof(str, NULL);
	return sscanf( str, "%f", &v ) == 1; 
}
inline bool from_c_string( double& v, const char* const str ) 
{
	//v = strtod(str, NULL);
	return sscanf( str, "%lf", &v ) == 1; 
}

inline bool from_c_string( long double& v, const char* const str ) 
{
	//v = strtold(str, NULL);
	return sscanf( str, "%Lf", &v ) == 1; 
}

/** Casting between pointer-to-function and pointer-to-object is hard to do... legally
 *
 *  reinterpret_cast<...>(...) isn't actually capable of performing such casts in many
 *  compilers, complaining that it isn't ISO C++.
 *
 *  This function doesn't really do anything to guarantee that the provided types are
 *  pointers, so use it appropriately.
 */

template <typename Goal_Type>
struct Dangerous_Pointer_Cast {
	template <typename Given_Type>
	static Goal_Type from(Given_Type given) {
		union {
			Given_Type given;
			Goal_Type goal;
		} caster;

		caster.given = given;

		return caster.goal;
	}
};

//////////////////////////////////////////////////////////
// STLSoft Timers
//////////////////////////////////////////////////////////

#ifdef WIN32
#include <winstl/performance/performance_counter.hpp>
typedef winstl::performance_counter performance_counter;
#define USE_PERFORMANCE_FOR_BOTH 1
#ifdef USE_PERFORMANCE_FOR_BOTH
typedef winstl::performance_counter processtimes_counter;	// it turns out this has higher resolution
#else // USE_PERFORMANCE_FOR_BOTH
#include <winstl/performance/processtimes_counter.hpp>
typedef winstl::processtimes_counter processtimes_counter;
#endif // USE_PERFORMANCE_FOR_BOTH
#else // WIN32
#include <unixstl/performance/performance_counter.hpp>
#include <unixstl/performance/processtimes_counter.hpp>
typedef unixstl::performance_counter performance_counter;
typedef unixstl::processtimes_counter processtimes_counter;
#endif // WIN32

class soar_timer
{
public:
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void reset() = 0;
	virtual uint64_t get_usec() = 0;

protected:
	soar_timer() {}
	virtual ~soar_timer() {}
};

template <class C>
class soar_timer_impl
	: public soar_timer
{
public:
	soar_timer_impl() {}
	~soar_timer_impl() {}

	void start() { timer.start(); }
	void stop() { timer.stop(); }
	void reset() { start(); stop(); }
	uint64_t get_usec() { return static_cast<uint64_t>(timer.get_microseconds()); }

private:
	C timer;

	soar_timer_impl(const soar_timer_impl&);
	soar_timer_impl& operator=(const soar_timer_impl&);
};

typedef soar_timer_impl<performance_counter> soar_wallclock_timer;
typedef soar_timer_impl<processtimes_counter> soar_process_timer;

class soar_timer_accumulator
{
private:
	uint64_t total;

public:
	soar_timer_accumulator() 
		: total(0) {}

	void reset() { total = 0; }
	void update(soar_timer& timer) { total += timer.get_usec(); }
	double get_sec() { return total / 1000000.0; }
	uint64_t get_usec() { return total; }
	uint64_t get_msec() { return total / 1000; }
};

#endif /*MISC_H_*/
