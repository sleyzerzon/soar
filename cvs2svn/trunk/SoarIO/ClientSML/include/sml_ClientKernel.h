/////////////////////////////////////////////////////////////////
// Kernel class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used by a client app (e.g. an environment) to represent
// the top level connection to the Soar kernel.  You start by creating
// one of these and then creating agents through it etc.
//
/////////////////////////////////////////////////////////////////
#ifndef SML_KERNEL_H
#define SML_KERNEL_H

#include "sml_ObjectMap.h"

// BADBAD: Until I figure out how to return strings back... (voigtjr)
#include <string>

namespace sml {

// Forward declarations
class Agent ;
class Connection ;

class Kernel
{
protected:
	Connection*			m_Connection ;
	ObjectMap<Agent*>	m_AgentMap ;

public:
	Kernel(Connection* pConnection);

	virtual ~Kernel();

	/*************************************************************
	* @brief Returns the connection information for this kernel
	*		 which is how we communicate with the kernel (e.g. embedded,
	*		 remotely over a socket etc.)
	*************************************************************/
	Connection* GetConnection() const { return m_Connection ; }

	/*************************************************************
	* @brief Creates a new Soar agent with the given name.
	*
	* @returns A pointer to the agent (or NULL if not found).  This object
	*		   is owned by the kernel and will be destroyed when the
	*		   kernel is destroyed.
	*************************************************************/
	Agent* CreateAgent(char const* pAgentName) ;

	/*************************************************************
	* @brief Looks up an agent by name (from our list of known agents).
	*
	* @returns A pointer to the new agent structure.  This object
	*		   is owned by the kernel and will be destroyed when the
	*		   kernel is destroyed.
	*************************************************************/
	Agent* GetAgent(char const* pAgentName) ;

	/*************************************************************
	* @brief Process a command line command
	*
	* @param pCommandLine Command line string to process.
	* @param pAgentName Agent name to apply the command line to.
	* @param pResult BADBAD: I don't know how else to return a string to the client
	*                so I'm currently returning an STL string.
	*************************************************************/
	bool ProcessCommandLine(char const* pCommandLine, char const* pAgentName, std::string* pResult) ;
};

}//closes namespace

#endif //SML_KERNEL_H