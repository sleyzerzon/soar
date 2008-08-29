#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  misc.cpp
 *
 * =======================================================================
 * Description  :  Misc functions that don't seem to fit elsewhere
 * =======================================================================
 */

#include <string>

#include "misc.h"

/***************************************************************************
 * Function     : is_natural_number
 **************************************************************************/
bool is_natural_number( std::string *str )
{
	const std::string nums = "0123456789";
	
	return ( str->find_first_not_of( nums ) == std::string::npos );
}

/***************************************************************************
 * Function     : string_multi_copy
 **************************************************************************/
std::string *string_multi_copy( const char *src, unsigned int n )
{
	int len = strlen( src );
	int i, pos;
	const char *runner;
	
	std::string *return_val = new std::string( len * n, ' '  );
	pos = 0;
	for ( i=0; i<n; i++ )
	{
		runner = src;
		while ( (*runner) != '\0' )
		{
			(*return_val)[ pos++ ] = (*runner);
			runner++;
		}
	}

	return return_val;
}

/***************************************************************************
 * Function     : get_number_from_symbol
 **************************************************************************/
double get_number_from_symbol( Symbol *sym )
{
	if ( sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE )
		return sym->fc.value;
	else if ( sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
		return (double) sym->ic.value;
	
	return 0.0;
}