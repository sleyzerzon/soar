/////////////////////////////////////////////////////////////////
// RhsFunction class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2007
//
// Support for right hand side functions.
/////////////////////////////////////////////////////////////////

#include "sml_RhsFunction.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "KernelHeaders.h"

using namespace sml ;

// This method is called by the kernel, which in turn calls the Execute method of the RhsFunction.
Symbol* RhsFunction::RhsFunctionCallback(agent* thisAgent, list* args, void* user_data)
{
	// Since we registered this callback, we know what the user data is.
	RhsFunction* rhsFunction = static_cast<RhsFunction*>(user_data);

	// Prepare arguments

	// List of symbols wrapped in gSymbols
	std::vector<Symbol*> symVector;
	for(; args != NIL; args = args = args->rest)
		symVector.push_back(static_cast<Symbol*>(args->first));

	Symbol* pSoarReturn = 0;

	// Check to make sure we have the right number of arguments.   
	if( (rhsFunction->GetNumExpectedParameters() == kPARAM_NUM_VARIABLE) ||
	  ((int)symVector.size() == rhsFunction->GetNumExpectedParameters()) )
	{
	 // Actually make the call.  We can do the dynamic cast because we passed in the
	 //  symbol factory and thus know how the symbol was created.
	 Symbol* pReturn = rhsFunction->Execute(&symVector);

	 // Return the result, assuming it is not NIL
	 if(rhsFunction->IsValueReturned() == true)
	 {
		// There should be a return value
		assert(pReturn != 0);
		if(pReturn != 0)
		{
		   // Return the result
		   pSoarReturn = pReturn;
		}
		else
		{
		   // We have to return something to prevent a crash, so we return an error code
		   pSoarReturn = make_sym_constant(thisAgent, "error_expected_rhs_function_to_return_value_but_it_did_NOT");
		}
	 }
	 else
	 {
		// Expected that the rhs function would not return a value, but it did.  Return value ignored.
		assert(pReturn == 0);
	 }

	// In any case, we are done using the return value
	if(pReturn != 0)
	{
		symbol_remove_ref(thisAgent, pReturn) ;
	}
	else
	{
		// Wrong number of arguments passed to RHS function
		assert(false) ;

		// We can return anything we want to soar; we return an error message so at least the problem is obvious.
		if(rhsFunction->IsValueReturned() == true)
			pSoarReturn = make_sym_constant(thisAgent, "error_wrong_number_of_args_passed_to_rhs_function");
		}
	}

	return pSoarReturn;
}
