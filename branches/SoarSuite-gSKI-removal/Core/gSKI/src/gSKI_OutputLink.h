/*************************************************************************
* PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
* FOR LICENSE AND COPYRIGHT INFORMATION. 
*************************************************************************/

/********************************************************************
* @file igski_outputlink.h 
*********************************************************************
* created:	   6/13/2002   14:54
*
* purpose: 
*********************************************************************/
#ifndef GSKI_OUTPUTLINK_H
#define GSKI_OUTPUTLINK_H

//#include "IgSKI_OutputLink.h"
#include "gSKI_Error.h"

#include "gSKI_OutputWorkingMemory.h"
#include "gSKI_Enumerations.h"

#include "IgSKI_Wme.h"
#include "IterUtils.h"
#include "gSKI_Iterator.h"
#include "gSKI_ReleaseImpl.h"

#include <string>
//#include <multimap>

#include "callback.h"
#include "io_soar.h"

namespace gSKI {

	/**
	* @brief Handles registration and invokation of OutputProcessors
	*
	* The IOutputLink interface is mainly responsible for handling the
	* registration and invocation of the OutputProcessor objects. Registration
	* is done through the AddOutputProcessor() method, which essentially 
	* associates a pattern on the OutputLink with an OutputProcessor. 
	* All WMObjects that match the specified pattern are then passed to the
	* specified OutputProcessor which handles converting the wme's to 
	* function/method calls and data for modifying the simulation environment.
	* If a WMObject matches multiple patterns it will be passed to multiple
	* OutputProcessors for processing. The OutputProcessors can be set to 
	* automatically process output during the Agent's output cycle or they
	* can be manually invoked using the SetAutomaticUpdate() or 
	* UpdateProcessors() methods.
	*/
	// TODO: Still need to address automatic update vs. manual update issues
	class OutputLink/*: public IOutputLink*/ {
	public:

		/**
		* @brief Constructor
		*
		* This constructor is responsible for creating the output link object
		* and registering it to receive the appropriate callbacks from soar
		* kernel. A pointer to the owning agent is passed in to provide information
		* for the callback registration process.
		*
		* @param agent The agent that owns the output link.
		*/
		OutputLink(agent* agent);

		/**
		* @brief Virtual Destructor
		*
		* The OutputLink is wholly owned by the Agent object. The destructor
		* should never be called by SSI developers. 
		*/
		virtual ~OutputLink();

		/**
		* @brief Returns a pointer to the root WMObject on the output link
		*
		* This method returns a pointer to the root WMObject on the output link.
		* This is a convenience method that may help with navigating the 
		* output link.
		*
		* @param rootObject A pointer to the root WMObject pointer.
		*          Used to hold the return value. Returning the actual
		*          pointer from this method would make it too easy to
		*          create memory leaks since the pointer has to be
		*          released.
		* @param error Pointer to client-owned error structure.  If the pointer
		*        is not 0 this structure is filled with extended error
		*        information.  If it is 0 (the default) extended error
		*        information is not returned.
		*
		* @returns A pointer to the root WMObject on the output link
		*/     
		void GetRootObject(IWMObject** rootObject, Error* err = 0);

		/**
		* @brief Returns the IWorkingMemory object associated with the 
		*         output link.
		*
		* This method returns a pointer to the IWorkingMemory object associated
		* with the OutputLink. This IWorkingMemory object allows Wmes and
		* WMObjects to be easily added, removed and replaced. It also provides
		* convenience methods for searching for Wmes and obtaining static
		* snap shots of the agent's working memory data (on the output link).
		*
		* @param error Pointer to client-owned error structure.  If the pointer
		*        is not 0 this structure is filled with extended error
		*        information.  If it is 0 (the default) extended error
		*        information is not returned.
		*
		* @returns The IWorkingMemory object associated with the data on 
		*           the agent's output link.
		*/     
		OutputWorkingMemory* GetOutputMemory()
		{
			return &m_memory;
		}

		/**
		* @brief Reinitializes the output link
		*
		* This method reinitializes the output link by releasing all the wmes
		* and wmobjects held by the output link. The output processors are not
		* effected.
		*
		*/
		void Reinitialize() { m_memory.Reinitialize(); }

	private:
		/**
		* @brief Method called when the output link is created by the 
		*   Soar kernel
		*
		* This method is called by the OutputPhaseCallback whe the callback
		* is of type ADDED_OUTPUT_COMMAND.
		*/
		// TODO: Fully comment this method
		void InitialUpdate(io_wme* wmelist);

		/**
		* @brief Method called when the output link is created by the 
		*   Soar kernel
		*
		* This method is called by the OutputPhaseCallback whe the callback
		* is of type MODIFIED_OUTPUT_COMMAND.
		*/
		// TODO: Fully comment this method
		void Update(io_wme* wmelist);

		/**
		* @brief Method called when the output link is created by the 
		*   Soar kernel
		*
		* This method is called by the OutputPhaseCallback whe the callback
		* is of type REMOVED_OUTPUT_COMMAND.
		*/
		// TODO: Fully comment this method
		void FinalUpdate(io_wme* wmelist);

		/**
		* @brief Responsible for processing the io_wmes that are recieved
		*   from an output phase callback
		*/
		void ProcessIOWmes(io_wme* wmelist);

		agent* m_agent;  /**< The agent that owns this output link. */
		OutputWorkingMemory m_memory; /**< Stores outputlink's wme's/objects */

	};

}


#endif
