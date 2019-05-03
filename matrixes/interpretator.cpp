#include <vector>
#include <cstdio>
#include <errno.h>
#include <stack>
#include <queue>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "matrix.h"


#define COLOR_RED "\x1b[31;1m"
#define COLOR_WHITE "\x1b[37;2m"
#define COLOR_CLEAR "\x1b[0m"
#define COLOR_GREEN "\x1b[32;1m"

using namespace std;


enum lex_type
{
    LEX_NONE, /*0*/
    LEX_DECLARE, LEX_INTEGER, LEX_FLOAT, LEX_RATIONAL, LEX_VECTOR, LEX_MATRIX, /*6*/
    LEX_PROCESS, LEX_READ, LEX_WRITE, LEX_PRINT, LEX_INFO, LEX_ROTATE, LEX_MAKE_CAN, LEX_ROW, LEX_COL, /*15*/
    LEX_FIN, /*16*/
    LEX_COLON, LEX_SEMICOLON, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH,/*25*/
    LEX_POWER, LEX_LBRACKET, LEX_RBRACKET, LEX_COMA, /*29*/
    LEX_NUM, LEX_FLOAT_NUM, LEX_RATIO_NUM, /*32*/
    LEX_STRING, /*33*/
    LEX_ID, /*34*/
    POLIZ_ASSIGN,/*35*/
    POLIZ_MTR_EL, /*36*/
    POLIZ_VEC_EL,/*37*/
    POLIZ_UNARY_MINUS /*38*/
};

vector <int> ints;
vector <double> doubles;
vector <Rational_number> ratios;
vector <Vector> vectors;
vector <Matrix> matrixes;

template<class T>
void remove_value (vector<T>& vec, int index)
{
    vec.erase(vec.begin() + index);
}

template<class T> 
int add_value (vector <T>& vec, const T& value)
{
    vec.push_back(value);
    return vec.size() - 1;
}

class Lex
{
    lex_type type;
    int value;

    int col;
    int row;
    public:
        Lex(lex_type type = LEX_NONE, int col = 0, int row = 0): type(type), value(0), col(col), row(row) {}
        Lex(lex_type type, int value, int col, int row) : type(type), value(0), col(col), row(row)
        {
            this->value = value;
        }

        Lex(const Lex& l): type(l.type), value(0)
        {
            value = l.value;
            col = l.col;
            row = l.row;
        }

        operator string();
        lex_type get_type() const { return type;}
        int get_value() const { return value;}
        
        int get_column() const {return col;}
        int get_row() const {return row;}

        void set_column(int col) {this->col = col;}
        void set_row (int row) {this->row = row;}

        void set_type (lex_type type) {this->type = type;}
        void set_value (int value) {this->value = value;}

        friend ostream& operator << (ostream& os, const Lex& lex);

};

class Error
{
    public:
        Lex lex;
        string reason;
        Error(const char* reason, Lex lex = Lex(LEX_NONE)) : lex(lex), reason(reason) {}
        Error(string reason, Lex lex = Lex(LEX_NONE)) : lex(lex), reason(reason) {}
};

class Ident
{
    string name;
    bool declare;
    lex_type type;
    bool assign;
    int value;

    public:
        Ident() : declare(false), assign(false), value(0) {}
        Ident(const string& str, lex_type type = LEX_NONE) : value (0)
        {
            name = str;
            declare = false;
            assign = false;
            this->type = type;
        }

        bool operator== (const string& str)
        {
            return name == str;
        }
        
        bool operator ==(const Ident& id)
        {
            return name == id.name;
        }
        string get_name () { return name;}
        lex_type get_type() {return type;}
        bool is_declared () { return declare;}
        bool is_assigned () { return assign;}
        int get_value() { return value;}

        
        void put_name(const string& str) { name = str;}
        void put_declare() { declare = true; }
        void put_assign() { assign = true; }
        void put_value (int rv);
        void put_type (lex_type type) {this->type = type;}
};

void Ident::put_value (int rv)
{
    value = rv;
}

vector<Ident> TID;

int put (const string& buf, lex_type type = LEX_NONE)
{
    vector<Ident>::iterator k;
    if  ((k = find(TID.begin(), TID.end(), buf)) != TID.end())
        return k - TID.begin();
    TID.push_back(Ident(buf, type));
    return TID.size() - 1;
}

vector<string> strings;

int add_string (const string& buf)
{
    vector<string>::iterator k;
    if ((k = find(strings.begin(), strings.end(), buf)) != strings.end())
        return k - strings.begin();
    strings.push_back(buf);
    return strings.size() - 1;
}

class Scanner
{
    FILE* file;
    char c;

    int row;
    int col;
    queue<Lex> returned;
    
    int look (const string& buf, const char ** list);

    void gc() 
    {
        c = fgetc(file);
        col++;
    }

    void rc()
    {
        ungetc(c,file);
        col--;
    }
    public:

        static const char* DEFINED_WORDS[], * DEFINED_SIGNS[];

        Scanner (const char* program) : row(1), col (0)
        {
            file = fopen (program, "r");
        }

        Scanner (FILE* file) : row(1), col(0)
        {
            this->file = file;
        }

        Lex get_lex();
        void return_lex (Lex& lex);
        void get_line();
};

const char*
Scanner::DEFINED_WORDS[] = {"", "declare", "integer", "float", "rational", "vector", "matrix", 
                                        "process", "read", "write", "print", "info", "rotate",
                                        "make_canonical", "row","column", NULL};

const char*
Scanner::DEFINED_SIGNS[] = {"", ":", ";", "=", "(", ")", "+", "-", "*", "/", "^", "[", "]", ",", NULL};

int Scanner::look(const string& buf, const char ** list)
{
    int i = 0;
    while (list[i])
    {
        if (buf == list[i])
            return i;
        ++i;
    }
    return 0;
}

void Scanner::return_lex(Lex& lex)
{
    returned.push(lex);
}

