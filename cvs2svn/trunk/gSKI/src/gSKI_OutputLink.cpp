/********************************************************************
* @file gski_outputlink.cpp
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   7/22/2002   13:40
*
* purpose: 
*********************************************************************/

#include "IgSKI_OutputProcessor.h"

#include "gSKI_OutputLink.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"
#include "gSKI_Symbol.h"
#include "gSKI_WMObject.h"
#include "gSKI_OutputWme.h"
#include "MegaAssert.h"
#include "IgSKI_Iterator.h"

#include "gSKI_OutputWMObject.h"

#include <iostream>
#include <algorithm>

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_OutputLink);


namespace gSKI
{

   /*
     ===============================

     ===============================
   */
 
  OutputLink::OutputLink(Agent* agent):
    m_agent(agent),
    m_memory(agent),
    m_autoupdate(true)
   {
      soar_add_callback( m_agent->GetSoarAgent(),
			 static_cast<void*>(m_agent->GetSoarAgent()),
			 OUTPUT_PHASE_CALLBACK,
			 OutputPhaseCallback,
			 static_cast<void*>(this),
			 0,
			 "output-link");
   }

   /*
     ===============================

     ===============================
   */

   OutputLink::~OutputLink() 
   {
      // Removing the static callback from the soar kernel
      soar_remove_callback( m_agent->GetSoarAgent(),
			    static_cast<void*>(m_agent->GetSoarAgent()),
			    OUTPUT_PHASE_CALLBACK,
			    "static_output_callback" );
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::AddOutputProcessor(const char* attributePath,
				       IOutputProcessor* processor,
				       Error* error)
   {
      ClearError(error);
      
      if ( processor == 0 || attributePath == 0 ) return;

      std::string path(attributePath);

      m_processormap.insert(std::pair<std::string, IOutputProcessor*>
                            (path, processor));
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::RemoveOutputProcessor(const char* attributePath,
                                          IOutputProcessor* processor,
					  Error* error)
   {
      ClearError(error);
      
      if ( processor == 0 || attributePath == 0 ) return;

      std::string path(attributePath);
  
      for ( tProcessorIt it = m_processormap.find(std::string(path));
            it != m_processormap.upper_bound(path);
            /* don't increment an erasure loop normally */) {
        // Searching through the processor map and removing all instances
        // of the processor for this path
        IOutputProcessor* tempprocessor = it->second;
        if ( tempprocessor == processor ) {
          m_processormap.erase(it++);
        } else {
          ++it;
        }
      }

   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::GetRootObject(IWMObject** rootObject, Error* error)
   {
      ClearError(error);
      
      MegaAssert(false, "NOT IMPLEMENTED YET!");

      rootObject = 0;
   }

   /*
     ===============================

     ===============================
   */

   IWorkingMemory* OutputLink::GetOutputMemory(Error* error)
   {
      ClearError(error);
      
      MegaAssert(false, "NOT IMPLEMENTED YET!");

      return 0;
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::OutputPhaseCallback( soar_callback_agent agent,
					 soar_callback_data callbackdata,
                                         soar_call_data calldata )
   {
      // Sorry but this has to be an old style cast to
      // keep egcs 1.1.2 from segfaulting
      // TODO: Find out why this cast won't work
      OutputLink* olink = (OutputLink*)(callbackdata);
      output_call_info* oinfo = static_cast<output_call_info*>(calldata);
      int callbacktype = oinfo->mode;
	  egSKIWorkingMemoryChange change = gSKI_ADDED_OUTPUT_COMMAND ;

      //std::cout << "Normal output link update cycle!" << std::endl;

      switch (callbacktype) {
      case ADDED_OUTPUT_COMMAND:
         olink->InitialUpdate(oinfo->outputs);
		 change = gSKI_ADDED_OUTPUT_COMMAND ;
         break;
      case MODIFIED_OUTPUT_COMMAND:
         olink->Update(oinfo->outputs);
		 change = gSKI_MODIFIED_OUTPUT_COMMAND ;
         break;
      case REMOVED_OUTPUT_COMMAND:
         olink->FinalUpdate(oinfo->outputs);
		 change = gSKI_REMOVED_OUTPUT_COMMAND ;
         break;
      default:
         MegaAssert(false, "The static output callback is of unknown type!");
         break;
      }

	  // Notify any listeners about this event
	  if (olink->m_workingMemoryListeners.GetNumListeners(gSKIEVENT_OUTPUT_PHASE_CALLBACK) != 0)
	  {
	     // We have a list of kernel wme objects and we need a list of gSKI objects.
		 // I'm not sure how to convert from one to the other, but here's my best guess.
	     std::vector<IWme*> outputWmes;

		// Walk through the list of wmes from the kernel
         for (io_wme* cur = oinfo->outputs; cur != 0; cur = cur->next)
		 {
		    // Get the kernel wme that corresponds to the io_wme (which is just a simple triplet of symbols)
			wme* wme = olink->m_memory.GetOutputWme(cur->id, cur->attr, cur->value);

			// DJP: Do we need to check the slot wmes too?  If this assert fails, we may need to add that slot code.
			MegaAssert( wme != 0, "IO wme not found in kernel!");

			 // Convert from kernel wme to gSKI wme here
			 IWme* pWme = new OutputWme(&olink->m_memory, wme) ;
			 outputWmes.push_back(pWme) ;
		 }
    
		 // I think this will release the wmes when the iterator is destroyed
         tWmeIter wmelist(outputWmes);

		 // Fire the event
         WorkingMemoryNotifier wmn(olink->m_agent, change, &wmelist);
	     olink->m_workingMemoryListeners.Notify(gSKIEVENT_OUTPUT_PHASE_CALLBACK, wmn);
	  }
   }

    /**
    * @brief Listen for changes to wmes attached to the output link.
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void OutputLink::AddWorkingMemoryListener(egSKIEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);
      m_workingMemoryListeners.AddListener(eventId, listener);

   }

    /**
    * @brief Remove an existing listener
    *
	* @param eventId		The event to listen to.  Can only be gSKIEVENT_OUTPUT_PHASE_CALLBACK currently.
	* @param listener	The handler to call when event is fired
    */
   void OutputLink::RemoveWorkingMemoryListener(egSKIEventId eventId, 
							     IWorkingMemoryListener* listener, 
								 Error*               err)
   {
      ClearError(err);
      m_workingMemoryListeners.RemoveListener(eventId, listener);
   }

   /*
     ===============================

     ===============================
   */

   void OutputLink::InitialUpdate(io_wme* wmelist)
   {
      // Perform necessary functions
      //std::cout << "\nInitial output link update!\n";

     ProcessIOWmes(wmelist);
   }
   /*
     ===============================

     ===============================
   */

   void OutputLink::Update(io_wme* wmelist)
   {
      // Perform necessary functions
      //std::cout << "\nNormal output link update!\n";

     ProcessIOWmes(wmelist);
   }

   /*
     ===============================

     ===============================
   */

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

   /*
     ===============================

     ===============================
   */

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
      m_memory.UpdateWithIOWmes(wmelist);

      // Searching for new matching patterns for IOutputProcessors
      
      if ( m_autoupdate ) {
         InvokeOutputProcessors();
      }
      
      
   }

   /*
     ===============================

     ===============================
   */
   void OutputLink::InvokeOutputProcessors(Error * err) 
   {
      ClearError(err);

      // TODO: Make this more efficient for repeated paths
      for ( tProcessorIt it = m_processormap.begin();
            it != m_processormap.end();
            ++it) {
        
         // Finding the objects that match the criterion
         OutputWMObject* rootObj;
         m_memory.GetOutputRootObject(&rootObj);
         tIWMObjectIterator* matchingObjects = m_memory.FindObjects(rootObj,
                                                                    it->first);
         
         for ( ;
               matchingObjects->IsValid();
               matchingObjects->Next() ) {
            // Getting the object from the iterator and invoking the processor
            IWMObject* curobj = matchingObjects->GetVal();
            it->second->ProcessOutput(&m_memory, curobj);
            curobj->Release();
         }
         
         rootObj->Release();
         matchingObjects->Release();
         
      }
   }
   
}

