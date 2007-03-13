/////////////////////////////////////////////////////////////////
// RhsFunction class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2007
//
// Support for right hand side functions.
/////////////////////////////////////////////////////////////////

#ifndef RHS_FUNCTION_H
#define RHS_FUNCTION_H

#include <vector>

typedef union symbol_union Symbol;
typedef struct agent_struct agent;
typedef struct cons_struct list;

namespace sml {

class KernelSML ;

class RhsFunction
{
protected:
	// The kernel
	KernelSML* m_pKernelSML ;

public:
	static Symbol* RhsFunctionCallback(agent* thisAgent, list* args, void* user_data) ;
	static const int kPARAM_NUM_VARIABLE = -1 ;

	RhsFunction() { }

	virtual ~RhsFunction() { }

	/** 
	* Returns the name of the RHS function.
	*
	* All Rhs functions must have unique names so that they can be identified in Soar
	*/
	virtual const char*  GetName() const = 0;

	/** 
	* Gets the number of parameters expected for this RHS function
	*/
	virtual int GetNumExpectedParameters() const = 0;

	/** 
	* Returns true if the RHS function returns a value other than 0 from Execute
	*/
	virtual bool         IsValueReturned() const = 0;

      /** 
       * Executes the RHS function given the set of symbols
       *
       * You should NOT release the symbol values that are passed in.
       * Because this is a callback, the calling method will release them.  However, if you clone
       *  or otherwise copy any values, you are responsible for releasing the copies.
       */
	  virtual Symbol* Execute(std::vector<Symbol*>* pArguments) = 0;
} ;

}

#endif