Lex Scanner::get_lex()
{
    enum state {H, IDENT, NUMBER, FLOAT, RATIONAL, LINE_COMMENT, COMMENT, STRING};
    string buf;
    state CS = H;
    if (!returned.empty())
    {
        Lex l = returned.front();
        returned.pop();
        return l;
    }
    do 
    {
        gc();
        switch (CS)
        {
            case H:
                if (c == ' ' || c == '\r' || c=='\t');
                else if (c == '\n')
                {
                    col = 0;
                    row++;
                }
                else if (isalpha(c) || c == '_')
                {
                    buf.push_back(c);
                    CS = IDENT;
                }
                else if (isdigit(c))
                {
                    buf.push_back(c);
                    CS = NUMBER;
                }
                else if (c == '#')
                {
                    CS = LINE_COMMENT;
                }
                
                else if (c == '/')
                {
                    gc();
                    if (c == '*')
                        CS = COMMENT;
                    else 
                    {
                        buf.push_back(c);
                        return Lex( LEX_SLASH, LEX_SLASH - LEX_FIN, col, row);
                        rc();
                    }
                }
                else if (c == EOF)
                {
                    return Lex(LEX_FIN, col, row);
                }
                else if (c == '"')
                {
                    CS = STRING;
                }
                else
                {
                    int adress;
                    buf.push_back(c);
                    if ((adress = look (buf, DEFINED_SIGNS)))
                    {
                        return Lex( (lex_type) (adress + (int) LEX_FIN), adress, col, row);
                    }
                    else
                    {
                        Lex l(LEX_NONE, col, row);
                        throw Error("Unknown character: ", l);
                    }
                }
                break;
            case IDENT:
                if (isalpha(c) || isdigit(c) || c == '_')
                {
                    buf.push_back(c);
                }
                else
                {
                    int adress;
                    rc();
                    if ((adress = look(buf, DEFINED_WORDS)))
                    {
                        return Lex((lex_type)adress, adress, col - buf.size() + 1, row);
                    }
                    else
                    {
                        adress = put(buf);
                        return Lex (LEX_ID, adress, col - buf.size() + 1, row);
                    }
                }
                break;
            case NUMBER:
                if (isdigit(c))
                {
                    buf.push_back(c);
                }
                else if (c == '.')
                {
                    buf.push_back(c);
                    CS = FLOAT;
                }
                else if (c == '/')
                {
                    buf.push_back(c);
                    CS = RATIONAL;
                }
                else
                {
                    rc();
                    int num = atoi (buf.c_str());
                    int adress = add_value(ints, num);
                    return Lex(LEX_NUM, adress, col - buf.size() + 1, row);
                }
                break;
            case FLOAT:
                {
                    int size = buf.size();
                    double num = atoi (buf.c_str());
                    double frac_part = 10;
                    if (!isdigit(c))
                    {
                        Lex l (LEX_NONE, col, row);
                        throw Error("Unexpected character: digit was expected", l);
                    }
                    while (isdigit(c))
                    {
                        num += (double)(c - '0')/ frac_part;
                        frac_part *= 10.0;
                        gc();
                    }
                    rc();
                    int adress = add_value(doubles, num);
                    return Lex(LEX_FLOAT_NUM, adress, col - buf.size() - size + 2, row);
                }
                break;
            case RATIONAL:
                {
                    if (!isdigit(c))
                        throw c;
                    while (isdigit(c))
                    {
                        buf.push_back(c);
                        gc();
                    }
                    try
                    {
                        rc();
                        Rational_number rat = buf.c_str();
                        int adress = add_value(ratios, rat);
                        return Lex (LEX_RATIO_NUM, adress, col - buf.size() + 1, row);
                    }
                    catch (Zerodivide & zr)
                    {
                        Lex l(LEX_NONE, col, row);
                        throw Error("Dividing by zero in rational number: ", l);
                    }
                    catch (NotANumber &nan)
                    {
                        Lex l(LEX_NONE, col, row);
                        throw Error("Is not a rational number: ", l);
                    }
                }
                break;
            case LINE_COMMENT:
                if (c == '\n')
                {
                    col = 0;
                    row++;
                    CS = H;
                }
                break;
            case COMMENT:
                if (c == '*')
                {
                    gc();
                    if (c == '/')
                    {
                        CS = H;
                    }
                }
                else if (c == EOF)
                {
                    Lex l (LEX_NONE, col, row);
                    throw Error("Expected '*/' instead of EOF", l);
                }
                else if (c == '\n')
                {
                    col = 0;
                    row++;
                }
                break;
            case STRING:
                if (c == '\\')
                {
                    gc();
                    if (c == 'n')
                        buf.push_back('\n');
                    else if (c == 't')
                        buf.push_back('\t');
                    else if (c == 'r')
                        buf.push_back('\r');
                    else if (c == '"')
                        buf.push_back(c);
                    else if (c == '\\')
                        buf.push_back('\\');
                    else
                    {
                        Lex l(LEX_NONE, col, row);
                        throw Error("Wrong escape character: ", l);
                    }
                }
                else if (c == '"')
                {
                    int adress = add_string(buf);
                    return Lex(LEX_STRING, adress, col - buf.size() + 1, row);
                }
                else if (c == EOF || c == '\n')
                {
                    Lex l(LEX_NONE, col, row);
                    string err = "Expected '\"' instead of: ";
                    if (c == EOF)
                        err += "EOF";
                    else err += "\\n";
                    throw Error(err, l);
                }
                else
                    buf.push_back(c);
                break;
        } // switch
            
    } // do while
    while (true);
}
Lex::operator string()
{
    string t;

    if (type == LEX_NONE)
        return t;
    else if (type <= LEX_COL)
        t = Scanner::DEFINED_WORDS[type];
    else if (type >= LEX_COLON && type <= LEX_COMA)
        t = Scanner::DEFINED_SIGNS[type - LEX_FIN];
    else if (type == LEX_NUM)
    {
        t = to_string(ints[value]);
    }
    else if (type == LEX_FLOAT_NUM)
        t = to_string(doubles[value]);
    else if (type == LEX_RATIO_NUM)
    {
        char *str = ratios[value].to_string();
        t = str;
        delete[]str;
    }
    else if (type == LEX_ID)
        t = TID[value].get_name();
    else if (type == LEX_STRING)
        t = strings[value];
    else if (type == LEX_FIN)
        t = "END";

    return t;
}
ostream& operator << (ostream& os, const Lex& lex)
{
    string t;

    if (lex.type == LEX_NONE)
        return os;
    else if (lex.type <= LEX_COL)
        t = Scanner::DEFINED_WORDS[lex.type];
    else if (lex.type >= LEX_COLON && lex.type <= LEX_COMA)
        t = Scanner::DEFINED_SIGNS[lex.type - LEX_FIN];
    else if (lex.type == LEX_NUM)
    {
        t = to_string(ints[lex.value]);
    }
    else if (lex.type == LEX_FLOAT_NUM)
        t = to_string(doubles[lex.value]);
    else if (lex.type == LEX_RATIO_NUM)
    {
        char *str = ratios[lex.value].to_string();
        t = str;
        delete[]str;
    }
    else if (lex.type == LEX_ID)
    {
        t = TID[lex.value].get_name();
        t+= " of type ";
        switch (TID[lex.get_value()].get_type())
        {
            case LEX_INTEGER:
                t += "'integer'";
                break;
            case LEX_FLOAT:
                t += "'float'";
                break;
            case LEX_RATIONAL:
                t += "'rational'";
                break;
            case LEX_MATRIX:
                t += "'matrix'";
                break;
            case LEX_VECTOR:
                t += "'vector'";
                break;
            default:
                t += "'unknown'";
                break;
        }
        os << t;
        return os;
    }
    else if (lex.type == LEX_STRING)
        t = strings[lex.value];
    else if (lex.type == LEX_FIN)
        t = "END";

    os  << "'" << t << "'";
    return os;
}

class Parser
{
    Lex current_lex;
    Scanner scan;
    stack <Lex> lexes;
    stack <Lex> ops;

    void PROGRAM();
    void DECLARATION();
    void DECLARE();
    void TYPE();
    void ID();
    void PROCESS();
    void OP();
    void OPS();
    void ASSIGN();
    void EXPRESSION();
    void EXPR();
    void UNARY();
    void ACTION();
    void EXPR2();
    void OBJECT();
    void FUNC();

    void declare (lex_type type, Lex id);
    void check_id();
    void check_op();
    void gl()
    {
        current_lex = scan.get_lex();
        //cout << current_lex;
    }
    public:
        vector<Lex> poliz;
        Parser(const char* name): scan(name) {}
        Parser(FILE* file): scan(file) {}
        void analyse();

};

void Parser::analyse()
{
    PROGRAM();
    //if (current_lex.get_type() != LEX_FIN)
      //  throw current_lex;

}

void Parser::PROGRAM()
{
    gl();
    DECLARATION();
    PROCESS();
}


void Parser::DECLARATION()
{
    if (current_lex.get_type() == LEX_DECLARE)
    {
        lexes.push(current_lex);
        gl();
        if (current_lex.get_type() != LEX_COLON)
        {
            Lex l(lexes.top());
            l.set_column(l.get_column() + 7);
            lexes.pop();
            throw Error("After \"declare\" expected ':' instead of: ", l);
        }
        gl();
        DECLARE();
     } 
    else
    {
        if (current_lex.get_type() != LEX_PROCESS)
            throw Error("Expected 'declare' or 'proccess' region instead of: ", current_lex);
        return;
    }
    gl();
    while (current_lex.get_type() != LEX_PROCESS)
    {
        DECLARE();
        gl();
    }
}

void Parser::DECLARE()
{
    lexes.push(current_lex);
    TYPE();
    gl();
    if (current_lex.get_type() != LEX_COLON)
    {
        throw Error("After type expected ':' instead of: ", current_lex);
    }
    current_lex = lexes.top();
    do
    {
        ID();
        declare(lexes.top().get_type(), current_lex);
        gl();
    }
    while(current_lex.get_type() == LEX_COMA);
    lexes.pop();
    if (current_lex.get_type() != LEX_SEMICOLON)
    {
        throw Error("Expected ';' after declation statement instead of: ", current_lex);
    }
}

void Parser::TYPE()
{
    if (current_lex.get_type() != LEX_INTEGER &&
        current_lex.get_type() != LEX_FLOAT &&
        current_lex.get_type() != LEX_RATIONAL &&
        current_lex.get_type() != LEX_VECTOR &&
        current_lex.get_type() != LEX_MATRIX)
        throw Error("Wrong type: ", current_lex);
}

void Parser::ID()
{
    gl();
    if (current_lex.get_type() != LEX_ID)
    {
        throw Error("Expected identificator instead of: ", current_lex);
    }
    Lex id = current_lex;
    gl();
    if (current_lex.get_type() == LEX_LPAREN)
    {
        poliz.push_back(id);
        gl();
        if (current_lex.get_type() == LEX_MINUS)
        {
            lexes.push(current_lex);
            gl();
        }
        if (current_lex.get_type() != LEX_NUM &&
            current_lex.get_type() != LEX_RATIO_NUM &&
            current_lex.get_type() != LEX_FLOAT_NUM &&
            current_lex.get_type() != LEX_STRING)
        {
            throw Error("Wrong argument in constructor: ", current_lex);
        }
        poliz.push_back(current_lex);
        if (!lexes.empty() && lexes.top().get_type() == LEX_MINUS)
        {
            poliz.push_back(Lex(POLIZ_UNARY_MINUS));
            lexes.pop();
        }
        poliz.push_back(Lex(POLIZ_ASSIGN));
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
        {
            throw Error("Expected ')' instead of: ", current_lex);
        }
    }
    else
        scan.return_lex(current_lex);
    current_lex = id;
}

