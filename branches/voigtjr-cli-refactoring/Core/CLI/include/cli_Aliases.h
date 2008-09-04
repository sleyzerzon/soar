#ifndef CLI_ALIASES_H
#define CLI_ALIASES_H

#include <string>
#include <map>
#include <vector>

namespace cli {

	typedef std::map< std::string, std::vector< std::string > > AliasMap;
	typedef std::map< std::string, std::vector< std::string > >::iterator AliasMapIter;
	typedef std::map< std::string, std::vector< std::string > >::const_iterator AliasMapConstIter;

	class Aliases
	{
	public:

		bool IsAlias( const std::string& command ) const
		{
			return m_AliasMap.find( command ) != m_AliasMap.end();
		}

		void NewAlias( const std::vector<std::string>& substitution, const std::string& commandToSubstitute )
		{
			if ( IsAlias( commandToSubstitute ) ) {
				RemoveAlias( commandToSubstitute );
			}
			m_AliasMap[ commandToSubstitute ] = substitution;
		}

		bool RemoveAlias( const std::string& command )
		{
			return m_AliasMap.erase( command ) ? true : false;
		}

		bool Translate( std::vector<std::string>& argv ) const
		{
			if ( argv.size() == 0 )
			{
				return false;
			}

			AliasMapConstIter iter = m_AliasMap.find( argv[0] );

			if ( iter == m_AliasMap.end() )
			{
				return false;
			}

			// Copy the alias out of the alias map
			std::vector< std::string > newArgv( ( m_AliasMap.find( argv[0] ) )->second );

			// Add the args from the passed argv to the alias
			std::vector< std::string >::iterator vIter = argv.begin();

			// ... skipping the first command (substituting it with the alias from the alias map
			vIter += 1;

			// add to the end of the new argv
			newArgv.insert( newArgv.end(), vIter, argv.end() );

			// save the new argv
			argv.swap( newArgv );

			// return true since we did a substitution
			return true;
		}

		AliasMap::const_iterator GetAliasMapBegin() const 
		{
			return m_AliasMap.begin();
		}

		AliasMap::const_iterator GetAliasMapEnd() const
		{
			return m_AliasMap.end();
		}

		std::string List(const std::string* pCommand = 0)
		{
			std::string result;

			for (AliasMapIter i = m_AliasMap.begin(); i != m_AliasMap.end(); ++i) {
				if (pCommand) {
					if (i->first != *pCommand) continue;
				}
				result += i->first;
				result += '=';
				for (std::vector<std::string>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
					result += *j;
					result += ' ';
				}
				if (pCommand) return result;
				result += '\n';
			}

			// didn't find one?
			if (pCommand) return result;

			// remove trailing newline
			result = result.substr(0, result.size()-1);
			return result;
		}

	private:
		
		AliasMap m_AliasMap;
	};

} // namespace cli

#endif // CLI_ALIASES_H
