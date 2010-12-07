/////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////

#ifndef CLI_INTERP_H
#define CLI_INTERP_H

#include <string>
#include <vector>

//#define PRINT_CALLBACKS 1
#ifdef PRINT_CALLBACKS
#include <iostream>
#endif

namespace cli
{
    enum Token
    {
        TOK_SEMICOLON,
        TOK_WHITESPACE,
        TOK_EOL,
        TOK_QUOTE,
        TOK_PIPE,
        TOK_LBRACE,
        TOK_RBRACE,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_POUND,
        TOK_STRING,
        TOK_BACKSLASH,
        TOK_END,
    };

    typedef bool (*CommandCallback)(std::vector<std::string>& argv, uintptr_t userData);

    class Interp
    {
    public:
        Interp() {}
        ~Interp() {}

        void SetHandler(CommandCallback callback, uintptr_t userData)
        {
            this->callback = callback;
            this->userData = userData;
        }

        bool Evaluate(const char* const input)
        {
            line = 1;
            newline = false;
            current = input;
            tok = TOK_SEMICOLON;

            GetToken();
            while (current)
            {
                if (tok == TOK_END)
                    break;
                Command();
                GetToken();
            }
    
            return current != 0;
        }

        int GetLineNumber()
        {
            return line;
        }

    private:

        void Command()
        {
            std::vector<std::string> argv;
            const char* argStart = tokStart;
            int argLen = Argument(argStart);
            while (argLen > 0)
            {
                // if first arg is echo, need to do special capture of line
                argv.push_back(std::string(argStart, argLen));
                GetToken();
                if (tok == TOK_END || tok == TOK_EOL)
                    break;
                argStart = tokStart;
                argLen = Argument(argStart);
            }

            if (argv.empty())
                return;

            if (callback)
            {
#ifdef PRINT_CALLBACKS
                std::cout << "\n[";
                for (int i = 0; i < argv.size(); i++)
                    std::cout << argv[i] << ",";
                std::cout << "]" << std::endl;
#endif
                if (!(*callback)(argv, userData))
                    current = 0;
            }
            return;
        }

        bool ReadUntilNext(Token until, int& argLen)
        {
            do
            {
                GetToken();
                argLen += tokLen;
                if (tok == TOK_END)
                    return false;
            } while (tok != until);
            return true;
        }

        int Argument(const char*& argStart)
        {
            int argLen = 0;

            switch (tok)
            {
            case TOK_WHITESPACE:
                // skip whitespace
                GetToken();
                argStart = tokStart;
                return Argument(argStart);

            case TOK_EOL:
                break;

            case TOK_POUND:
                if(!ReadUntilNext(TOK_EOL, argLen))
                    current = 0;
                argLen = 0;
                break;

            default:
                argLen += tokLen;
                break;

            case TOK_PIPE:
            case TOK_QUOTE:
                argLen += tokLen;
                if(!ReadUntilNext(tok, argLen))
                    current = 0;
                break;

            case TOK_LPAREN:
            case TOK_LBRACE:
                if (!NestedArgument(argLen))
                    current = 0;
                break;

            case TOK_RPAREN:
            case TOK_RBRACE:
                current = 0;
                break;
            }

            return argLen;
        }

        bool NestedArgument(int& argLen)
        {
            int bdepth = 0;
            int pdepth = 0;

            do
            {
                // Don't get the first one, we got it already
                if (bdepth || pdepth)
                    GetToken();

                switch (tok)
                {
                case TOK_END:
                    return false;

                case TOK_LBRACE:
                    argLen += tokLen;
                    bdepth += 1;
                    break;

                case TOK_RBRACE:
                    argLen += tokLen;
                    bdepth -= 1;
                    break;

                case TOK_LPAREN:
                    argLen += tokLen;
                    pdepth += 1;
                    break;

                case TOK_RPAREN:
                    argLen += tokLen;
                    pdepth -= 1;
                    break;

                case TOK_QUOTE:
                case TOK_PIPE:
                    argLen += tokLen;
                    if(!ReadUntilNext(tok, argLen))
                        return false;
                    break;

                case TOK_POUND:
                    argLen += tokLen;
                    if(!ReadUntilNext(TOK_EOL, argLen))
                        return false;
                    break;

                default:
                    argLen += tokLen;
                    break;
                }

            } while (bdepth || pdepth);
            return true;
        }

        void GetToken()
        {
            if (tok == TOK_END)
                return;

            tokStart = current;
            tokLen = 0;

            if (current == 0)
                return;

            tok = PeekToken();

            bool done = false;
            for (;;)
            {
                switch (tok)
                {
                case TOK_EOL:
                    newline = true;
                    tokLen += 1;
                    current += 1;
                    done = true;
                    break;

                default: 
                    tokLen += 1;
                case TOK_SEMICOLON: 
                    if (newline)
                    {
                        newline = false;
                        line += 1;
                    }
                    current += 1;
                case TOK_END:
                    done = true;
                    break;

                case TOK_BACKSLASH:
                    // advance and treat next char as string
                    tokLen += 1;
                    current += 1;
                    if (newline)
                    {
                        newline = false;
                        line += 1;
                    }
                    // except that if it is a newline, we do need to account for that for debugging
                    if (*current == '\n')
                        newline = true;
                    // falls through

                case TOK_WHITESPACE:
                case TOK_STRING:
                    tokLen += 1;
                    if (newline)
                    {
                        newline = false;
                        line += 1;
                    }
                    current += 1;
                    break;
                }
                
                if (done)
                    break;

                Token newTok = PeekToken();
                if (newTok != tok)
                    break;
            }
        }

        Token PeekToken()
        {
            switch (*current)
            {
            case '{':  return TOK_LBRACE;
            case '}':  return TOK_RBRACE;
            case '(':  return TOK_LPAREN;
            case ')':  return TOK_RPAREN;
            case '#':  return TOK_POUND;
            case ';':  return TOK_SEMICOLON;
            case 0:    return TOK_END;
            case '\"': return TOK_QUOTE;
            case '|':  return TOK_PIPE;
            case '\n': return TOK_EOL;
            case '\\': return TOK_BACKSLASH;

            case ' ':
            case '\t':
            case '\r':
                return TOK_WHITESPACE;

            default: 
                break;
            }
            return TOK_STRING;
        }

        CommandCallback callback;
        uintptr_t userData;
        const char* current;

        Token tok;
        const char* tokStart;
        int tokLen;
        int line;
        bool newline;
    };
}

#endif // CLI_INTERP_H
