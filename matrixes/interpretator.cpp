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

        void set_type (lex_type type) {this->type = type;}
        void set_value (int value) {this->value = value;}

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
        cout << current_lex;
    }
    public:
        vector<Lex> poliz;
        Parser(const char* name): scan(name) {}
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
        gl();
        if (current_lex.get_type() != LEX_COLON)
            throw current_lex;
        gl();
        DECLARE();
    }
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
    while (current_lex.get_type() != LEX_FIN &&
           current_lex.get_type() != LEX_SEMICOLON)
    {
        gl();
        OP();
        if (current_lex.get_type() == LEX_SEMICOLON ||
            current_lex.get_type() == LEX_FIN)
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
        {
            cout << "\"(\" was expected instead of :";
            throw current_lex;
        }
        gl();
        if (current_lex.get_type() != LEX_STRING)
        {
            cout << "Expected argument of type string instead of: ";
            throw current_lex;
        }
        poliz.push_back(current_lex);
        poliz.push_back(lexes.top());
        lexes.pop();
        gl();
        if (current_lex.get_type() != LEX_RPAREN)
        {
            cout << "\")\" was expected\n instead of: ";
            throw current_lex;
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
                {
                    cout << "Expected argument of type integer instead of: ";
                    throw current_lex;
                }
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
                    {
                        cout << "Expected ',' instead of: ";
                        throw current_lex;
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
                    cout << "Expected ']' instead of: ";
                    throw current_lex;
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
        cout << "Expected some object insted of: ";
        throw current_lex;
    }
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
        poliz.push_back(Lex(LEX_INTEGER,
                            current_lex.get_value(),
                            current_lex.get_column(),
                            current_lex.get_row()));
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
    cout << "STARTED POLIZ EXECUTION\n";
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
                        throw id;
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
                                else 
                                    throw op;
                            }
                            break;
                        case LEX_NUM: case LEX_INTEGER:
                            ints[op.get_value()] = - ints[op.get_value()];
                            break;
                        case LEX_RATIO_NUM: case LEX_RATIONAL:
                            ratios[op.get_value()] = - ratios[op.get_value()];
                        case LEX_FLOAT_NUM: case LEX_FLOAT:
                            doubles[op.get_value()] = - doubles[op.get_value()];
                        default:
                            throw op;
                            break;
                    }
                } // POLIZ_UNARY_MINUS
                break;
            case LEX_ASSIGN:
                {
                    Lex l_id = args.top();
                    if (l_id.get_type() != LEX_ID)
                        throw l_id;
                    Ident id = TID[l_id.get_value()];
                    args.pop();
                    Lex exp = args.top();
                    args.pop();
                    check_assignment(id, exp);
                    args.push(Lex(id.get_type(), id.get_value(), l_id.get_column(), l_id.get_row()));
                } // LEX_ASSIGN
                break;
            case LEX_PLUS:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    args.pop();
                    Lex res = check_add_sub(lv, rv, '+');
                    args.push(res);
                } // LEX_PLUS
                break;
            case LEX_MINUS:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    args.pop();
                    Lex res = check_add_sub(lv, rv, '-');
                    args.push(res);
                } // LEX_MINUS
                break;
            case LEX_TIMES:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    Lex res = check_mul(lv, rv);
                    args.push(res);
                } // LEX_TIMES
                break;
            case LEX_SLASH:
                {
                    Lex rv = args.top();
                    args.pop();
                    Lex lv = args.top();
                    args.pop();
                    Lex res = check_div(lv, rv);
                    args.push(res);
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
                        if (!id.is_assigned()) throw lv;
                        rv = Lex(id.get_type(), id.get_value(), rv.get_column(), rv.get_row());
                    }
                    if (lv.get_type() == LEX_ID)
                    {
                        Ident id = TID[lv.get_value()];
                        if (!id.is_assigned()) throw rv;
                        lv = Lex(id.get_type(), id.get_value(), lv.get_column(), lv.get_row());
                    }
                    if (lv.get_type() != LEX_MATRIX)
                        throw lv;
                    if (rv.get_type() != LEX_INTEGER)
                        throw rv;
                    Matrix result = matrixes[lv.get_value()] ^ ints[rv.get_value()];
                    int adress = add_value(matrixes, result);
                    Lex res(LEX_MATRIX, adress, lv.get_column(), rv.get_row());

                    args.push(res);
                } // LEX_INFO
                break;
            case LEX_INFO:
                {
                    Lex op = args.top();
                    args.pop();
                    if (op.get_type() != LEX_STRING)
                        throw op;
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
                        if (!id.is_assigned()) throw op;
                        op = Lex(id.get_type(), id.get_value(), op.get_column(), op.get_row());
                    }
                    switch(op.get_type())
                    {
                        case LEX_MATRIX:
                            {
                                char* str = matrixes[op.get_value()].to_string();
                                cout << str;
                                delete[]str;
                            }
                            break;
                        case LEX_VECTOR:
                            {
                                char* str = vectors[op.get_value()].to_string();
                                cout << str;
                                delete[]str;
                            }
                            break;
                        case LEX_RATIONAL:
                            {
                                char* str = ratios[op.get_value()].to_string();
                                cout << str;
                                delete[]str;
                            }
                        case LEX_FLOAT:
                            cout << doubles[op.get_value()];
                            break;
                        case LEX_INTEGER:
                            cout << ints[op.get_value()];
                            break;
                        default:
                            throw op;
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
                        if (!id.is_assigned()) throw col;
                        col = Lex(id.get_type(), id.get_value(), col.get_column(), col.get_row());
                    }
                    if (mtr.get_type() == LEX_ID)
                    {
                        Ident id = TID[mtr.get_value()];
                        if (!id.is_assigned()) throw mtr;
                        mtr = Lex(id.get_type(), id.get_value(), mtr.get_column(), mtr.get_row());
                    }

                    if (col.get_type() != LEX_INTEGER) throw col;
                    if (mtr.get_type() != LEX_MATRIX) throw mtr;

                    Matrix_col_coord coord(ints[col.get_value()]);
                    int adress;
                    try
                    {
                        adress = add_value(vectors, matrixes[mtr.get_value()][coord]);
                    }
                    catch (OutOfRangeMatrix & orf)
                    {
                        orf.debug_print();
                        throw col;
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
                        if (!id.is_assigned()) throw row;
                        row = Lex(id.get_type(), id.get_value(), row.get_column(), row.get_row());
                    }
                    if (mtr.get_type() == LEX_ID)
                    {
                        Ident id = TID[mtr.get_value()];
                        if (!id.is_assigned()) throw mtr;
                        mtr = Lex(id.get_type(), id.get_value(), mtr.get_column(), mtr.get_row());
                    }

                    if (row.get_type() != LEX_INTEGER) throw row;
                    if (mtr.get_type() != LEX_MATRIX) throw mtr;

                    Matrix_row_coord coord(ints[row.get_value()]);
                    int adress;
                    try
                    {
                        adress = add_value(vectors, matrixes[mtr.get_value()][coord]);
                    }
                    catch (OutOfRangeMatrix & orf)
                    {
                        orf.debug_print();
                        throw row;
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
                        if (!id.is_assigned()) throw op;
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
                        throw op;
                } // LEX_ROTATE
                break;
            case LEX_WRITE:
                {
                    Lex str = args.top();
                    args.pop();
                    Lex op = args.top();
                    args.pop();

                    if (str.get_type() != LEX_STRING)
                        throw str;
                    if (op.get_type() == LEX_ID)
                    {
                        Ident id = TID[op.get_value()];
                        if (!id.is_assigned()) throw op;
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
                            of.debug_print();
                            throw str;
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
                            of.debug_print();
                            throw str;
                        }
                    }
                    else
                    {
                        FILE* file = fopen(strings[str.get_value()].c_str(), "wa");
                        if (file == NULL)
                        {
                            cout << "Can't open file '" << strings[str.get_value()] << "'\n";
                            throw str;
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
                            throw op;
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
                        throw str;
                    if (op.get_type() != LEX_ID)
                        throw op;
                    Ident id = TID[op.get_value()];
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
                                    ofr.debug_print();
                                    throw str;
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
                                    of.debug_print();
                                    throw str;
                                }
                            }
                            break;
                        case LEX_RATIONAL:
                            {
                                Scanner scan(strings[str.get_value()].c_str());
                                Lex l = scan.get_lex();
                                if (l.get_type() != LEX_RATIO_NUM)
                                    throw l;
                                put_value(ratios, ratios[l.get_value()], id);
                                remove_value(ratios, l.get_value());
                            }
                            break;
                        default:
                            {
                                FILE* file = fopen(strings[str.get_value()].c_str(), "r");
                                if (file == NULL)
                                {
                                    cout << "Can't open file '"<< strings[str.get_value()] << "'.\n";
                                    throw str;
                                }
                                if (id.get_type() == LEX_INTEGER)
                                {
                                    int val;
                                    fscanf(file, "%d", &val);
                                    put_value(ints, val, id);
                                }
                                if (id.get_type() == LEX_FLOAT)
                                {
                                    double val;
                                    fscanf(file, "%lf", &val);
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
                        if (!id.is_assigned()) throw mtr;
                        mtr = Lex(id.get_type(), id.get_value(), mtr.get_column(), mtr.get_row());
                    }

                    if (mtr.get_type() != LEX_MATRIX)
                        throw mtr;
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
                        ofm.debug_print();
                        throw current_lex;
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
                        ofv.debug_print();
                        throw current_lex;
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
                        if (!id.is_assigned()) throw op;
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
                        throw op;
                }
                break;
            default:
                throw current_lex;
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
                    put_value(matrixes, matrixes[id.get_value()], lv);
                }
                else if (id.get_type() == LEX_VECTOR)
                {
                    Matrix mtr = vectors[id.get_value()];
                    put_value(matrixes, mtr, lv);
                }
                else
                    throw lv;
            }
            else if (lv.get_type() == LEX_VECTOR)
            {
                if (id.get_type() != lv.get_type())
                    throw id;
                put_value(vectors, vectors[id.get_value()], lv);
            }
            else
            {
                if (id.get_type() == LEX_VECTOR ||
                    id.get_type() == LEX_MATRIX)
                    throw id;
                if (lv.get_type() == LEX_RATIONAL)
                {
                    switch (id.get_type())
                    {
                        case LEX_RATIONAL:
                            put_value(ratios, ratios[id.get_value()], lv);
                            break;
                        case LEX_FLOAT:
                            {
                                Rational_number ratio = doubles[id.get_value()];
                                put_value(ratios, ratio, lv);
                            }
                            break;
                        case LEX_INTEGER:
                            {
                                Rational_number ratio = ints[id.get_value()];
                                put_value(ratios, ratio, lv);
                            }
                            break;
                        default:
                            throw id;
                            break;
                    }
                }
                else if (lv.get_type() == LEX_INTEGER)
                {
                    switch (id.get_type())
                    {
                        case LEX_RATIONAL:
                            {
                                int i = (int)ratios[id.get_value()];
                                put_value(ints, i, lv);
                            }
                            break;
                        case LEX_FLOAT:
                            {
                                int i = doubles[id.get_value()];
                                put_value(ints, i, lv);
                            }
                            break;
                        case LEX_INTEGER:
                            {
                                put_value(ints, ints[id.get_value()], lv);
                            }
                            break;
                        default:
                            throw id;
                            break;
                    }

                }
                else if (lv.get_type() == LEX_FLOAT)
                {
                    switch (id.get_type())
                    {
                        case LEX_RATIONAL:
                            {
                                double i = (double)ratios[id.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        case LEX_FLOAT:
                            {
                                double i = doubles[id.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        case LEX_INTEGER:
                            {
                                double i = ints[id.get_value()];
                                put_value(doubles,i, lv);
                            }
                            break;
                        default:
                            throw id;
                            break;
                    }

                }
                else throw id;
            } // if id.type is number

        }// case LEX_ID
        case LEX_INTEGER: case LEX_NUM:
                {
                    switch(lv.get_type())
                    {
                        case LEX_INTEGER:
                            put_value(ints, ints[rv.get_value()], lv);
                            break;
                        case LEX_RATIONAL:
                            {
                                Rational_number rat = ints[rv.get_value()];
                                put_value(ratios, rat, lv);
                            }
                            break;
                        case LEX_FLOAT:
                            {
                                double i = ints[rv.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        default:
                            throw lv;
                            break;
                    }
                }
                break;
        case LEX_RATIONAL: case LEX_RATIO_NUM:
                {
                    switch(lv.get_type())
                    {
                        case LEX_INTEGER:
                            {
                                int i = (int)ratios[rv.get_value()];
                                put_value(ints, i, lv);
                            }
                            break;
                        case LEX_RATIONAL:
                            {
                                Rational_number rat = ratios[rv.get_value()];
                                put_value(ratios, rat, lv);
                            }
                            break;
                        case LEX_FLOAT:
                            {
                                double i =(double) ratios[rv.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        default:
                            throw lv;
                            break;
                    }
                }
                break;
        case LEX_FLOAT: case LEX_FLOAT_NUM:
                {
                    switch(lv.get_type())
                    {
                        case LEX_INTEGER:
                            {
                                int i = (int)doubles[rv.get_value()];
                                put_value(ints, i, lv);
                            }
                            break;
                        case LEX_RATIONAL:
                            {
                                Rational_number rat = doubles[rv.get_value()];
                                put_value(ratios, rat, lv);
                            }
                            break;
                        case LEX_FLOAT:
                            {
                                double i = doubles [rv.get_value()];
                                put_value(doubles, i, lv);
                            }
                            break;
                        default:
                            throw lv;
                            break;
                    }
                }
        case LEX_MATRIX:
                {
                    if (lv.get_type() != LEX_MATRIX)
                        throw rv;
                    put_value(matrixes, matrixes[rv.get_value()], lv);
                }
                break;
        case LEX_VECTOR:
                {
                    if (lv.get_type() == LEX_MATRIX)
                    {
                        Matrix mtr = vectors[rv.get_value()];
                        put_value(matrixes, mtr, lv);
                    }
                    else if (lv.get_type() == LEX_VECTOR)
                        put_value(vectors, vectors[rv.get_value()], lv);
                    else
                        throw rv;

                }
                break;
         default:
                throw rv;
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
            throw lv;
        lv = Lex(id.get_type(), id.get_value(), lv.get_column(), lv.get_row());
    }
    if (rv.get_type() == LEX_ID)
    {
        Ident id = TID[rv.get_value()];
        if (!id.is_assigned())
            throw rv;
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
                        throw rv;
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
                        throw rv;
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
                        throw rv;
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
                        wrm.debug_print();
                        throw rv;
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
                        wrm.debug_print();
                        throw rv;
                    }
                }
                else 
                    throw rv;
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
                        wrv.debug_print();
                        throw rv;
                    }
                }
                else
                    throw rv;
                adress = add_value(vectors, result);
                res.set_type(LEX_VECTOR);
                res.set_value(adress);
            }
            break;
        default:
            throw lv;
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
                        throw rv;
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
                        throw rv;
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
                        throw rv;
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
                        wrv.debug_print();
                        throw rv;
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
                        wrm.debug_print();
                        throw rv;
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
                            throw rv;
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
                        wrm.debug_print();
                        throw rv;
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
                        wrm.debug_print();
                        throw rv;
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
                            throw rv;
                            break;
                    }
                }
                adress = add_value(matrixes, result);
                res.set_type(LEX_MATRIX);
                res.set_value(adress);

            }
            break; // LEX_VECTOR
        default:
            throw lv;
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
                            int result = ints[lv.get_value()] / ints[rv.get_value()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            int result = ints[lv.get_value()] / doubles[rv.get_type()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {
                            int result = ints[lv.get_value()] / (int)ratios[rv.get_type()];
                            adress = add_value(ints, result);
                            res.set_type(LEX_INTEGER);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw rv;
                        break;

                } //switch fo rv
                break;
            } // LEX_INTEGER
        case LEX_FLOAT:
            switch (rv.get_type())
                {
                    case LEX_INTEGER:
                        {
                            double result = doubles[lv.get_value()] / ints[rv.get_value()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_FLOAT:
                        {
                            double result = doubles[lv.get_value()] / doubles[rv.get_type()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    case LEX_RATIONAL:
                        {
                            double result = doubles[lv.get_value()] / (double)ratios[rv.get_type()];
                            adress = add_value(doubles, result);
                            res.set_type(LEX_FLOAT);
                            res.set_value(adress);
                        }
                        break;
                    default:
                        throw rv;
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
                        throw rv;
                        break;

                } //switch fo rv
            }
            catch(Zerodivide &zr)
            {
                zr.debug_print();
                throw rv;
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
                            throw rv;
                            break;
                    }
                }
                catch (Zerodivide& zr)
                {
                    zr.debug_print();
                    throw rv;
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
                            throw rv;
                            break;
                    }
                }
                catch (Zerodivide &zr)
                {
                    zr.debug_print();
                    throw rv;
                }
                adress = add_value(matrixes, result);
                res.set_type(LEX_MATRIX);
                res.set_value(adress);

            }
            break; // LEX_VECTOR
        default:
            throw lv;
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
    if (argc < 2)
        return 1;
    try
    {
        Parser pars(argv[1]);
        pars.analyse();
        Executer ex;
        ex.execute(pars.poliz);
    }
    catch (char c)
    {
        return 1;
    }
    catch (Lex l)
    {
        cout << l << " col: " << l.get_column() << " row: " << l.get_row() << endl;
        return 2;
    }

    return 0;
}
