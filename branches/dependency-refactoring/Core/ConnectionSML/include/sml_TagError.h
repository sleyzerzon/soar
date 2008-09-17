/////////////////////////////////////////////////////////////////
// TagError class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which represents an error in a result with
// the tag <error>.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_TAG_ERROR_H
#define SML_TAG_ERROR_H

#include "ElementXML.h"
#include "sml_Names.h"

#include <boost/lexical_cast.hpp>

namespace sml {

class TagError : public soarxml::ElementXML
{
public:
	TagError(void);
	~TagError(void);

	void SetDescription(char const* pErrorMsg)
	{
		this->SetCharacterData(pErrorMsg) ;
	}

	void SetErrorCode(int error)
	{
		this->AddAttributeFast(sml_Names::kErrorCode, boost::lexical_cast< std::string >( error ).c_str() ) ;
	}

};

}

#endif	// SML_TAG_ERROR_H
