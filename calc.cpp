#include <iostream>
#include <stack>
#include <vector>
#include <string>

using namespace std;

enum lex_type
{
    LEX_NUM, LEX_ADD, LEX_SUB,
    LEX_MUL, LEX_DIV, LEX_LBRACE,
    LEX_RBRACE, LEX_NONE
};

class Lex
{
    lex_type type;
    int value;
    public:
        Lex(lex_type a_type = LEX_NONE, int a_value = 0) 
            : type(a_type), value(a_value) {}
        int get_value() {return value;}
        int get_type() {return type;}
};

int analyse(string str);
int poliz_translation(vector<Lex>& buff);

int main (int argc, char* argv[])
{
    string str;

    while (true)
    {
        getline(cin, str, '\n');

        if (str == "exit")
            break;
        try
        {
            cout << analyse(str) << endl;
        }
        catch (string err)
        {
            cout << err << endl;
        }
    }
    
    return 0;
}

int analyse (string str)
{
    vector <Lex> buff;
    stack<Lex> ops;
    enum an{NONE, NUM, OPER};
    int i = 0;
    int val = 0;
    an a = NONE;
    while (i <= (int)str.length())
    {
        switch (a)
        {
            case NONE:
                if (str[i] >= '0' && str[i] <= '9')
                {
                    val = 0;
                    a = NUM;
                }
                else if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/'
                                       || str[i] == '(' || str[i] == ')')
                {
                    a = OPER;
                }
                else if (str[i] == ' ' || str[i] == '\0')
                {
                    i++;
                }
                else throw ("Wrong expression: " + str);
                break;
            case NUM:
                if (str[i] >= '0' && str[i] <= '9')
                {
                    val *= 10;
                    val += (str[i] - '0');
                    i++;
                }
                else
                {
                    buff.push_back(Lex (LEX_NUM, val));
                    a = NONE;
                }
                break;
            case OPER:
                if (str[i] == '+')
                {
                    while (!ops.empty() && ops.top().get_type() != LEX_LBRACE)
                    {
                        buff.push_back(ops.top());
                        ops.pop();
                    }

                    ops.push(Lex(LEX_ADD));
                }
                else if (str[i] == '-')
                {
                    while (!ops.empty() && ops.top().get_type() != LEX_LBRACE)
                    {
                        buff.push_back(ops.top());
                        ops.pop();
                    }
                    ops.push(Lex(LEX_SUB));
                }
                else if (str[i] == '*')
                {
                    while (!ops.empty() && 
                            (ops.top().get_type() != LEX_ADD 
                             && ops.top().get_type() != LEX_SUB
                             && ops.top().get_type() != LEX_LBRACE))
                    {
                        buff.push_back(ops.top());
                        ops.pop();
                    }
                    ops.push(Lex(LEX_MUL));

                }
                else if (str[i] == '/')
                {
                    while (!ops.empty() && 
                            (ops.top().get_type() != LEX_ADD 
                             && ops.top().get_type() != LEX_SUB
                             && ops.top().get_type() != LEX_LBRACE))
                    {
                        buff.push_back(ops.top());
                        ops.pop();
                    }
                    ops.push(Lex(LEX_DIV));
                }
                else if (str[i] == '(')
                    ops.push(Lex(LEX_LBRACE));
                else if (str[i] == ')')
                {
                    while (!ops.empty() && ops.top().get_type() != LEX_LBRACE)
                    {
                        buff.push_back(ops.top());
                        ops.pop();
                    }
                    if (ops.empty()) throw "Wrong expression:" + str;
                    ops.pop();
                }
                i++;
                a = NONE;
                break;
        }
    }
    while (!ops.empty())
    {
        buff.push_back(ops.top());
        ops.pop();
    }
    
    for (Lex l : buff)
    {
        switch (l.get_type())
        {
            case LEX_NUM:
                cout << "LEX_NUM ";
                break;
            case LEX_ADD:
                cout << "LEX_ADD ";
                break;
            case LEX_SUB:
                cout << "LEX_SUB ";
                break;
            case LEX_MUL:
                cout << "LEX_MUL ";
                break;
            case LEX_DIV:
                cout << "LEX_DIV ";
                break;
        }
    }

    cout << endl;
    return poliz_translation(buff);
}

int poliz_translation(vector <Lex>& buff)
{
    stack<int> args;
    int i = 0;
    while (i < (int) buff.size())
    {
        switch(buff[i].get_type())
        {
            case LEX_NUM:
                args.push(buff[i].get_value());
                break;
            case LEX_ADD:
                {
                    int a = args.top();
                    args.pop();
                    int b = args.top();
                    args.pop();
                    args.push(a + b);
                    break;
                }
            case LEX_SUB:
                {
                    int a = args.top();
                    args.pop();
                    int b = args.top();
                    args.pop();
                    args.push (b - a);
                    break;
                }
            case LEX_MUL:
                {
                    int a = args.top();
                    args.pop();
                    int b = args.top();
                    args.pop();
                    args.push (b * a);
                    break;
                }
            case LEX_DIV:
                {
                    int a = args.top();
                    args.pop();
                    int b = args.top();
                    if (!a) throw "Division by zero!";
                    args.pop();
                    args.push (b / a);
                }
                break;
        }
        i++;
    }

    return args.top();
}
