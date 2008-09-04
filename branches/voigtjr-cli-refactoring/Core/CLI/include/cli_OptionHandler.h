#ifndef CLI_OPTIONHANDLER_H
#define CLI_OPTIONHANDLER_H

#include <string>
#include <vector>
#include <exception>

namespace cli
{
	struct Option
	{
		Option( char shortOption )
			: shortOption( shortOption )
			, count( 0 )
			, argument( 0 )
			, argumentRequired( false )
		{}

		Option( char shortOption, char const* longOption, int* count, std::string* argument = 0, bool argumentRequired = true)
			: shortOption( shortOption )
			, longOption( longOption )
			, count( count )
			, argument( argument )
			, argumentRequired( argumentRequired )
		{}

		char shortOption;
		std::string longOption;
		int* count;
		std::string* argument;
		bool argumentRequired;

		bool operator==( const Option& other ) const
		{
			return this->shortOption == other.shortOption;
		}
		bool operator!=( const Option& other ) const
		{
			return !( *this == other );
		}
	};

	class OptionHandler
	{
	public:
		OptionHandler();
		virtual ~OptionHandler();

		void AddOption( char shortOption, char const* longOption, int* count );
		void AddOptionRequiredArgument( char shortOption, char const* longOption, int* count, std::string* argument );
		void AddOptionOptionalArgument( char shortOption, char const* longOption, int* count, std::string* argument );

		void ProcessOptions( std::vector< std::string >& argv );

		struct AmbiguousOptionException : public std::exception
		{
			std::vector< Option > possiblities;
			AmbiguousOptionException( const std::vector< Option >& possiblities )
				: possiblities( possiblities )
			{}

			char const* what() const throw()
			{
				return "AmbiguousOptionException";
			}
		};

		struct OptionArgumentRequiredException : public std::exception
		{
			Option option;
			OptionArgumentRequiredException( const Option& option )
				: option( option )
			{}

			char const* what() const throw()
			{
				return "OptionArgumentRequiredException";
			}
		};

		struct UnrecognizedOptionException : public std::exception
		{
			std::string option;
			UnrecognizedOptionException( const std::string& option )
				: option( option )
			{}

			UnrecognizedOptionException( char& option )
			{
				this->option.assign( 1, option );
			}

			char const* what() const throw()
			{
				return "UnrecognizedOptionException";
			}
		};

	private:
		std::vector< Option > m_Options;
	};
}

#endif // CLI_OPTIONHANDLER_H
