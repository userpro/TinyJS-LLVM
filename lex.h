#ifndef TINYJS_LEXER
#define TINYJS_LEXER

#include <string>
#include <cctype>
#include <map>
#include <iostream>

namespace Lexer
{
    using std::cin;
    using std::cout;
    using std::endl;

    enum class Type: char
    {
        tok_none, tok_eof,

        // commands
        tok_function,
        // primary
        tok_identifier,
        // value
        tok_integer, tok_float, tok_string, tok_single_char, tok_op, 
        tok_return, tok_break, tok_continue,

        tok_variable_declare, // var let
        tok_if, tok_for, tok_while, tok_do_while,
    };

    static std::map<Type, std::string> TokenName {
        { Type::tok_none             , "tok_none"             },
        { Type::tok_eof              , "tok_eof"              },
        // commands
        { Type::tok_function         , "tok_function"         },
        // primary
        { Type::tok_identifier       , "tok_identifier"       },
        { Type::tok_integer          , "tok_integer"          },
        { Type::tok_float            , "tok_float"            },
        { Type::tok_string           , "tok_string"           },
        { Type::tok_single_char      , "tok_single_char"      },
        { Type::tok_op               , "tok_op"               },
        { Type::tok_return           , "tok_return"           },
        { Type::tok_if               , "tok_if"               },
        { Type::tok_while            , "tok_while"            },
        { Type::tok_for              , "tok_for"              },
        { Type::tok_do_while         , "tok_do_while"         },
        { Type::tok_variable_declare , "tok_variable_declare" },
    };

    static std::map<std::string, Type> KeywordToken {
        { "function" , Type::tok_function         },
        { "if"       , Type::tok_if               },
        { "for"      , Type::tok_for              },
        { "while"    , Type::tok_while            },
        { "do"       , Type::tok_do_while         },
        { "var"      , Type::tok_variable_declare },
        { "let"      , Type::tok_variable_declare },
        { "return"   , Type::tok_return           },
        { "break"    , Type::tok_break            },
        { "continue" , Type::tok_continue         },
    };

    class Token
    {
    public:
        Type tk_type;
        std::string tk_string;

    public:
        Token() : tk_type(Type::tok_none), tk_string("") {}
        Token(Type tk_type, const std::string& tk_string) : tk_type(tk_type), tk_string(tk_string) {}
        ~Token() = default;
    };

    class LexerImpl
    {
        using IntType = unsigned long long;

    private:
        std::streambuf* StreamBackup;
        std::string CurStr;
        char LastChar;

    public:
        Token CurToken;
        IntType LineNumber;

    public:
        LexerImpl() : LexerImpl(nullptr) {}
        LexerImpl(std::streambuf* StreamPtr) : 
            CurStr(""), LastChar(' '), CurToken(Token()), LineNumber(1) 
        {
            if (StreamPtr) set_input(StreamPtr);
        }
        ~LexerImpl() { recover_input(); }

        // Copy Constructor
        LexerImpl(const LexerImpl&) = delete;
        // Assign Constructor
        const LexerImpl& operator =(const LexerImpl&) = delete;
        // Move Constructor
        LexerImpl(LexerImpl&&) = delete;
        const LexerImpl& operator =(LexerImpl&&) = delete;

        void lexer_reset() 
        {
            CurStr = "";
            LastChar = ' ';
            CurToken = Token();
            LineNumber = 1;
        }

        void set_input(std::streambuf* sptr)
        {
            StreamBackup = nullptr;
            if (sptr)
            {
                StreamBackup = cin.rdbuf();
                cin.rdbuf(sptr);
            }
        }
        void recover_input() { cin.rdbuf(StreamBackup); }

        char get_next_char() { return LastChar; }

        Token get_next_token()
        {
            // Skip space
            while (isspace(LastChar))
            {
                if (LastChar == '\n')
                    LineNumber++;
                LastChar = cin.get();
                if (cin.eof()) // End of file
                    return CurToken = Token(Type::tok_eof, "");
            }

            // Identifier
            // ([a-zA-Z_][a-zA-Z0-9_]*)
            if (isalpha(LastChar) || LastChar == '_')
            {
                CurStr = LastChar;
                while (isalnum((LastChar = cin.get())) || LastChar == '_')
                    CurStr += LastChar;

                // Is keyword?
                if (KeywordToken.find(CurStr) != KeywordToken.end())
                    return CurToken = Token(KeywordToken[CurStr], CurStr);
                return CurToken = Token(Type::tok_identifier, CurStr);
            }

            // Number
            // (-?[0-9]*.[0-9]*)
            // If previous token is not a number, maybe minus
            if (isdigit(LastChar))
            {
                CurStr = LastChar;
                while (isdigit(LastChar = cin.get()))
                    CurStr += LastChar;

                if (LastChar != '.')
                    return CurToken = Token(Type::tok_integer, CurStr);

                CurStr += LastChar;
                while (isdigit((LastChar = cin.get())))
                    CurStr += LastChar;

                return CurToken = Token(Type::tok_float, CurStr);
            }

            // String
            // ((".*?")|('.*?'))
            if (LastChar == '\'' || LastChar == '\"')
            {
                char end_char = LastChar;
                CurStr = "";
                while (true)
                {
                    if (cin.peek() == '\\')
                        CurStr += get_special_char();
                    else
                        CurStr += cin.get();
                    if (cin.peek() == end_char)
                        break;
                }
                cin.get(); // eat end_char
                LastChar = cin.get(); // pre-read a char
                return CurToken = Token(Type::tok_string, CurStr);
            }

            // Comment
            // (//.*?\n)
            if (LastChar == '/' && cin.peek() == '/')
            {
                while (cin.peek() != '\n' && cin.peek() != '\r')
                    cin.get();
                while (cin.peek() == '\n' || cin.peek() == '\r')
                {
                    LineNumber++;
                    cin.get();
                }
                LastChar = cin.get();
                return get_next_token();
            }

            // Operator
            CurStr = LastChar;
            switch (LastChar)
            {
                case '+': case '-':
                case '*': case '/':
                case '>': case '<':
                case '=': case '!':
                    double_char_op('>');
                    double_char_op('<');
                    double_char_op('=');
                    break;

                case '&':
                    double_char_op('&');
                    break;
                case '|':
                    double_char_op('|');
                    break;

                default:
                    CurToken = Token(Type::tok_single_char, CurStr);
                    break;
            }

            LastChar = cin.get(); // pre-read a char
            // Single char
            return CurToken;
        }

        void double_char_op(char c)
        {
            if (cin.peek() == c)
                CurStr += cin.get();
            CurToken = Token(Type::tok_op, CurStr);
        }

        char get_special_char()
        {
            cin.get(); // eat '\\'
            char c;
            switch ((c = cin.get()))
            {
                case 'n': return '\n';
                case 't': return '\t';
                case 'r': return '\r';
                default: return c;
            }
        }

        void print_token(const Token& t)
        {
            cout << "Token {" << endl << "  tk_type: " << TokenName[t.tk_type] << endl;
            cout << "  tk_string: " << t.tk_string << endl;
            // cout << " " << int(t.tk_string[0]) << endl;
            cout << "}" << endl;
        }
    };

}

#endif