void Parser::PROCESS()
{
    lexes.push(current_lex);
    gl();
    if (current_lex.get_type() != LEX_COLON)
    {
        lexes.top().set_column(lexes.top().get_column() + 7);
        throw Error("Expected ':' after \"process\" instead of: ", lexes.top());
        lexes.pop();
    }
    gl();
    while(current_lex.get_type() != LEX_FIN)
    {
        OPS();
        gl();
    }
}
void Parser::OPS()
{
    OP();
    while (current_lex.get_type() != LEX_SEMICOLON)
    {
        if (current_lex.get_type() == LEX_FIN)
        {
            //current_lex.set_row(current_lex.get_row() - 1);
            throw Error("Expected ';' or another operator instead of: ", current_lex);
        }
        gl();
        OP();
        if (current_lex.get_type() == LEX_SEMICOLON)
        {
            poliz.push_back(current_lex);
            break;
        }
    }
}

void Parser::OP()
{
    //gl();
    if (current_lex.get_type() == LEX_ID)
    {
        Lex id = current_lex;
        gl();
        if (current_lex.get_type() == LEX_ASSIGN)
        {
            Lex op = current_lex;
            gl();
            EXPRESSION();
            poliz.push_back(id);
            poliz.push_back(op);
        }
        else   
        {
            scan.return_lex(current_lex);
            current_lex = id;
            EXPRESSION();
        }
    }
    else if (current_lex.get_type() == LEX_INFO)
    {
        lexes.push(current_lex);
        gl();
        if (current_lex.get_type() != LEX_LPAREN)
        {
            throw Error( "\"(\" was expected instead of: ", current_lex);
        }
        gl();
        if (current_lex.get_type() != LEX_STRING)
        {
            throw Error( "Expected argument of type string instead of: ", current_lex);
        }
        poliz.push_back(current_lex);
        poliz.push_back(lexes.top());
        lexes.pop();
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
        {
            throw Error( "\")\" was expected instead of: ", current_lex);
        }
        gl();
    }
    else
        EXPRESSION();
}

void Parser::EXPRESSION()
{
    EXPR();
    while (current_lex.get_type() == LEX_PLUS ||
           current_lex.get_type() == LEX_MINUS)
    {
        Lex op = current_lex;
        gl();
        EXPR();
        poliz.push_back(op);
    }
}
void Parser::EXPR()
{
    UNARY();
    while (current_lex.get_type() == LEX_TIMES ||
           current_lex.get_type() == LEX_SLASH ||
           current_lex.get_type() == LEX_POWER)
    {
        Lex op = current_lex;
        gl();
        UNARY();
        poliz.push_back(op);
    }
}
void Parser::UNARY()
{
    if (current_lex.get_type() == LEX_MINUS)
    {
        gl();
        ACTION();
        poliz.push_back(Lex(POLIZ_UNARY_MINUS));
    }
    else
    {
        if (current_lex.get_type() == LEX_PLUS)
            gl();
        ACTION();
    }
}
void Parser::ACTION()
{
    EXPR2();
    if (current_lex.get_type() == LEX_COLON)
    {
        FUNC();
    }
}

void Parser::EXPR2()
{
    if (current_lex.get_type() == LEX_LPAREN)
    {
        gl();
        do
        {
            OP();
        }
        while (current_lex.get_type() != LEX_RPAREN);
        gl();
    }
    else
        OBJECT();
}

void Parser::OBJECT()
{
    if (current_lex.get_type() == LEX_ID)
    {
        Ident id = TID[current_lex.get_value()];
        check_id();
        if (id.get_type() == LEX_MATRIX ||
            id.get_type() == LEX_VECTOR)
        {
            lexes.push(current_lex);
            gl();
            if (current_lex.get_type() == LEX_LBRACKET)
            {
                gl();
                if (current_lex.get_type() != LEX_NUM)
                {
                    throw Error( "Expected argument of type 'integer' instead of: ", current_lex);
                }
                if (id.get_type() == LEX_MATRIX)
                {
                    Lex row = current_lex;
                    gl();
                    if (current_lex.get_type() == LEX_COMA)
                    {
                        gl();
                        if (current_lex.get_type() != LEX_NUM)
                        {
                            throw Error("Expected argument of type 'integer' instead of: ", current_lex);
                        }
                        Lex col = current_lex;
                        poliz.push_back(lexes.top());
                        poliz.push_back(Lex(POLIZ_MTR_EL,
                                            lexes.top().get_value(),
                                            col.get_value(),
                                            row.get_value()
                                           ));
                    }
                    else
                    {
                         throw Error( "Expected ',' instead of: ", current_lex);
                    }
                } // Matrix
                else
                {
                   poliz.push_back(lexes.top());
                   poliz.push_back(current_lex);
                   poliz.push_back(Lex(POLIZ_VEC_EL,
                                       lexes.top().get_value(),
                                       current_lex.get_value()
                                      ));
                } // Vector
                
                gl();
                if (current_lex.get_type() != LEX_RBRACKET)
                {
                    throw Error( "Expected ']' instead of: ", current_lex);
                }
            } // Brackets check
            else
            {
                scan.return_lex(current_lex);
                current_lex = lexes.top();
                poliz.push_back(current_lex);
            } // No brackets
            lexes.pop();
        }// Matrix or Vector
        else
        {
            poliz.push_back(current_lex);
        }
    } // ID
    else if (current_lex.get_type() == LEX_NUM)
    {
        poliz.push_back(Lex(LEX_INTEGER,
                            current_lex.get_value(),
                            current_lex.get_column(),
                            current_lex.get_row()));
    }
    else if (current_lex.get_type() == LEX_RATIO_NUM)
    {
         poliz.push_back(Lex(LEX_RATIONAL,
                            current_lex.get_value(),
                            current_lex.get_column(),
                            current_lex.get_row()));

    }
    else if(current_lex.get_type() == LEX_FLOAT_NUM)
    {
         poliz.push_back(Lex(LEX_FLOAT,
                            current_lex.get_value(),
                            current_lex.get_column(),
                            current_lex.get_row()));

    }
    else
    {
        throw Error( "Expected identificator or number instead of: ", current_lex);
    }
    gl();
}

void Parser::check_id()
{
    if (!TID[current_lex.get_value()].is_declared())
    {
        throw Error("Not declared identificator: ", current_lex);
    }
}

void Parser::declare(lex_type type, Lex id)
{
    if (TID[id.get_value()].is_declared())
        throw Error("Already declared identificator: ", id);
    TID[id.get_value()].put_declare();
    TID[id.get_value()].put_type(type);
}

void Parser::FUNC()
{
    gl();
    if (current_lex.get_type() == LEX_READ ||
        current_lex.get_type() == LEX_WRITE)
    {
        lexes.push(current_lex);
        gl();
        if (current_lex.get_type() != LEX_LPAREN)
        {
            throw Error("Excpected '(' instead of: ", current_lex);
        }
        gl();
        if (current_lex.get_type() != LEX_STRING)
        {
            throw Error("Expected argument of type string in arguments instead of: ", current_lex);
        }
        
        poliz.push_back(current_lex);
        poliz.push_back(lexes.top());
        lexes.pop();
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
        {
            throw Error("Expected ')' instead of: ", current_lex);
        }
    }
    else if (current_lex.get_type() == LEX_PRINT ||
             current_lex.get_type() == LEX_ROTATE ||
             current_lex.get_type() == LEX_MAKE_CAN)
    {
        poliz.push_back(current_lex);
    }
    else if (current_lex.get_type() == LEX_COL ||
             current_lex.get_type() == LEX_ROW)
    {
        lexes.push(current_lex);
        gl();
        if (current_lex.get_type() != LEX_LPAREN)
        {
            throw Error("Expected '(' instead of: ", current_lex);
        }
        gl();
        poliz.push_back(current_lex);
        poliz.push_back(lexes.top());
        lexes.pop();
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
        {
            throw Error("Expected ')' instead of: ", current_lex);
        }
    }
    else
    {
        throw Error("Unknown function: ", current_lex);
    }
    gl();
}

class Executer
{
    Lex current_lex;
    
    void check_assignment(Ident& lv, Lex& rv);
    Lex check_add_sub (Lex& lv,Lex& rv, char op);
    Lex check_mul (Lex& lv, Lex& rv);
    Lex check_div (Lex& lv, Lex& rv);
    
    template <class T>
    void put_value(vector<T>& vec, T& value,Ident& id);

    public:
        void execute (vector <Lex>& poliz);
};

