#include <vector>
#include <cstdio>
#include <stack>
#include <queue>
#include <algorithm>
#include <string>
#include <unistd.h>
#include <iostream>
#include <typeinfo>
#include "matrix.h"

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
        }
        lex_type get_type() const { return type;}
        int get_value() const { return value;}
        
        int get_column() const {return col;}
        int get_row() const {return row;}

        void set_column(int col) {this->col = col;}
        void set_row (int row) {this->row = row;}

        friend ostream& operator << (ostream& os, const Lex& lex);

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
        ~Scanner( )
        {
            if (file != stdin)
                fclose(file);
        }
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
                    else throw c;
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
                    double num = atoi (buf.c_str());
                    double frac_part = 10;
                    if (!isdigit(c))
                        throw c;
                    while (isdigit(c))
                    {
                        num += (double)(c - '0')/ frac_part;
                        frac_part *= 10.0;
                        gc();
                    }
                    rc();
                    int adress = add_value(doubles, num);
                    return Lex(LEX_FLOAT_NUM, adress, col - buf.size() + 1, row);
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
                        zr.debug_print();
                        cout << "On line: " << row << " column: " << col << endl;
                        throw c;
                    }
                    catch (NotANumber &nan)
                    {
                        nan.debug_print();
                        cout << "On line: " << row << " column: " << col << endl;
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
                    throw c;
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
                    else 
                        throw c;
                }
                else if (c == '"')
                {
                    int adress = add_string(buf);
                    return Lex(LEX_STRING, adress, col - buf.size() + 1, row);
                }
                else if (c == EOF || c == '\n')
                    throw c;
                else
                    buf.push_back(c);
                break;
        } // switch
            
    } // do while
    while (true);
}

ostream& operator << (ostream& os, const Lex& lex)
{
    string t;
    if (lex.type <= LEX_COL)
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
        t = ratios[lex.value].to_string();
    else if (lex.type == LEX_ID)
        t = TID[lex.value].get_name();
    else if (lex.type == LEX_STRING)
        t = strings[lex.value];
    else if (lex.type == LEX_FIN)
        t = "END";
    os << '(' << t << ");" << endl;
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

    void declare (lex_type type, int adress);
    void check_id();
    void check_op();
    void gl()
    {
        current_lex = scan.get_lex();
    }
    public:
        vector<Lex> poliz;
        Parser(const char* name): scan(name) {}
        void analyse();

};

void Parser::analyse()
{
    PROGRAM();
    if (current_lex.get_type() != LEX_FIN)
        throw current_lex;

}

void Parser::PROGRAM()
{
    gl();
    DECLARATION();
    PROCESS();
}

