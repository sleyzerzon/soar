/////////////////////////////////////////////////////////////////
// AnalyzeXML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// An SML document consists of a series of elements.  We want to be
// able to ask questions like "give me the <result> tag" element and
// either get it or get NULL to say it's not in the document.
//
// We could do that by searching the SML document to answer each
// request, but that's time consuming.  Instead, we'll run through the
// entire document once and create references to the appropriate pieces
// and then each request becomes a look up.
//
// This class represents those references into the document.
// 
/////////////////////////////////////////////////////////////////

#ifndef ANALYZE_XML_H
#define ANALYZE_XML_H

#include "sml_ArgMap.h"
#include "ElementXMLHandle.h"

namespace sml {

// Forward declarations
class ElementXML ; 

class AnalyzeXML
{
protected:
	ElementXML_Handle m_hRootObject ;// The message being analyzed
	ElementXML* m_pCommand ;		// The Command tag (for calls)
	ElementXML* m_pResult ;			// The Result tag (for responses)
	ElementXML* m_pError ;			// The Error tag

	bool		m_IsSML ;			// True if this is an <sml> XML object

	ArgMap		m_ArgMap ;			// Map from name to <arg> elements.  BUGBUG: To support multiple commands need multiple of these maps (one per command).

public:
	AnalyzeXML();
	virtual ~AnalyzeXML(void);

	// Call this on the message we want to analyze
	void Analyze(ElementXML const* pRootXML) ;

	// Each of these either returns a reference to the tag or NULL (if this document doesn't contain that tag)
	ElementXML const* GetCommandTag() const			{ return m_pCommand ; }
	ElementXML const* GetResultTag() const			{ return m_pResult ; }
	ElementXML const* GetErrorTag()	const			{ return m_pError ; }

	bool	IsSML() const { return m_IsSML ; }

	// Returns the name of the command (or NULL if no command tag or no name in that tag)
	char const* GetCommandName() const ;

	// Returns the character data from the result tag (or NULL if no result tag or no character data in that tag)
	char const* GetResultString() const ;

	// Returns the character data from the result tag as an int (or default value if there is no character data in the tag)
	int GetResultInt(int defaultValue) const ;

	// Returns the result as a bool
	bool GetResultBool(bool defaultValue) const ;

	/*************************************************************
	* @brief Look up an argument by name.  Returns NULL if not found.
	*************************************************************/
	char const* GetArgValue(char const* pArgName) const
	{
		return m_ArgMap.GetArgValue(pArgName, -1) ;
	}

	/*************************************************************
	* @brief As "GetArgValue" but parsed as a boolean.
	*************************************************************/
	bool GetArgBool(char const* pArgName, bool defaultValue) const
	{
		return GetArgBool(pArgName, -1, defaultValue) ;
	}

	/*************************************************************
	* @brief As "GetArgValue" but parsed as an int.
	*************************************************************/
	int GetArgInt(char const* pArgName, int defaultValue) const
	{
		return GetArgInt(pArgName, -1, defaultValue) ;
	}

	// We no longer use these argument position methods.
protected:
	/*************************************************************
	* @brief Arguments for commands are either named (e.g. print "-wme s2") or can be looked up based on order
	* (e.g. print s2).  Either all arguments are named or none are (in a valid SML doc).
	* This lookup supports both.  You pass in the name of the argument and its position
	* in the argument order.  We look for it by name and if that fails, look for it by position.
	* You either get back the right arg or NULL if this document doesn't contain that argument.
	* If you wish to look up a value by name only pass -1 for the position (shouldn't need this very often if ever).
	*************************************************************/
	char const* GetArgValue(char const* pArgName, int argPos) const
	{
		return m_ArgMap.GetArgValue(pArgName, argPos) ;
	}

	/*************************************************************
	* @brief As "GetArgValue" but parsed as a boolean.
	*************************************************************/
	bool GetArgBool(char const* pArgName, int argPos, bool defaultValue) const ;

	/*************************************************************
	* @brief As "GetArgValue" but parsed as an int.
	*************************************************************/
	int GetArgInt(char const* pArgName, int argPos, int defaultValue) const ;

	void AnalyzeArgs(ElementXML const* pCommand) ;
};

}

#endif	// ANALYSIS_XML_H
