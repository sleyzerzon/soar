#include <portability.h>

#include "cli_OptionHandler.h"

#include <algorithm>

using namespace cli;

struct CollectMatches 
{
	const std::string& argument;
	std::vector< Option >& matches;

	CollectMatches( const std::string& argument, std::vector< Option >& matches ) 
		: argument( argument )
		, matches( matches )
    {
    }

    void operator()( const Option& value ) 
    {
		if ( value.longOption.compare( 0, argument.size(), argument ) == 0 )
		{
			matches.push_back( value );
		}
    }

private:
	CollectMatches& operator=( const CollectMatches& );
};

Option DetermineLongOption( const std::string& argument, const std::vector< Option >& options )
{
	std::vector< Option > matches;

	std::for_each( options.begin(), options.end(), CollectMatches( argument, matches ) );

	if ( matches.empty() ) 
	{
		throw OptionHandler::UnrecognizedOptionException( argument );
	}

	if ( matches.size() > 1 )
	{
		throw OptionHandler::AmbiguousOptionException( matches );
	}
	
	return matches.front();
}

bool HandleOptionArguments( const Option& option, const std::vector< std::string >::const_iterator& iter, const std::vector< std::string >& argv )
{
	if ( iter != argv.end() )
	{
		if ( option.argumentRequired )
		{
			// the argument that follows must be the option argument
			*option.argument = *iter;

			return true;
		}
		else
		{
			// if the next argument starts with a dash, we assume it is a new option
			// there are pros and cons to this assumption
			assert ( !iter->empty() );
			if ( iter->at( 0 ) != '-' )
			{
				// the argument that follows must be the option argument
				*option.argument = *iter;

				return true;
			}
		}
	}
	else
	{
		if ( option.argumentRequired )
		{
			throw OptionHandler::OptionArgumentRequiredException( option );
		}
	}
	return false;
}

OptionHandler::OptionHandler()
{
}

OptionHandler::~OptionHandler()
{
}

void OptionHandler::AddOption( char shortOption, char const* longOption, int* count )
{
	Option option( shortOption, longOption, count );
	m_Options.push_back( option );
}

void OptionHandler::AddOptionRequiredArgument( char shortOption, char const* longOption, int* count, std::string* argument )
{
	Option option( shortOption, longOption, count, argument );
	m_Options.push_back( option );
}

void OptionHandler::AddOptionOptionalArgument( char shortOption, char const* longOption, int* count, std::string* argument )
{
	Option option( shortOption, longOption, count, argument, false );
	m_Options.push_back( option );
}

void ClearOption( Option& option )
{
	option.argument = 0;
	option.count = 0;
}

void OptionHandler::ProcessOptions( std::vector< std::string >& argv )
{
	assert( argv.size() );

	// clear options
	std::for_each( m_Options.begin(), m_Options.end(), ClearOption );

	// iterate through arguments, skipping first
	std::vector< std::string >::iterator iter = argv.begin();
	++iter;

	while ( iter != argv.end() )
	{
		// args less than 2 characters cannot mean anything to us
		if ( iter->size() < 2 )
		{
			++iter;
			continue;
		}

		if ( iter->at( 0 ) == '-' )
		{
			if ( iter->at( 1 ) == '-' )
			{
				if ( iter->size() == 2 ) 
				{
					// special end of options argument
					argv.erase( iter );
					return;
				}

				// long option
				Option option( DetermineLongOption( iter->substr( 2 ), m_Options ) );
			
				// increment incidence count
				option.count += 1;

				// consume option
				iter = argv.erase( iter );

				if ( option.argument )
				{
					if ( HandleOptionArguments( option, iter, argv ) )
					{
						// consume option argument
						iter = argv.erase( iter );
					}
				}
				continue;
			}

			// short option(s)
			size_t index = 1;
			std::vector< Option >::iterator optionIter = m_Options.end();
			do {
				{
					Option temp( iter->at( index ) );
					optionIter = find( m_Options.begin(), m_Options.end(), temp );
				}
				if ( optionIter == m_Options.end() )
				{
					throw UnrecognizedOptionException( iter->at( index ) );
				}

				// increment incidence count
				optionIter->count += 1;

				if ( optionIter->argument )
				{
					// if this option could take an argument, it must be the last one in the group
					// even if optional argument
					if ( index != iter->size() - 1 )
					{
						throw OptionArgumentRequiredException( optionIter->shortOption );
					}
				}

			} while ( ++index < iter->size() );

			// consume option(s)
			iter = argv.erase( iter );

			if ( optionIter->argument )
			{
				if ( HandleOptionArguments( *optionIter, iter, argv ) )
				{
					// consume option argument
					iter = argv.erase( iter );
				}
			}
			continue;
		}

		// not an option
		++iter;
	}

	// done
}