void Executer::execute(vector <Lex>& poliz)
{
    stack <Lex> args;
    int index = 0, size = poliz.size();
    while (index < size)
    {
        current_lex = poliz[index];
        switch(current_lex.get_type())
        {
            case LEX_NUM: case LEX_RATIO_NUM: case LEX_FLOAT_NUM: case LEX_ID:
            case LEX_INTEGER: case LEX_FLOAT: case LEX_RATIONAL: case LEX_STRING:
                args.push(current_lex);
                break; // Operands
            case POLIZ_ASSIGN:
                {
                    int adress;
                    Lex arg = args.top();
                    args.pop();
                    Lex id = args.top();
                    args.pop();
                    if (id.get_type() != LEX_ID)
                    {
                        throw Error("Expected identificator for initialization: ",
                                     current_lex);
                    }
                    switch(TID[id.get_value()].get_type())
                    {
                        case LEX_MATRIX:
                            if (arg.get_type() != LEX_STRING)
                            {
                                throw Error("Expected file-name for matrix initialization instead of: ",
                                            current_lex);
                            }
                            try
                            {
                                adress = add_value(matrixes, 
                                                   Matrix(strings[arg.get_value()].c_str()));
                            }
                            catch(OpenFileError &of)
                            {
                                throw Error("Can't open file: ", arg);
                            }
                            break;
                        case LEX_VECTOR:
                            if (arg.get_type() != LEX_STRING)
                            {
                                throw Error("Expected file-name for vector initialization instead of: ",
                                            current_lex);
                            }
                            try
                            {
                                adress = add_value(vectors,
                                                   Vector(strings[arg.get_value()].c_str()));
                            }
                            catch (OpenFileError &of)
                            {
                                throw Error("Can't open file: ",arg);
                            }
                            break;
                        case LEX_INTEGER:
                            if (arg.get_type() != LEX_NUM)
                            {
                                throw Error("Expected number for integer initialization instead of: ",
                                            arg);
                            }
                            adress = arg.get_value();
                            break;
                        case LEX_FLOAT:
                            if (arg.get_type() != LEX_FLOAT_NUM)
                            {
                                throw Error("Expected pointed number for float initialization instead of:", 
                                            arg);
                            }
                            adress = arg.get_value();
                            break;
                        case LEX_RATIONAL:
                            if (arg.get_type() == LEX_RATIO_NUM)
                            {
                                adress = arg.get_value();
                            }
                            else if (arg.get_type() == LEX_NUM)
                            {
                                Rational_number i = ints[arg.get_value()];
                                adress = add_value(ratios, i);
                            }
                            else if (arg.get_type() == LEX_FLOAT_NUM)
                            {
                                Rational_number i = doubles[arg.get_value()];
                                adress = add_value(ratios, i);
                            }
                            else
                            {
                                throw Error("Expected number for initialization of rational number instead of",
                                            arg);
                            }
                            break;
                        default:
                            {
                                throw Error("Unrnown identificatior type: ", id);
                            }
                            break;
                    }
                    TID[id.get_value()].put_value(adress);
                    TID[id.get_value()].put_assign();
                } // POLIZ_ASSIGN
                break;
            case LEX_SEMICOLON:
                args.pop();
                break;
            case POLIZ_UNARY_MINUS:
                {
                    Lex op = args.top();
                    switch(op.get_type())
                    {
                        case LEX_ID:
                            {
                                Ident id = TID[op.get_value()];
                                if(id.get_type() == LEX_INTEGER)
                                    ints[id.get_value()] = - ints[id.get_value()];
                                else if (id.get_type() == LEX_RATIONAL)
                                    ratios[id.get_value()] = - ratios[id.get_value()];
                                else if (id.get_value() == LEX_FLOAT)
                                    doubles[id.get_value()] = - doubles[id.get_value()];
                                else if (id.get_type() == LEX_VECTOR)
                                    vectors[id.get_value()] = (Rational_number) (-1)*vectors[id.get_value()];
                                else if (id.get_type() == LEX_MATRIX)
                                    matrixes[id.get_value()] = (Rational_number)(-1)*matrixes[id.get_value()];
                                else 
                                {
                                    throw Error("Unknown identificator type: ", op);
                                }
                            }
                            break;
                        case LEX_NUM: case LEX_INTEGER:
                            ints[op.get_value()] = - ints[op.get_value()];
                            break;
                        case LEX_RATIO_NUM: case LEX_RATIONAL:
                            ratios[op.get_value()] = - ratios[op.get_value()];
                            break;
                        case LEX_FLOAT_NUM: case LEX_FLOAT:
                            doubles[op.get_value()] = - doubles[op.get_value()];
                            break;
                        case LEX_MATRIX:
                            matrixes[op.get_value()] = (Rational_number)(-1)*matrixes[op.get_value()];
                            break;
                        case LEX_VECTOR:
                            vectors[op.get_value()] = (Rational_number)(-1)*vectors[op.get_value()];
                            break;
                        default:
                            throw Error("Unknown operand type: ", op);
                            break;
                    }
                } // POLIZ_UNARY_MINUS
                break;
            case LEX_ASSIGN:
                {
                    Lex l_id = args.top();
                    args.pop();
                    Lex exp = args.top();
                    if (l_id.get_type() != LEX_ID)
                    {
                        throw Error("Expected identificator as left operand for assigment operator instead of: ", l_id);
                    }
                    Ident id = TID[l_id.get_value()];
                    args.pop();
                    try
                    {
                        check_assignment(TID[l_id.get_value()], exp);
                    }
                    catch(const char* err)
                    {
                        throw Error(err, l_id);
                    }
                    catch(Error& err)
                    {
                        throw err;
                    }
                    args.push(Lex(TID[l_id.get_value()].get_type(), TID[l_id.get_value()].get_value(), l_id.get_column(), l_id.get_row()));
                } // LEX_ASSIGN
                break;
            case LEX_PLUS:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    args.pop();

                    try
                    {
                        Lex res = check_add_sub(lv, rv, '+');
                        args.push(res);
                    }
                    catch (Overflow &of)
                    {
                        throw Error("Caught overflow of two operands: ",
                                    Lex(LEX_NONE,lv.get_column(), lv.get_row()));
                    }
                } // LEX_PLUS
                break;
            case LEX_MINUS:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    args.pop();
                    try
                    {
                        Lex res = check_add_sub(lv, rv, '-');
                        args.push(res);

                    }
                    catch (Overflow &of)
                    {
                        throw Error("Caught overflow of two operands: ",
                                    Lex(LEX_NONE,lv.get_column(), lv.get_row()));

                    }
                } // LEX_MINUS
                break;
            case LEX_TIMES:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    try
                    {
                        Lex res = check_mul(lv, rv);
                        args.push(res);
                    }
                    catch (Overflow &of)
                    {
                        throw Error("Caught overflow of two operands: ",
                                    Lex(LEX_NONE,lv.get_column(), lv.get_row()));
                    }
                } // LEX_TIMES
                break;
            case LEX_SLASH:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    args.pop();
                    try
                    {
                        Lex res = check_div(lv, rv);
                        args.push(res);
                    }
                    catch (Overflow &of)
                    {
                        throw Error("Caught overflow of two operands: ",
                                    Lex(LEX_NONE,lv.get_column(), lv.get_row()));
                    }
                } // LEX_SLASH
                break;
            case LEX_POWER:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    args.pop();
                    if (rv.get_type() == LEX_ID)
                    {
                        Ident id = TID[rv.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", rv);
                        rv = Lex(id.get_type(), id.get_value(), rv.get_column(), rv.get_row());
                    }
                    if (lv.get_type() == LEX_ID)
                    {
                        Ident& id = TID[lv.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", lv);
                        lv = Lex(id.get_type(), id.get_value(), lv.get_column(), lv.get_row());
                    }
                    if (lv.get_type() != LEX_MATRIX)
                    {
                        throw Error("Expected left operand of type 'matrix' instead of: ", lv);
                    }
                    if (rv.get_type() != LEX_INTEGER)
                    {
                        throw Error("Expected right operand of type 'integer' instead of: ", rv);
                    }
                    try
                    {
                        Matrix result = matrixes[lv.get_value()] ^ ints[rv.get_value()];
                        
                        int adress = add_value(matrixes, result);
                        Lex res(LEX_MATRIX, adress, lv.get_column(), lv.get_row());

                        args.push(res);
                    }
                    catch (WrongMatrixSize & wrm)
                    {
                        throw Error("Matrix must have equal columns and rows: ", lv);
                    }
                    catch (Overflow& of)
                    {
                        throw Error("Caught overflow of two operands: ",
                                    Lex(LEX_NONE,lv.get_column(), lv.get_row()));
                    }
                } // LEX_INFO
                break;
            case LEX_INFO:
                {
                    Lex op = args.top();
                    args.pop();
                    if (op.get_type() != LEX_STRING)
                    {
                        throw Error("Expected argument of type 'string' instead of: ", op);
                    }
                    cout << strings[op.get_value()];
                } // LEX_INFO
                break;
            case LEX_PRINT:
                {
                    Lex op = args.top();
                    args.pop();
                    if (op.get_type() == LEX_ID)
                    {
                        Ident id = TID[op.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", op);
                        op = Lex(id.get_type(), id.get_value(), op.get_column(), op.get_row());
                    }
                    switch(op.get_type())
                    {
                        case LEX_MATRIX:
                            {
                                char* str = matrixes[op.get_value()].to_string();
                                cout << str << endl;
                                delete[]str;
                            }
                            break;
                        case LEX_VECTOR:
                            {
                                char* str = vectors[op.get_value()].to_string();
                                cout << str << endl;
                                delete[]str;
                            }
                            break;
                        case LEX_RATIONAL:
                            {
                                char* str = ratios[op.get_value()].to_string();
                                cout << str << endl;
                                delete[]str;
                            }
                            break;
                        case LEX_FLOAT:
                            cout << doubles[op.get_value()] << endl;
                            break;
                        case LEX_INTEGER:
                            cout << ints[op.get_value()] << endl;
                            break;
                        default:
                            throw Error("Unknown operand type: ", op);
                            break;
                    }
                } // LEX_PRINT
                break;
            case LEX_COL:
                {
                    Lex col = args.top();
                    args.pop();
                    Lex mtr = args.top();
                    args.pop();

                    if (col.get_type() == LEX_ID)
                    {
                        Ident id = TID[col.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", col);
                        col = Lex(id.get_type(), id.get_value(), col.get_column(), col.get_row());
                    }
                    if (mtr.get_type() == LEX_ID)
                    {
                        Ident id = TID[mtr.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", mtr);
                        mtr = Lex(id.get_type(), id.get_value(), mtr.get_column(), mtr.get_row());
                    }

                    if (col.get_type() != LEX_INTEGER && col.get_type() != LEX_NUM)  
                        throw Error("Expected argument of type 'integer' instead of: ", col);
                    if (mtr.get_type() != LEX_MATRIX) 
                        throw Error("Expected operand of type 'matrix' instead of: ", mtr);

                    Matrix_col_coord coord(ints[col.get_value()]);
                    int adress;
                    try
                    {
                        adress = add_value(vectors, matrixes[mtr.get_value()][coord]);
                    }
                    catch (OutOfRangeMatrix & orf)
                    {
                        throw Error("Column is out of matrix range: ", col);
                    }

                    args.push(Lex(LEX_VECTOR, adress, mtr.get_column(), mtr.get_row()));
                } //LEX_COL
                break;
            case LEX_ROW:
                {
                    Lex row = args.top();
                    args.pop();
                    Lex mtr = args.top();
                    args.pop();

                    if (row.get_type() == LEX_ID)
                    {
                        Ident id = TID[row.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", row);
                        row = Lex(id.get_type(), id.get_value(), row.get_column(), row.get_row());
                    }
                    if (mtr.get_type() == LEX_ID)
                    {
                        Ident id = TID[mtr.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", mtr);
                        mtr = Lex(id.get_type(), id.get_value(), mtr.get_column(), mtr.get_row());
                    }

                    if (row.get_type() != LEX_INTEGER && row.get_type() != LEX_NUM) 
                        throw Error("Expected argument of type 'integer' instead of: ", row);
                    if (mtr.get_type() != LEX_MATRIX) 
                        throw Error("Expected operand of type 'matrix' instead of: ", mtr);

                    Matrix_row_coord coord(ints[row.get_value()]);
                    int adress;
                    try
                    {
                        adress = add_value(vectors, matrixes[mtr.get_value()][coord]);
                    }
                    catch (OutOfRangeMatrix & orf)
                    {
                        throw Error("Row is out of matrix range: ", row);
                    }

                    args.push(Lex(LEX_VECTOR, adress, mtr.get_column(), mtr.get_row()));
                } // LEX_ROW
                break;
            case LEX_ROTATE:
                {
                    Lex op = args.top();
                    args.pop();
                    if (op.get_type() == LEX_ID)
                    {
                        Ident id = TID[op.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", op);
                        op = Lex(id.get_type(), id.get_value(), op.get_column(), op.get_row());
                    }
                    if (op.get_type() == LEX_MATRIX)
                    {
                        Matrix tr = ~matrixes[op.get_value()];
                        int adress = add_value(matrixes, tr);
                        args.push(Lex(LEX_MATRIX, adress, op.get_column(), op.get_row()));
                    }
                    else if (op.get_type() == LEX_VECTOR)
                    {
                        Matrix vert(vectors[op.get_value(), Vertical]);
                        int adress = add_value(matrixes, vert);
                        args.push(Lex(LEX_MATRIX, adress, op.get_column(), op.get_row()));
                    }
                    else
                    {
                        throw Error("Expected 'vector' or 'matrix' as operand instead of: ", op);
                    }
                } // LEX_ROTATE
                break;
            case LEX_WRITE:
                {
                    Lex str = args.top();
                    args.pop();
                    Lex op = args.top();
                    args.pop();

                    if (str.get_type() != LEX_STRING)
                    {
                        throw Error("Expected argument of type 'string' instead of: ", str);
                    }
                    if (op.get_type() == LEX_ID)
                    {
                        Ident id = TID[op.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", op);
                        op = Lex(id.get_type(), id.get_value(), op.get_column(), op.get_row());
                    }
                    if (op.get_type() == LEX_MATRIX)
                    {
                        try
                        {
                            matrixes[op.get_value()].write(strings[str.get_value()].c_str());
                        }
                        catch(OpenFileError & of)
                        {
                            throw Error("Can't open file: ", str);
                        }
                    }
                    else if (op.get_type() == LEX_VECTOR)
                    {
                        try
                        {
                            vectors[op.get_value()].write(strings[str.get_value()].c_str());
                        }
                        catch (OpenFileError &of)
                        {
                            throw Error("Can't open file: ", str);
                        }
                    }
                    else
                    {
                        FILE* file = fopen(strings[str.get_value()].c_str(), "wa");
                        if (file == NULL)
                        {
                            throw Error("Can't open file: ", str);
                        }
                        if (op.get_type() == LEX_INTEGER)
                        {
                            fprintf(file, "%d\n", ints[op.get_value()]);
                        }
                        else if (op.get_type() == LEX_FLOAT)
                        {
                            fprintf(file, "%lf\n", doubles[op.get_value()]);
                        }
                        else if (op.get_type() == LEX_RATIONAL)
                        {
                            char *str = ratios[op.get_value()].to_string();
                            fprintf(file, "%s\n", str);
                            delete[]str;
                        }
                        else 
                        {
                            throw Error("Unknown operand type: ", op);
                        }
                    }
                } // LEX_WRITE
                break; 
            case LEX_READ:
                {
                    Lex str = args.top();
                    args.pop();
                    Lex op = args.top();
                    args.pop();
                    if (str.get_type() != LEX_STRING)
                    {
                        throw Error("Expected argument of type 'string' instead of: ", str);
                    }
                    if (op.get_type() != LEX_ID)
                    {
                        throw Error("Expected identeficator for 'read' fuction instead of: ", op);
                    }
                    Ident &id = TID[op.get_value()];
                    switch(id.get_type())
                    {
                        case LEX_MATRIX:
                            {
                                try
                                {
                                    Matrix mtr(strings[str.get_value()].c_str());
                                    put_value(matrixes, mtr, id);
                                }
                                catch(OpenFileError &ofr)
                                {
                                    throw Error("Can't open file: ", str);
                                }
                                catch(WrongLexeme & wr)
                                {
                                    throw Error("Can't read matrixin file: ", str);
                                }
                            }
                            break;
                        case LEX_VECTOR:
                            {
                                try
                                {
                                    Vector vec(strings[str.get_value()].c_str());
                                    put_value(vectors, vec, id);
                                }
                                catch (OpenFileError &of)
                                {
                                    throw Error("Can't open file: ", str);
                                }
                                catch (WrongLexeme& wr)
                                {
                                    throw Error("Can't read vectort in file: ", str);
                                }
                            }
                            break;
                        case LEX_RATIONAL:
                            {
                                FILE* file = fopen(strings[str.get_value()].c_str(), "r");
                                if (file == NULL)
                                {
                                    throw Error("Can't open file: ", str);
                                }
                                Scanner scan(file);
                                Lex l = scan.get_lex();
                                if (l.get_type() != LEX_RATIO_NUM)
                                {
                                    throw Error("Can't read rational number in file: ", str);
                                }
                                put_value(ratios, ratios[l.get_value()], id);
                                remove_value(ratios, l.get_value());
                                fclose(file);
                            }
                            break;
                        default:
                            {
                                FILE* file = fopen(strings[str.get_value()].c_str(), "r");
                                if (file == NULL)
                                {
                                    throw Error("Can't open file: ", str);
                                }
                                if (id.get_type() == LEX_INTEGER)
                                {
                                    int val;
                                    errno = 0;
                                    fscanf(file, "%d", &val);
                                    if (errno) 
                                        throw Error("Can't read integer in file: ", str);
                                    put_value(ints, val, id);
                                }
                                if (id.get_type() == LEX_FLOAT)
                                {
                                    double val;
                                    errno = 0;
                                    fscanf(file, "%lf", &val);
                                    if (errno)
                                        throw Error("Can't read float in file: ", str);
                                    put_value(doubles, val, id);
                                }
                                else 
                                    throw op;
                            }
                            break;
                    }
                    args.push(op);
                } // LEX_READ
                break;
            case POLIZ_MTR_EL:
                {
                    Lex mtr = args.top();
                    args.pop();
                    if (mtr.get_type() == LEX_ID)
                    {
                        Ident id = TID[mtr.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", mtr);
                        mtr = Lex(id.get_type(), id.get_value(), mtr.get_column(), mtr.get_row());
                    }

                    if (mtr.get_type() != LEX_MATRIX)
                    {
                        throw Error("Excepted operand of type 'matrix' instead of: ", mtr);
                    }
                    int col = current_lex.get_column();
                    int row = current_lex.get_row();

                    Matrix_coords coords(row, col);
                    
                    try
                    {
                        Rational_number ratio = matrixes[mtr.get_value()][coords];
                        int adress = add_value(ratios, ratio);

                        args.push(Lex(LEX_RATIONAL, adress, mtr.get_column(), mtr.get_row()) );
                    }
                    catch (OutOfRangeMatrix &ofm)
                    {
                        string err = "Out of range matrix of row: " + to_string(row) + "and col: " + to_string(col);
                        throw Error(err, mtr);
                    }
                } // POLIZ_MTR_EL
                break;
            case POLIZ_VEC_EL:
                {
                    Lex vec = args.top();
                    args.pop();
                    if (vec.get_type() == LEX_ID)
                    {
                        Ident id = TID[vec.get_value()];
                        if (!id.is_assigned()) throw vec;
                        vec = Lex(id.get_type(), id.get_value(), vec.get_column(), vec.get_row());
                    }
                    int coord = current_lex.get_column();
                    if (vec.get_type() != LEX_VECTOR)
                        throw vec;
                    try
                    {
                        Rational_number ratio = vectors[vec.get_value()][coord];
                        int adress = add_value(ratios, ratio);

                        args.push(Lex(LEX_RATIONAL, adress, vec.get_column(), vec.get_row()));
                    }
                    catch (OutOfRangeVector & ofv)
                    {
                        string err = "Out of range vector of element: " + to_string(coord);
                        throw Error(err, vec);
                    }
                } // POLIZ_VEC_EL
                break;
            case LEX_MAKE_CAN:
                {
                    Lex op = args.top();
                    args.pop();
                    if (op.get_type() == LEX_ID)
                    {
                        Ident id = TID[op.get_value()];
                        if (!id.is_assigned()) 
                            throw Error("Not assigned identificator: ", op);
                        op = Lex (id.get_type(), id.get_value(), op.get_column(), op.get_row());
                    }
                    if (op.get_type() == LEX_RATIONAL)
                    {
                        ratios[op.get_value()].make_canonical();
                        args.push(op);
                    }
                    //else if (op.get_type() == LEX_MATRIX) WTF??? 
                    //else if (op.get_type() == LEX_VECTOR) WTF???
                    else
                    {
                        throw Error("Operand can'be made canonical: ", op);
                    }
                }
                break;
            default:
                throw Error("Unknow lexeme: ", current_lex);
                break;
        } //switch
        index++;
    }
}

template <class T>
void Executer::put_value(vector<T>& vec, T& value, Ident& id)
{
    if (id.is_assigned())
        vec[id.get_value()] = value;
    else
    {
        int adress = add_value(vec, value);
        id.put_value(adress);
        id.put_assign();
    }
}

void Executer::check_assignment(Ident& lv, Lex& rv)
{
    if (rv.get_type() == LEX_ID)
    {
        Ident id = TID[rv.get_value()];
        if (!id.is_assigned())
            throw Error("Not assigned identificator: ", rv);
        rv = Lex(id.get_type(), id.get_value(), rv.get_column(), rv.get_row());
    }
    switch(lv.get_type())
    {
        case LEX_INTEGER:
                {
                    switch(rv.get_type())
                    {
                        case LEX_INTEGER: case LEX_NUM:
                            put_value(ints, ints[rv.get_value()], lv);
                            break;
                        case LEX_RATIONAL: case LEX_RATIO_NUM:
                            {
                                int i = (int)ratios[rv.get_value()];
                                put_value(ints, i, lv);
                            }
                            break;
                        case LEX_FLOAT: case LEX_FLOAT_NUM:
                            {
                                int i = doubles[rv.get_value()];
                                put_value(ints, i, lv);
                            }
                            break;
                        default:
                            throw Error("Wrong right operand type. Expected number instead of: ", rv);
                            break;
                    }
                }
                break;
        case LEX_RATIONAL: 
                {
                    switch(rv.get_type())
                    {
                        case LEX_INTEGER: case LEX_NUM:
                            {
                                Rational_number i = ints[rv.get_value()];
                                put_value(ratios, i, lv);
                            }
                            break;
                        case LEX_RATIONAL: case LEX_RATIO_NUM:
                            {
                                Rational_number rat = ratios[rv.get_value()];
                                put_value(ratios, rat, lv);
                            }
                            break;
                        case LEX_FLOAT: case LEX_FLOAT_NUM:
                            {
                                Rational_number i = doubles[rv.get_value()];
                                put_value(ratios, i, lv);
                            }
                            break;
                        default:
                            throw Error("Wrong right operand type. Expected number instead of: ", rv);
                            break;
                    }
                }
                break;
        case LEX_FLOAT:
                {
                    switch(rv.get_type())
                    {
                        case LEX_INTEGER: case LEX_NUM:
                            {
                                double i = ints[rv.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        case LEX_RATIONAL: case LEX_RATIO_NUM:
                            {
                                double i = (double)ratios[rv.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        case LEX_FLOAT: case LEX_FLOAT_NUM:
                            {
                                double i = doubles [rv.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        default:
                            throw Error("Wrong right operand. Expected number instead of: ", rv);
                            break;
                    }
                }
                break;
        case LEX_VECTOR:
                {
                    if (rv.get_type() != LEX_VECTOR)
                        throw Error("Wrong right operand. Expected 'vector' instead of: ", rv);
                    put_value(vectors, vectors[rv.get_value()], lv);
                }
                break;
        case LEX_MATRIX:
                {
                    if (rv.get_type() == LEX_VECTOR)
                    {
                        Matrix mtr = Matrix(vectors[rv.get_value()], Horizontal);
                        put_value(matrixes, mtr, lv);
                    }
                    else if (rv.get_type() == LEX_MATRIX)
                        put_value(matrixes, matrixes[rv.get_value()], lv);
                    else
                        throw Error("Wrong right operand. Expected 'matrix' or 'vector' instead of: ", rv);

                }
                break;
         default:
                throw "Unknown type of left operand: ";
                break;         
    }//switch

}

Lex Executer::check_add_sub(Lex& lv,Lex& rv, char op)
{
    Lex res;
    int adress;
    if (lv.get_type() == LEX_ID)
    {
        Ident id = TID[lv.get_value()];
        if (!id.is_assigned())
            throw Error("Not assigned identificator: ", lv);
        lv = Lex(id.get_type(), id.get_value(), lv.get_column(), lv.get_row());
    }
    if (rv.get_type() == LEX_ID)
    {
        Ident id = TID[rv.get_value()];
        if (!id.is_assigned())
            throw Error("Not assigned identificator: ", rv);
        rv = Lex(id.get_type(), id.get_value(), lv.get_column(), lv.get_row());
    }
    switch(lv.get_type())
    {
        case LEX_INTEGER:
            {
                int result = ints[lv.get_value()];
                switch(rv.get_type())
                {
                    case LEX_INTEGER:
                         if (op == '+')
                            result += ints[rv.get_value()];
                        else if (op == '-')
                            result -= ints[rv.get_value()];
                        break;
                        
                        break;
                    case LEX_FLOAT:
                        if (op == '+')
                            result += doubles[rv.get_value()];
                        else if (op == '-')
                            result -= doubles[rv.get_value()];
                        break;
                        
                    case LEX_RATIONAL:
                        if (op == '+')
                            result += (int) ratios[rv.get_value()];
                        else if(op == '-')
                            result -= (int) ratios[rv.get_value()];
                        break;
                    default:
                        throw Error("Wrong right operand. Expected number instead of: ", rv);
                        break;
                }

                adress = add_value(ints, result);
                res.set_type(LEX_INTEGER);
                res.set_value(adress);

            }
            break;
        case LEX_FLOAT:
            {
                double result = doubles[lv.get_value()];
                switch(rv.get_type())
                {
                    case LEX_INTEGER:
                         if (op == '+')
                            result += ints[rv.get_value()];
                        else if (op == '-')
                            result -= ints[rv.get_value()];
                        break;
                        
                        break;
                    case LEX_FLOAT:
                        if (op == '+')
                            result += doubles[rv.get_value()];
                        else if (op == '-')
                            result -= doubles[rv.get_value()];
                        break;
                        
                    case LEX_RATIONAL:
                        if (op == '+')
                            result += (double) ratios[rv.get_value()];
                        else if(op == '-')
                            result -= (double) ratios[rv.get_value()];
                        break;
                    default:
                        throw Error("Wrong right operand. Expected number instead of: ", rv);
                        break;
                }

                adress = add_value(doubles, result);
                res.set_type(LEX_FLOAT);
                res.set_value(adress);
            }
            break;
        case LEX_RATIONAL:
            {
                Rational_number result = ratios[lv.get_value()];
                switch(rv.get_type())
                {
                    case LEX_INTEGER:
                         if (op == '+')
                            result += ints[rv.get_value()];
                        else if (op == '-')
                            result -= ints[rv.get_value()];
                        break;
                        
                        break;
                    case LEX_FLOAT:
                        if (op == '+')
                            result += doubles[rv.get_value()];
                        else if (op == '-')
                            result -= doubles[rv.get_value()];
                        break;
                        
                    case LEX_RATIONAL:
                        if (op == '+')
                            result += ratios[rv.get_value()];
                        else if(op == '-')
                            result -= ratios[rv.get_value()];
                        break;
                    default:
                        throw Error("Wrong right operand. Expected number instead of: ", rv);
                        break;
                }

                adress = add_value(ratios, result);
                res.set_type(LEX_RATIONAL);
                res.set_value(adress);
            }
            break;
        case LEX_MATRIX:
            {
                Matrix result = matrixes[lv.get_value()];
                if (rv.get_type() == LEX_VECTOR)
                {
                    try
                    {
                        if (op == '+')
                            result += Matrix(vectors[rv.get_value()], Horizontal);
                        else if (op == '-')
                            result -= Matrix(vectors[rv.get_value()], Horizontal);
                    }
                    catch(WrongMatrixSize& wrm)
                    {   
                        throw Error("Left matrix don't have equal size to right vector: ", rv);
                    }
                }
                else if (rv.get_type() == LEX_MATRIX)
                {
                    try
                    {
                        if (op == '+')
                            result += matrixes[rv.get_value()];
                        else if (op == '-')
                            result -= matrixes[rv.get_value()];
                    }
                    catch(WrongMatrixSize &wrm)
                    {
                        throw Error("Right matrix don't have equal size to left one: ", rv);
                    }
                }
                else 
                    throw Error("Expected right operand of type 'vector' or 'matrix' instead of: ", rv);
                adress = add_value(matrixes, result);
                res.set_type(LEX_MATRIX);
                res.set_value(adress);
            }
            break;
        case LEX_VECTOR:
            {
                Vector result = vectors[lv.get_value()];
                if (rv.get_type() == LEX_VECTOR)
                {
                    try
                    {
                        if (op == '+')
                            result += vectors[rv.get_value()];
                        else if (op == '-')
                            result -= vectors[rv.get_value()];
                    }
                    catch(WrongVectorSize& wrv)
                    {   
                        throw Error("Not equal size of right vector to left one: ", rv);
                    }
                }
                else
                    throw Error("Expected right operand of type 'vector' instead of: ", rv);
                adress = add_value(vectors, result);
                res.set_type(LEX_VECTOR);
                res.set_value(adress);
            }
            break;
        default:
            throw Error("Unknown type of left operand: ", lv);
            break;
    }// switch for lv
    res.set_column(lv.get_column());
    res.set_row(lv.get_row());

    return res;
}

Lex Executer::check_mul(Lex& lv, Lex& rv)
{
    Lex res;
    int adress;
    if (lv.get_type() == LEX_ID)
    {
        Ident id = TID[lv.get_value()];
        if (!id.is_assigned())
            throw Error("Not assigned identificator: ", lv);
        lv = Lex(id.get_type(), id.get_value(), lv.get_column(), lv.get_row());
    }

    if (rv.get_type() == LEX_ID)
    {
        Ident id = TID[rv.get_value()];
        if (!id.is_assigned())
            throw Error("Not assigned identificator: ", rv);
        rv = Lex(id.get_type(), id.get_value(), rv.get_column(), rv.get_row());
    }

    switch(lv.get_type())
    {
        case LEX_INTEGER:
            {
                switch (rv.get_type())
                {
                    case LEX_INTEGER:
                        {
                            int result = ints[lv.get_value()] * ints[rv.get_value()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            int result = ints[lv.get_value()] * doubles[rv.get_type()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {
                            int result = ints[lv.get_value()] * (int)ratios[rv.get_type()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_MATRIX:
                        {
                            Matrix result = (Rational_number)ints[lv.get_value()] * matrixes[rv.get_value()];
                            adress = add_value(matrixes, result);
                            res.set_type(LEX_MATRIX);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_VECTOR:
                        {
                            Vector result = (Rational_number)ints[lv.get_value()]*vectors[rv.get_value()];
                            adress = add_value(vectors, result);
                            res.set_type(LEX_VECTOR);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw Error("Unknown type of right operand: ", rv);
                        break;

                } //switch fo rv
                break;
            } // LEX_INTEGER
        case LEX_FLOAT:
            switch (rv.get_type())
                {
                    case LEX_INTEGER:
                        {
                            double result = doubles[lv.get_value()] * ints[rv.get_value()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            double result = doubles[lv.get_value()] * doubles[rv.get_type()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {
                            double result = doubles[lv.get_value()] * (double)ratios[rv.get_type()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_MATRIX:
                        {
                            Matrix result = (Rational_number)doubles[lv.get_value()] * matrixes[rv.get_value()];
                            adress = add_value(matrixes, result);
                            res.set_type(LEX_MATRIX);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_VECTOR:
                        {
                            Vector result = (Rational_number)doubles[lv.get_value()]*vectors[rv.get_value()];
                            adress = add_value(vectors, result);
                            res.set_type(LEX_VECTOR);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw Error("Unknown type of right operand: ", rv);
                        break;

                } //switch fo rv
            break; // LEX_FLOAT
        case LEX_RATIONAL:
            switch (rv.get_type())
                {
                    case LEX_INTEGER:
                        {
                            Rational_number result = ratios[lv.get_value()] * ints[rv.get_value()];
                            adress = add_value(ratios, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            Rational_number result = ratios[lv.get_value()] * doubles[rv.get_type()];
                            adress = add_value(ratios, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {
                            Rational_number result = ratios[lv.get_value()] * ratios[rv.get_type()];
                            adress = add_value(ratios, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_MATRIX:
                        {
                            Matrix result = ratios[lv.get_value()] * matrixes[rv.get_value()];
                            adress = add_value(matrixes, result);
                            res.set_type(LEX_MATRIX);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_VECTOR:
                        {
                            Vector result = ratios[lv.get_value()]*vectors[rv.get_value()];
                            adress = add_value(vectors, result);
                            res.set_type(LEX_VECTOR);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw Error("Unknown type of right operand: ", rv);
                        break;

                } //switch fo rv
            break; // LEX_RATIONAL
        case LEX_VECTOR:
            {
                if (rv.get_type() == LEX_VECTOR)
                {
                    Rational_number result;
                    try
                    {
                        result = vectors[lv.get_value()] * vectors[rv.get_value()];
                    }
                    catch (WrongVectorSize& wrv)
                    {
                        throw Error("Size of right vector must be equal to left one: ", rv);
                    }
                    adress = add_value(ratios, result);
                    res.set_type(LEX_RATIONAL);
                    res.set_value(adress);
                }
                else if (rv.get_type() == LEX_MATRIX)
                {
                    Matrix result = Matrix(vectors[lv.get_value()], Horizontal);
                    try
                    {
                        result *= matrixes[rv.get_value()];
                    }
                    catch (WrongMatrixSize& wrm)
                    {
                        throw Error("Number of right matrix columns must be 1: ", rv);
                    }
                    adress = add_value(matrixes, result);
                    res.set_type(LEX_MATRIX);
                    res.set_value(adress);
                }
                else
                {
                    Vector result = vectors[lv.get_value()];
                    switch(rv.get_type())
                    {
                        case LEX_INTEGER:
                            result *= ints[rv.get_value()];
                            break;
                        case LEX_FLOAT:
                            result *= doubles[rv.get_value()];
                            break;
                        case LEX_RATIONAL:
                            result *= ratios[rv.get_value()];
                            break;
                        default:
                            throw Error("Unknown type of right operand: ", rv);
                            break;
                    }

                    adress = add_value(vectors, result);
                    res.set_type(LEX_VECTOR);
                    res.set_value(adress);
                }
            }
            break; // LEX_VECTOR
        case LEX_MATRIX:
            {
                Matrix result = matrixes[lv.get_value()];
                if (rv.get_type() == LEX_VECTOR)
                {
                    try
                    {
                        result *= Matrix(vectors[rv.get_value()], Horizontal);
                    }
                    catch (WrongMatrixSize& wrm)
                    {
                        throw Error("Size of right vector must be equal to number of rows of left matrix: ",
                                    rv);
                    }
                }
                else if (rv.get_type() == LEX_MATRIX)
                {
                    try
                    {
                        result *= matrixes[rv.get_value()];
                    }
                    catch (WrongMatrixSize& wrm)
                    {
                        throw Error("Size of right matrix columns must be equal to number of rows of left one: ",
                                    rv);
                    }
                }
                else
                {
                    switch(rv.get_type())
                    {
                        case LEX_INTEGER:
                            result *= ints[rv.get_value()];
                            break;
                        case LEX_FLOAT:
                            result *= doubles[rv.get_value()];
                            break;
                        case LEX_RATIONAL:
                            result *= ratios[rv.get_value()];
                            break;
                        default:
                            throw Error("Unknown type of right operand: ", rv);
                            break;
                    }
                }
                adress = add_value(matrixes, result);
                res.set_type(LEX_MATRIX);
                res.set_value(adress);

            }
            break; // LEX_VECTOR
        default:
            throw Error("Unknown type of left operand: ", lv);
            break;
    } // switch for lv

    res.set_column(lv.get_column());
    res.set_row(lv.get_row());

    return res;
}
Lex Executer::check_div(Lex& lv, Lex& rv)
{
    Lex res;
    int adress;
    if (lv.get_type() == LEX_ID)
    {
        Ident id = TID[lv.get_value()];
        if (!id.is_assigned())
            throw lv;
        lv = Lex(id.get_type(), id.get_value(), lv.get_column(), lv.get_row());
    }

    if (rv.get_type() == LEX_ID)
    {
        Ident id = TID[rv.get_value()];
        if (!id.is_assigned())
            throw rv;
        rv = Lex(id.get_type(), id.get_value(), rv.get_column(), rv.get_row());
    }

    switch(lv.get_type())
    {
        case LEX_INTEGER:
            {
                switch (rv.get_type())
                {
                    case LEX_INTEGER:
                        {
                            if (!ints[rv.get_value()])
                                throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                            int result = ints[lv.get_value()] / ints[rv.get_value()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            if (!doubles[rv.get_value()])
                                throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                            int result = ints[lv.get_value()] / doubles[rv.get_type()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {
                            if (ratios[rv.get_value()] == 0)
                                throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                            int result = ints[lv.get_value()] / (int)ratios[rv.get_type()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw Error("Expected number of right operand instead of: ", rv);
                        break;

                } //switch fo rv
                break;
            } // LEX_INTEGER
        case LEX_FLOAT:
            switch (rv.get_type())
                {
                    case LEX_INTEGER:
                        {
                            if (!ints[rv.get_value()])
                                throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                            double result = doubles[lv.get_value()] / ints[rv.get_value()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            if (!doubles[rv.get_value()])
                                throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                            double result = doubles[lv.get_value()] / doubles[rv.get_type()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {
                            if (ratios[rv.get_value()] == 0)
                                throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                            double result = doubles[lv.get_value()] / (double)ratios[rv.get_type()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw Error("Expected number as right operand instead of: ", rv);
                        break;

                } //switch fo rv
            break; // LEX_FLOAT
        case LEX_RATIONAL:
            try
            {
            switch (rv.get_type())
                {
                    case LEX_INTEGER:
                        {
                            Rational_number result = ratios[lv.get_value()] / ints[rv.get_value()];
                            adress = add_value(ratios, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            Rational_number result = ratios[lv.get_value()] / doubles[rv.get_type()];
                            adress = add_value(ratios, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {

                            Rational_number result = ratios[lv.get_value()] / ratios[rv.get_type()];
                            adress = add_value(ratios, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw Error("Expected number as right operand instead of: ", rv);
                        break;

                } //switch fo rv
            }
            catch(Zerodivide &zr)
            {
                throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
            }
            break; // LEX_RATIONAL
        case LEX_VECTOR:
            {
                Vector result = vectors[lv.get_value()];
                try
                {
                    switch(rv.get_type())
                    {
                        case LEX_INTEGER:
                            result /= ints[rv.get_value()];
                            break;
                        case LEX_FLOAT:
                            result /= doubles[rv.get_value()];
                            break;
                        case LEX_RATIONAL:
                            result /= ratios[rv.get_value()];
                            break;
                        default:
                            throw Error("Expected number as right operand instead of: ", rv);
                            break;
                    }
                }
                catch (Zerodivide& zr)
                {
                    throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                }

                adress = add_value(vectors, result);
                res.set_type(LEX_VECTOR);
                res.set_value(adress);
                
            }
            break; // LEX_VECTOR
        case LEX_MATRIX:
            {
                Matrix result = matrixes[lv.get_value()]; 
                try
                {
                    switch(rv.get_type())
                    {
                        case LEX_INTEGER:
                            result /= ints[rv.get_value()];
                            break;
                        case LEX_FLOAT:
                            result /= doubles[rv.get_value()];
                            break;
                        case LEX_RATIONAL:
                            result /= ratios[rv.get_value()];
                            break;
                        default:
                            throw Error("Expected number as right operand instead of: ", rv);
                            break;
                    }
                }
                catch (Zerodivide &zr)
                {
                    throw Error("Right operand of division equals to zero: ", 
                                            Lex(LEX_NONE, rv.get_column(), rv.get_row()));
                }
                adress = add_value(matrixes, result);
                res.set_type(LEX_MATRIX);
                res.set_value(adress);

            }
            break; // LEX_VECTOR
        default:
            throw Error("Unknown type of left operand: ", lv);
            break;
    } // switch for lv

    res.set_column(lv.get_column());
    res.set_row(lv.get_row());

    return res;
}


//
// --------------------------------------MAIN----------------------------------------
//

int main (int argc, char* argv[])
{
    FILE* file;
    if (argc < 2)
    {
        file = stdin;
    }
    else
    {
        file = fopen(argv[1], "r");
        if (!file)
        {
            cout << COLOR_RED << "error: " << COLOR_CLEAR << endl;
            cout << "Can't open execution file: " << argv[1] << endl;
            return 1;
        }
    }
    try
    {
        Parser pars(file);
        pars.analyse();
        Executer ex;
        ex.execute(pars.poliz);
    }
    catch (Error& err)
    {
        cerr << ((argc < 2) ? "stdin" : argv[1]) << ":" << err.lex.get_row()<< ":" << err.lex.get_column() << ":";
        cerr << COLOR_RED << " error: " << COLOR_CLEAR << endl;
        cerr << err.reason << err.lex << endl;

        string str;
        if (file == stdin) return 1;
        fseek(file, 0, SEEK_SET);
        for (int i = 1; i < err.lex.get_row(); i++)
        {
            int ch = fgetc(file);
            while (ch != '\n')
                ch = fgetc(file);
        }
        while (1)
        {
            char ch = fgetc(file);
            if (ch == '\n' || ch == EOF)
                break;
            str.push_back((char)ch);
        }

        cerr << str << endl;
        
        if (err.lex.get_type() == LEX_FIN)
        {
            for (int i = 0; i < (int)str.size(); i++)
                cerr << " ";
        }
        else
            for (int i = 1; i < err.lex.get_column(); i++)
                cerr << " ";
        cerr << COLOR_GREEN << "^";
        
        if (err.lex.get_type() != LEX_FIN)
        {
            for (int i = 1; i < (int) ((string)err.lex).size(); i++)
                cerr << "~";
        }
        cerr << COLOR_CLEAR << endl;
        //if (file != stdin)
        fclose(file);
        return 1;
    }
    if (file != stdin)
        fclose(file);
    return 0;
}