void Parser::DECLARATION()
{
    if (current_lex.get_type() != LEX_DECLARE)
    {
        gl();
        if (current_lex.get_type() != LEX_COLON)
            throw current_lex;
        gl();
        DECLARE();
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
    gl();
    if (current_lex.get_type() != LEX_COLON)
        throw current_lex;
    current_lex = lexes.top();
    TYPE();
    do
    {
        ID();
        declare(lexes.top().get_type(), current_lex.get_value());
    }
    while(current_lex.get_type() == LEX_COMA);
    lexes.pop();
    if (current_lex.get_type() != LEX_SEMICOLON)
        throw current_lex;
}

void Parser::TYPE()
{
    if (current_lex.get_type() != LEX_INTEGER &&
        current_lex.get_type() != LEX_FLOAT &&
        current_lex.get_type() != LEX_RATIONAL &&
        current_lex.get_type() != LEX_VECTOR &&
        current_lex.get_type() != LEX_MATRIX)
        throw current_lex;
}

void Parser::ID()
{
    gl();
    if (current_lex.get_type() != LEX_ID)
        throw current_lex;
    gl();
    if (current_lex.get_type() == LEX_LPAREN)
    {
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
            throw current_lex;
        poliz.push_back(current_lex);
        if (!lexes.empty() && lexes.top().get_type() == LEX_MINUS)
        {
            poliz.push_back(Lex(POLIZ_UNARY_MINUS));
            lexes.pop();
        }
        poliz.push_back(Lex(POLIZ_ASSIGN));
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
            throw current_lex;
    }
    gl();
}

void Parser::PROCESS()
{
    gl();
    if (current_lex.get_type() != LEX_COLON)
        throw current_lex;
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
    do
    {
        gl();
        OP();
        if (current_lex.get_type() == LEX_SEMICOLON)
        {
            poliz.push_back(current_lex);
            break;
        }
    }
    while (true);
}

void Parser::OP()
{
    //gl();
    if (current_lex.get_type() == LEX_ID)
    {
        lexes.push(current_lex);
        gl();
        if (current_lex.get_type() == LEX_ASSIGN)
        {
            poliz.push_back(lexes.top());
            lexes.pop();
            lexes.push(current_lex);
            EXPRESSION();
            poliz.push_back(lexes.top());
            lexes.pop();
        }
        else   
        {
            scan.return_lex(current_lex);
            current_lex = lexes.top();
            lexes.pop();
            EXPRESSION();
        }
    }
    else if (current_lex.get_type() == LEX_INFO)
    {
        lexes.push(current_lex);
        gl();
        if (current_lex.get_type() != LEX_LPAREN)
            throw current_lex;
        gl();
        if (current_lex.get_type() != LEX_STRING)
            throw current_lex;
        poliz.push_back(current_lex);
        poliz.push_back(lexes.top());
        lexes.pop();
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
            throw current_lex;
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
        lexes.push(current_lex);
        gl();
        EXPR();
        poliz.push_back(lexes.top());
        lexes.pop();
    }
}
void Parser::EXPR()
{
    UNARY();
    while (current_lex.get_type() == LEX_TIMES ||
           current_lex.get_type() == LEX_SLASH ||
           current_lex.get_type() == LEX_POWER)
    {
        lexes.push(current_lex);
        gl();
        UNARY();
        poliz.push_back(lexes.top());
        lexes.pop();
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
                if (current_lex.get_value() != LEX_NUM)
                    throw current_lex;
                if (id.get_type() == LEX_MATRIX)
                {
                    Lex row = current_lex;
                    gl();
                    if (current_lex.get_type() == LEX_COMA)
                    {
                        gl();
                        if (current_lex.get_value() != LEX_NUM)
                            throw current_lex;
                        Lex col = current_lex;
                        poliz.push_back(lexes.top());
                        poliz.push_back(Lex(POLIZ_MTR_EL,
                                            lexes.top().get_value(),
                                            col.get_value(),
                                            row.get_value()
                                           ));
                    }
                    else
                        throw current_lex;
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
                    throw current_lex;
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
    else if (current_lex.get_type() == LEX_NUM ||
             current_lex.get_type() == LEX_RATIO_NUM ||
             current_lex.get_type() == LEX_FLOAT_NUM)
    {
        poliz.push_back(current_lex);
    }
    else
        throw current_lex;
    gl();
}

void Parser::check_id()
{
    if (!TID[current_lex.get_value()].is_declared())
        throw current_lex;
}

void Parser::declare(lex_type type, int adress)
{
    TID[adress].put_declare();
    TID[adress].put_type(type);
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
            throw current_lex;
        gl();
        if (current_lex.get_type() != LEX_STRING)
            throw current_lex;
        
        poliz.push_back(current_lex);
        poliz.push_back(lexes.top());
        lexes.pop();
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
            throw current_lex;
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
            throw current_lex;
        gl();
        if (current_lex.get_type() != LEX_NUM)
            throw current_lex;
        poliz.push_back(current_lex);
        poliz.push_back(lexes.top());
        lexes.pop();
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
            throw current_lex;
    }
    else
        throw current_lex;
    gl();
}

class Executer
{
    Lex current_lex;
    
    void check_assignment(Ident& lv, Lex& rv);
    void check_add_div (Lex& lv, Lex& rv);
    void check_mul (Lex& lv, Lex& rv);
    void check_div (Lex& lv, Lex& rv);

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
                args.push(current_lex);
                break;
            case POLIZ_ASSIGN:
                {
                    int adress;
                    Lex arg = args.top();
                    args.pop();
                    Lex id = args.top();
                    args.pop();
                    switch(TID[id.get_value()].get_type())
                    {
                        case LEX_MATRIX:
                            if (arg.get_type() != LEX_STRING)
                                throw arg;
                            try
                            {
                                adress = add_value(matrixes, 
                                                   Matrix(strings[arg.get_value()].c_str()));
                            }
                            catch(OpenFileError &of)
                            {
                                of.debug_print();
                                throw arg;
                            }
                            break;
                        case LEX_VECTOR:
                            if (arg.get_type() != LEX_STRING)
                                throw arg;
                            try
                            {
                                adress = add_value(vectors,
                                                   Vector(strings[arg.get_value()].c_str()));
                            }
                            catch (OpenFileError &of)
                            {
                                of.debug_print();
                                throw arg;
                            }
                            break;
                        case LEX_INTEGER:
                            if (arg.get_type() != LEX_NUM)
                                throw arg;
                            adress = arg.get_value();
                            break;
                        case LEX_FLOAT:
                            if (arg.get_type() != LEX_FLOAT_NUM)
                                throw arg;
                            adress = arg.get_value();
                            break;
                        case LEX_RATIONAL:
                            if (arg.get_type() != LEX_RATIO_NUM)
                                throw arg;
                            adress = arg.get_value();
                            break;
                        default:
                            throw id;
                            break;
                    }
                    TID[id.get_value()].put_value(adress);
                    TID[id.get_value()].put_assign();
                }
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
                                else 
                                    throw op;
                            }
                            break;
                        case LEX_NUM:
                            ints[op.get_value()] = - ints[op.get_value()];
                            break;
                        case LEX_RATIO_NUM:
                            ratios[op.get_value()] = - ratios[op.get_value()];
                        case LEX_FLOAT_NUM:
                            doubles[op.get_value()] = - doubles[op.get_value()];
                        default:
                            throw op;
                            break;
                    }
                }
                break;
            case LEX_ASSIGN:
                {
                    Lex l_id = args.top();
                    Ident id = TID[l_id.get_value()];
                    args.pop();
                    Lex exp = args.top();
                    args.pop();
                    if (id.get_type() != LEX_ID)
                        throw id;
                    check_assignment(id, exp);
                }
                break;
            default:
                throw current_lex;
                break;
        }
    }
}

