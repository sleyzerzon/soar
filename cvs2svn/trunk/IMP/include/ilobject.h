#ifndef IL_OBJECT_H
#define IL_OBJECT_H

#include <vector>
#include <string>
#include <iostream>

#include "sml_ClientWMElement.h"
/************************************************************************
* ILObject
* 
* This class contains the necessary data and methods for encapsulating
* a specific line entry as given to the IMP program by the user.  
*************************************************************************
*/

enum eElementType
{
	ELEMENT_STRING,
	ELEMENT_INT,
	ELEMENT_FLOAT,
	ELEMENT_ID,
	ELEMENT_TYPE_TBD,
	//ELEMENT_LAST_TYPE = ELEMENT_TYPE_TBD
};

const std::string k_intString("int");
const std::string k_stringString("str");
const std::string k_floatString("flt");
const std::string k_idString("id");
const std::string k_onChangeString("onchange");
const std::string k_conditionString("cond");
const std::string k_cycleString("cycle");
const std::string k_TBD("To Be Determined at runtime");

eElementType		StringToType(const std::string& source);
const std::string	TypeToString(eElementType type);

enum eUpdateFrequency
{
	UPDATE_NEVER,
	UPDATE_ON_CHANGE,			//TODO isn't this really a case of ON_CONDITION?
	UPDATE_ON_CONDITION, 
	UPDATE_EVERY_CYCLE
};

//A union is actually cleaner for this, but there was some nonsense that resulted
//Only one of these members will "exist" at a time, which fits better with the union concept
struct WMEValue
{
	//this is a string so that a variable name can be stored here temporarily, to be replaced
	//with the string representation of the integer value later
	std::string i;
	//int i;
	//this is a string so that a variable name can be stored here temporarily, to be replaced
	//with the string representation of the floating point value later	
	std::string f;
	//double f;
	std::string s;
	std::string id;
};

typedef std::vector<eElementType> typesContainter_t;
typedef typesContainter_t::iterator typesIterator_t;

class InputLinkObject
{

public:
	InputLinkObject();
	//InputLinkObject(std::string& inParent, std::string& inName, std::vector<eElementType>& inTypes, std::string& inValue);
	~InputLinkObject();

	void		AddElementType(std::string& inType){m_elementTypes.push_back(StringToType(inType));}
	void		SetParentId(std::string& inParent){m_parentId = inParent;}
	void		SetAttribName(std::string& inName){m_attribName = inName;}
	void		SetStartValue(std::string& inValue);
	void		SetSimulationClassName(std::string& inName){m_simulationClassName = inName;}
	
	void		SetUpdateValue(std::string& inValue);
	/************************************************************************/
	/* Sets the initial type for this WME.  By default, type is TBD                                                                     */
	/************************************************************************/
	void		SetType(std::string& inValue);
	/************************************************************************/
	/* Sometimes the string arg we start with is const.  Functionally equivalent
	/  to the non-const version                                                                     */
	/************************************************************************/
	void		SetType(const std::string& inValue);
	/************************************************************************/
	/* With no args, set the initial type to be equal to the first 
	/  element of the type container (which shouldn't be empty if this is called)                                                                     */
	/************************************************************************/
	void		SetType();
	int			GetNumTypes() const {return m_elementTypes.size();}
	eElementType GetCurrentType() const {return m_curType;}
	std::string	GetFrequencyAsString() const;
	std::string	GetUpdateValue() const {return m_updateValue;}
	std::string	GetStartValue() const;
	std::string GetParent() const {return m_parentId;}
	std::string GetGeneratedName() const {return m_name;}
	std::string GetAttributeName() const {return m_attribName;}
	std::string GetSimulationClassName() const {return m_simulationClassName;}
	void PrintTypes(std::ostream& stream);

	void SetUpdateFrequency(std::string& inValue);
	void SetUpdateCondition(std::string& inValue);
	void SetGeneratedName(std::string& myName){m_name = myName;}

	friend std::ostream& operator << (std::ostream& stream, InputLinkObject& obj);
	bool	HasBeenInspected() const {return m_beenInspected;}
	void	MarkAsInspected() {m_beenInspected = true;}

private:

	std::string				m_parentId;
	std::string				m_attribName;
	std::string				m_updateValue;
	std::string				m_updateCondition;
	std::string				m_simulationClassName;
	typesContainter_t	m_elementTypes;
	eElementType			m_curType;
	WMEValue					m_value;
	eUpdateFrequency	m_updateFrequency;

	bool							m_beenInspected;//TODO determine if this is unnecessary
	
	//sml::WMElement*		m_wme;
	
	//this may end up being a vector of strings
	std::string				m_name;//The name of the generated variable corresponding to 'this'
};

#endif //IL_OBJECT_H