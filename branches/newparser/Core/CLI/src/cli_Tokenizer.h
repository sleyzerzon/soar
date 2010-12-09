/////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////

#ifndef CLI_TOKENIZER_H
#define CLI_TOKENIZER_H

#include <string>
#include <vector>

//#define PRINT_CALLBACKS 1
#ifdef PRINT_CALLBACKS
#include <iostream>
#endif

namespace cli
{
    class TokenizerCallback
    {
    public:
        virtual ~TokenizerCallback() {}
        virtual bool HandleCommand(std::vector<std::string>& argv) = 0;
    };

    class TokenizerCurrent
    {
    private:
        int line;
        int offset;
        const char* current;

    public:
        TokenizerCurrent()
        {
            SetCurrent(0);
        }
        TokenizerCurrent(const char* initial)
        {
            SetCurrent(initial);
        }
        virtual ~TokenizerCurrent() {}
        
        void SetCurrent(const char* initial)
        {
            current = initial;
            if (current && *current)
            {
                line = 1;
                offset = 1;
            }
            else
            {
                line = 0;
                offset = 0;
            }
        }

        void Fail()
        {
            current = 0;
        }

        void Increment()
        {
            if (*current == '\n')
            {
                line += 1;
                offset = 0;
            }
            current += 1;
            offset += 1;
        }

        int GetLine()
        {
            return line;
        }

        int GetOffset()
        {
            return offset;
        }

        bool Good()
        {
            return current != 0;
        }

        bool Bad()
        {
            return !current;
        }

        char Get()
        {
            return *current;
        }

        bool Eof()
        {
            return !*current;
        }
    };

    class Tokenizer
    {
    private:
        TokenizerCurrent current;
        TokenizerCallback* callback;
        int commandStartLine;
        const char* error;

    public:
        Tokenizer()
            : callback(0), error(0) 
        {}
        virtual ~Tokenizer() {}

        void SetHandler(TokenizerCallback* callback)
        {
            this->callback = callback;
        }

        bool Evaluate(const char* const input)
        {
            current.SetCurrent(input);
            commandStartLine = 1;
            error = 0;

            while (current.Good())
            {
                if (current.Eof())
                    break;
                ParseCommand();
            }
            return current.Good();
        }

        int GetCommandLineNumber()
        {
            return commandStartLine;
        }

        int GetCurrentLineNumber()
        {
            return current.GetLine();
        }

        const char* GetErrorString()
        {
            return error;
        }

        int GetOffset()
        {
            return current.GetOffset();
        }

    private:
        void ParseCommand()
        {
            std::vector< std::string > argv;
            SkipWhitespaceAndComments();
            while (ParseWord(argv))
                SkipWhitespace();

            if (current.Bad())
                return;

            if (argv.empty())
                return;

#ifdef PRINT_CALLBACKS
            std::cout << "\n[";
            for (int i = 0; i < argv.size(); i++)
                std::cout << argv[i] << ",";
            std::cout << "]" << std::endl;
#endif
            if (callback)
            {
                if (!callback->HandleCommand(argv))
                {
                    error = "callback returned error";
                    current.Fail();
                }
            }
        }

        bool ParseWord(std::vector< std::string >& argv)
        {
            if (current.Eof())
                return false;

            std::string word;
            if (argv.empty())
                commandStartLine = current.GetLine();

            switch (current.Get())
            {
            case ';':
                break;

            case '"':
                argv.push_back(word);
                ReadQuotedString(argv);
                break;

            case '{':
                argv.push_back(word);
                ReadBraces(argv);
                break;

            default:
                argv.push_back(word);
                ReadNormalWord(argv);
                break;
            }

            return !AtEndOfCommand();
        }

        void ReadNormalWord(std::vector< std::string >& argv)
        {
            do
            {
                char c = current.Get();
                bool semi = false;
                switch (c)
                {
                case '\\':
                    c = ParseEscapeSequence();
                    if (current.Bad())
                        return;
                    break;

                case ';':
                    semi = true;
                    // falls to be consumed
                default:
                    current.Increment();
                    break;
                }

                if (semi)
                    return;

                argv.back().push_back(c);
            }
            while (!current.Eof() && !isspace(current.Get()));
        }

        bool AtEndOfCommand()
        {
            while (current.Good())
            {
                switch (current.Get())
                {
                case '\n':
                case ';':
                    current.Increment();
                case 0:
                    return true;

                default:
                    break;
                }

                if (!isspace(current.Get()))
                    return false;

                current.Increment();
            }
            return true;
        }

        void ReadQuotedString(std::vector< std::string >& argv)
        {
            current.Increment(); // consume "

            while (current.Get() != '"')
            {
                switch (current.Get())
                {
                case 0: 
                    error = "unexpected eof";
                    current.Fail();
                    return;

                case '\\':
                    {
                        char c = ParseEscapeSequence();
                        if (current.Bad()) return;
                        argv.back().push_back(c);
                    }
                    break;

                default:
                    argv.back().push_back(current.Get());
                    current.Increment();
                    break;
                }
            }

            current.Increment(); // consume "
        }

        char ParseEscapeSequence()
        {
            current.Increment(); // consume backslash

            // future work? newline, octal, hex, wide hex

            char ret = 0;
            bool increment = true;
            switch (current.Get())
            {
            case 0:
                error = "unexpected eof";
                current.Fail();
                return 0;
            case 'a':
                ret = '\a';
                break;
            case 'b':
                ret = '\b';
                break;
            case 'f':
                ret = '\f';
                break;
            case 'n':
                ret = '\n';
                break;
            case 'r':
                ret = '\r';
                break;
            case 't':
                ret = '\t';
                break;
            case 'v':
                ret = '\v';
                break;
            case '\n':
                SkipWhitespace();
                ret = ' ';
                increment = false; // SkipWhitespace leaves us past where we want to be
                break;
            default:
                ret = current.Get();
                break;
            }
            if (increment)
                current.Increment();
            return ret;
        }

        void ReadBraces(std::vector< std::string >& argv)
        {
            current.Increment(); // consume brace;
            int depth = 1;
            while (depth)
            {
                switch (current.Get())
                {
                case 0:
                    error = "unexpected eof, unmatched opening brace";
                    current.Fail();
                    return;

                case '}':
                    depth -= 1;
                    if (depth) // consume only the final closing brace
                        argv.back().push_back(current.Get());
                    break;

                case '{':
                    depth += 1;
                    // falls through
                default:
                    argv.back().push_back(current.Get());
                    break;
                }
                current.Increment();
            }
        }

        void SkipWhitespaceAndComments()
        {
            SkipWhitespace();
            switch (current.Get())
            {
            case '0':
                return;
            case '#':
                SkipToEndOfLine();
                SkipWhitespaceAndComments();
                return;
            default:
                break;
            }
        }

        void SkipWhitespace()
        {
            while (!current.Eof() && isspace(current.Get()))
                current.Increment();
        }

        void SkipToEndOfLine()
        {
            while (current.Get() != '\n')
            {
                current.Increment();
                if (current.Eof())
                    break;
            }
        }
    };
}

#endif // CLI_TOKENIZER_H
