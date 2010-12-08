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
        const char* current;

    public:
        TokenizerCurrent()
            : line(1), current(0)
        {}
        TokenizerCurrent(const char* initial)
            : line(1), current(initial)
        {}
        virtual ~TokenizerCurrent() {}
        
        void SetCurrent(const char* initial)
        {
            current = initial;
        }

        void Fail()
        {
            current = 0;
        }

        void Increment()
        {
            if (*current == '\n')
                line += 1;
            current += 1;
        }

        void Decrement()
        {
            current -= 1;
            if (*current == '\n')
                line -= 1;
        }

        int GetLine()
        {
            return line;
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

    public:
        Tokenizer()
            : callback(0) 
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
                if (!callback->HandleCommand(argv))
                    current.Fail();
        }

        bool ParseWord(std::vector< std::string >& argv)
        {
            if (current.Eof())
                return false;

            std::string word;
            if (argv.empty())
                commandStartLine = current.GetLine();
            argv.push_back(word);

            switch (current.Get())
            {
            case '"':
                ReadQuotedString(argv);
                break;

            case '{':
                ReadBraces(argv);
                break;

            default:
                do
                {
                    argv.back().push_back(current.Get());
                    current.Increment();
                }
                while (!current.Eof() && !isspace(current.Get()));
                break;
            }

            return !AtEndOfCommand();
        }

        bool AtEndOfCommand()
        {
            while (current.Good())
            {
                switch (current.Get())
                {
                case 0:
                case '\n':
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
                case 0: // unexpected eof
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
                    break;
                }
                current.Increment();
            }

            current.Increment(); // consume "
        }

        char ParseEscapeSequence()
        {
            current.Increment(); // consume backslash

            // future work? newline, octal, hex, wide hex

            char ret = 0;
            switch (current.Get())
            {
            case 0: // unexpected eof
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
                current.Decrement(); // SkipWhitespace leaves us past where we want to be
                break;
            default:
                ret = current.Get();
                break;
            }
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
                case 0: // unexpected eof, unmatched brace
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
