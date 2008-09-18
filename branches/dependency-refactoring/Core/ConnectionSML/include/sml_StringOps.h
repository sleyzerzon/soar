/////////////////////////////////////////////////////////////////
// StringOps
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : July 2004
//
// It's often useful to abstract over the string operations,
// in case a particular library fails to provide the normal implementation
// or we decide to switch functions later (e.g. from case sensitive to
// case insensitive matching).
//
/////////////////////////////////////////////////////////////////

#ifndef STRING_OPS_H
#define STRING_OPS_H

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

namespace sml {

/*************************************************************
* @brief Returns true if strings are equal (case sensitive).
*************************************************************/
inline bool IsStringEqual( char const* pStr1, char const* pStr2 )
{
	//return pStr1 && pStr2 && std::string( pStr1 ) == pStr2;
	return boost::equals(pStr1, pStr2);
}
inline bool IsStringEqual( char const* pStr1, const std::string& str2 ) 
{
	//return pStr1 && str2 == pStr1;
	return boost::equals(pStr1, str2);
}
inline bool IsStringEqual( const std::string& str1, char const* pStr2 ) 
{
	//return pStr2 && str1 == pStr2;
	return boost::equals(str1, pStr2);
}
inline bool IsStringEqual( const std::string& str1, const std::string& str2 ) 
{
	return str1 == str2;
}

/*************************************************************
* @brief Returns true if strings are equal (case insensitive).
*************************************************************/
inline bool IsStringEqualIgnoreCase( std::string str1, std::string str2 ) 
{
	return boost::iequals(str1, str2);
	//boost::algorithm::to_lower( str1 );
	//boost::algorithm::to_lower( str2 );
	//return str1 == str2;
}

/*************************************************************
* @brief Returns a copy of the string.
*		 Some libraries may not have strdup().  If so we
*		 can fix it here.
*************************************************************/
char* StringCopy(char const* pStr) ;

/*************************************************************
* @brief Deletes copied strings
*************************************************************/
void StringDelete(char* pStr) ;

/*************************************************************
* @brief A utility function, splits a command line into argument
*		 tokens and stores them in the argumentVector string.
*************************************************************/
int Tokenize(std::string cmdline, std::vector<std::string>& argumentVector);

/*************************************************************
* @brief Trim comments off of a line (for command parsing)
* @return true on success, false if there is a new-line before a pipe quotation ends
*************************************************************/
bool Trim(std::string& line);

}

#endif