void Executer::check_assignment(Ident& lv, Lex& rv)
{
    switch(rv.get_value())
    {
        case LEX_ID:
        {
            Ident id = TID[rv.get_value()];
            if (!id.is_assigned())
                throw lv;
            if (lv.get_type() == LEX_MATRIX)
            { 
                if (id.get_type() == lv.get_type())
                {
                    if (lv.is_assigned())
                        matrixes[lv.get_value()] = matrixes[id.get_value()];
                    else
                    {
                        int adress = add_value(matrixes,
                                               matrixes[id.get_value()]);
                        lv.put_value(adress);
                        lv.put_assign();
                    }
                }
                else if (id.get_type() == LEX_VECTOR)
                {
                    if (lv.is_assigned())
                        matrixes[lv.get_value()] = vectors[id.get_value()];
                    else
                    {
                        int adress = add_value(matrixes,
                                               (Matrix)vectors[id.get_value()]);
                        lv.put_value(adress);
                        lv.put_assign();
                    }
                }
                else
                    throw lv;
            }
            else if (lv.get_type() == LEX_VECTOR)
            {
                if (id.get_type() != lv.get_type())
                    throw id;
                if (lv.is_assigned())
                    vectors[lv.get_value()] = vectors[id.get_value()];
                else
                {
                    int adress = add_value(vectors, vectors[id.get_value()]);
                    lv.put_value(adress);
                    lv.put_assign();
                }
            }

        }
    }

}

int main (int argc, char* argv[])
{
    if (argc < 2)
        return 1;
    try
    {
        Scanner s(argv[1]);
        Lex l = s.get_lex();
        while (l.get_type() != LEX_FIN)
        {
            cout << l;
            l = s.get_lex();
        }
    }
    catch (char c)
    {
        return 1;
    }

    return 0;
}
