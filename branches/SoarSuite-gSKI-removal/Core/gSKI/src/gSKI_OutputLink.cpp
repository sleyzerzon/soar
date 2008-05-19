#include <portability.h>

/*************************************************************************
* PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
* FOR LICENSE AND COPYRIGHT INFORMATION. 
*************************************************************************/

/********************************************************************
* @file gski_outputlink.cpp
*********************************************************************
* created:	   7/22/2002   13:40
*
* purpose: 
*********************************************************************/

#include "IgSKI_OutputProcessor.h"

#include "gSKI_OutputLink.h"
#include "gSKI_Error.h"
#include "gSKI_Symbol.h"
#include "gSKI_WMObject.h"
#include "gSKI_OutputWme.h"
#include "MegaAssert.h"
#include "IgSKI_Iterator.h"

#include "gSKI_OutputWMObject.h"

#include <iostream>
#include <algorithm>

namespace gSKI
{

	OutputLink::OutputLink(agent* agent)
		: m_agent(agent),
		m_memory(agent)
	{
	}

	OutputLink::~OutputLink() 
	{
	}

	void OutputLink::GetRootObject(IWMObject** rootObject, Error* error)
	{
		ClearError(error);

		OutputWMObject* obj;
		m_memory.GetOutputRootObject(&obj);
		*rootObject = obj;
	}

	void OutputLink::InitialUpdate(io_wme* wmelist)
	{
		// Perform necessary functions
		//std::cout << "\nInitial output link update!\n";

		ProcessIOWmes(wmelist);
	}

	void OutputLink::Update(io_wme* wmelist)
	{
		// Perform necessary functions
		//std::cout << "\nNormal output link update!\n";

		ProcessIOWmes(wmelist);
	}

	void OutputLink::FinalUpdate(io_wme* wmelist)
	{
		// Perform necessary functions
		//std::cout << "\nFinal output link update!\n";
		//
		// TODO: Insure that we don't want the call to ProcessIOWmes
		//        done here.
		//ProcessIOWmes(wmelist);
		//OutputWMObject* x = m_memory.GetOutputRootObject();

		//x->Release();
	}

	void OutputLink::ProcessIOWmes(io_wme* wmelist)
	{
		// Counting the number of io_wmes
		/*
		int count = 0;
		for (io_wme* cur = wmelist; cur != 0; cur = cur->next) {
		++count;

		std::cout << "\n------------------\n" 
		<< "Number = " << count << std::endl
		<< "ID     = " 
		<< gSymbol::ConvertSymbolToString(cur->id) 
		<< std::endl
		<< "Attr   = " 
		<< gSymbol::ConvertSymbolToString(cur->attr) 
		<< std::endl
		<< "Value  = " 
		<< gSymbol::ConvertSymbolToString(cur->value) 
		<< std::endl
		<< "------------------\n";
		}
		std::cout << "\nRecieved " << count << " Wmes on output link!\n";
		*/

		// Updating the working memory object with the io wme list
		// This call walks the current list of wmes and creates and destroys
		// gSKI proxy objects to match this list.
		m_memory.UpdateWithIOWmes(wmelist);

		// Searching for new matching patterns for IOutputProcessors

		//if ( m_autoupdate ) {
		//   InvokeOutputProcessors();
		//}


	}

}

