/////////////////////////////////////////////////////////////////
// WorkingMemory class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used to represent Soar's working memory.
// We maintain a copy of this on the client so we can just
// send changes over to the kernel.
//
// Basic method is that working memory is stored as a tree
// of Element objects.
// When the client makes a change to the tree, we modify the tree
// and add the change to the list of changes to make to WM.
// At some point, we actually send that list of changes over.
// We should be able to be clever about collapsing changes together
// in the list of deltas (e.g. change value A->B->C can remove the
// A->B change (since it's overwritten by B->C).
//
/////////////////////////////////////////////////////////////////
#ifndef SML_WORKING_MEMORY_H
#define SML_WORKING_MEMORY_H

#include "sml_ObjectMap.h"
#include "sml_DeltaList.h"

namespace sml {

// Forward declarations
class Agent ;
class Connection ;
class StringElement ;
class Identifier ;

class WorkingMemory
{
protected:
	long		m_TimeTagCounter ;
	Agent*		m_Agent ;
	Identifier*	m_InputLink ;
	DeltaList	m_DeltaList ;

public:
	WorkingMemory() ;

	virtual ~WorkingMemory();

	void			SetAgent(Agent* pAgent)	{ m_Agent = pAgent ; }
	Agent*			GetAgent() const		{ return m_Agent ; }
	char const*		GetAgentName() const ;
	Connection*		GetConnection()	const ;

	Identifier*		GetInputLink() ;
	StringElement*	CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue);
	long			GenerateTimeTag() ;
	bool			Commit() ;
};

}//closes namespace

#endif //SML_WORKING_MEMORY_H
