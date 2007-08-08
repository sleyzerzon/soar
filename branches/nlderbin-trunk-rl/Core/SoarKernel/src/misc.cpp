#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

