#ifndef CLI_COMMANDOUTPUT_H
#define CLI_COMMANDOUTPUT_H

#include "sml_TagResult.h"
#include "sml_TagArg.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <cassert>

namespace cli
{
	class CommandOutput
	{
	public:
		CommandOutput()
			: m_UsingRawOutput( true )
		{}

		void SetUsingRawOutput( bool usingRawOutput ) 
		{ 
			m_UsingRawOutput = usingRawOutput;
		}

		bool GetUsingRawOutput() const 
		{ 
			return m_UsingRawOutput; 
		}

		void Clear()
		{
			if ( m_RawOutput.str().length() )
			{
				m_RawOutput.str( "" );
			}

			if ( m_ResponseTags.size() )
			{
				assert( false ); // this next line must be checked
				std::vector< soarxml::ElementXML* >( m_ResponseTags ).swap( m_ResponseTags );
			}
		}

		bool HaveOutput()
		{
			return m_RawOutput.str().length() || m_ResponseTags.size();
		}

		// raw
		std::ostringstream m_RawOutput;

		// structured
		void AppendArgTag( const char* pParam, const char* pType, const char* pValue ) 
		{
			sml::TagArg* pTag = new sml::TagArg();
			pTag->SetParam( pParam );
			pTag->SetType( pType );
			pTag->SetValue( pValue );
			m_ResponseTags.push_back( pTag );
		}

		void AppendArgTagFast( const char* pParam, const char* pType, const char* pValue )
		{
			sml::TagArg* pTag = new sml::TagArg();
			pTag->SetParamFast( pParam );
			pTag->SetTypeFast( pType );
			pTag->SetValue( pValue );
			m_ResponseTags.push_back( pTag );
		}

		void PrependArgTag( const char* pParam, const char* pType, const char* pValue ) 
		{
			sml::TagArg* pTag = new sml::TagArg();
			pTag->SetParam( pParam );
			pTag->SetType( pType );
			pTag->SetValue( pValue );
			m_ResponseTags.insert( m_ResponseTags.begin(), pTag );
		}

		void PrependArgTagFast( const char* pParam, const char* pType, const char* pValue ) 
		{
			sml::TagArg* pTag = new sml::TagArg();
			pTag->SetParamFast( pParam );
			pTag->SetTypeFast( pType );
			pTag->SetValue( pValue );
			m_ResponseTags.insert( m_ResponseTags.begin(), pTag );
		}

		void RecordResponse( soarxml::ElementXML* pResponse )
		{
			sml::TagResult* pTag = new sml::TagResult();
			
			for ( std::vector< soarxml::ElementXML* >::iterator iter = m_ResponseTags.begin(); iter != m_ResponseTags.end(); ++iter )
			{
				pTag->AddChild( *iter );
			}

			pResponse->AddChild( pTag );
		}

	private:
		bool m_UsingRawOutput;
		std::vector< soarxml::ElementXML* > m_ResponseTags;
	};

}

#endif // CLI_COMMANDOUTPUT_H
