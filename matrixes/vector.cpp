#include "rational.h"
#include "exception.h"
#include "vector.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

Vector::~Vector()
{
    while(node) node = Node<Rational_number>::remove(node->return_key(), node);
}

char* MathObject::read_str(FILE* file,int &err)
{
    int i = 0;
    char* str = NULL;
    do
    {
        int ch = fgetc(file);

        char* temp =(char*) realloc(str,sizeof(char)*(i + 1));
        if (temp == NULL)
        {
            free(str);
            //throw Exception();
            break;
        }
        str = temp;
        if (ch == '#')
        {
            while(ch != '\n' || ch != EOF) ch = fgetc(file);
            str[i] = '\0';
            err = (ch == EOF) ? -1 : 0;
            return str;
        }

        if (ch == EOF)
        {
            str[i] = '\0';
            err = -1;
            return str;
        }
        if (ch == '\n')
        {
            err = 0;
            str[i] = '\0';
            return str;
        }
        else
            str[i] =(char)ch;
        i++;
    }
    while (true);

    return NULL;
}

Rational_number Vector::Iterator::operator= (const Rational_number& num)
{
    if (num == 0) remove();
    else
        provide() = num;
    return num;
}

Rational_number Vector::Iterator::operator+= (const Rational_number& num)
{
    Rational_number& location = provide();
    location += num;
    Rational_number res = location;
    if (res == 0) remove();
    return res;
}


Rational_number Vector::Iterator::operator-= (const Rational_number& num)
{
    Rational_number& location = provide();
    location -= num;
    Rational_number res = location;
    if (res == 0) remove();
    return res;
}


Rational_number Vector::Iterator::operator*= (const Rational_number& num)
{
    Rational_number& location = provide();
    location *= num;
    Rational_number res = location;
    if (res == 0) remove();
    return res;
}


Rational_number Vector::Iterator::operator/= (const Rational_number& num)
{
    Rational_number& location = provide();
    location /= num;
    Rational_number res = location;
    if (res == 0) remove();
    return res;
}


Rational_number Vector::Iterator::operator++ ()
{
    Rational_number& location = provide();
    Rational_number res = ++location;
    if (location == 0) remove();
    return res;
}

Rational_number Vector::Iterator::operator++ (int)
{
    Rational_number& location = provide();
    Rational_number res = location++;
    if (location == 0) remove();
    return res;
}

Rational_number Vector::Iterator::operator-- ()
{
    Rational_number& location = provide();
    Rational_number res = --location;
    if (location == 0) remove();
    return res;
}

Rational_number Vector::Iterator::operator-- (int)
{
    Rational_number& location = provide();
    Rational_number res = location--;
    if (location == 0) remove();
    return res;
}


Rational_number Vector::Iterator::find(Node<Rational_number>* p)
{
    if (!p) return 0;
    if (index == p->return_key()) return p->value;
    if (index < p->return_key()) return find(p->return_left());
    return find(p->return_right());
}

Vector::Iterator::operator Rational_number()
{
    return find(master.node);
}

Rational_number& Vector::Iterator::provide()
{
    Node<Rational_number>* head = master.node;
    if (find(master.node) == 0) head = Node<Rational_number>::insert(index, master.node, 0);
    while(true)
    {
        if (index == head->return_key()) return head->value;
        else if(index < head->return_key()) head = head->return_left();
        else head = head->return_right();
    }
}

void Vector::Iterator::remove()
{
    Node<Rational_number>::remove(index, master.node);
}

Vector::Vector(unsigned int size, States state) : node(0)
{
    this->size = size;

    switch(state)
    {
    case Zeros:
        break;
    case Ones:
        for (unsigned int i = 0; i < size; i++)
            (*this)[i]= 1;
        break;
    default:
        //throw Exception();
        break;
    }
}

Vector::Vector(const Vector& vec) : node(0)
{
    size = vec.size;
    
    node = Node<Rational_number>::copy(node, vec.node);
}

Vector::Vector(const char* file_name) : node(0)
{
    FILE* f = fopen(file_name, "r");
    //if (f == NULL) //throw Exception();
    
    while(true)
    {
        int err = -1;
        char* str = read_str(f, err);
        if (err || str == NULL) 
        {
            if (str != NULL)
                free(str);
            break;
        }
        


        
    }

    fclose(f);
}

char* Vector::to_string()
{
    char* completed = NULL;
    
    char* str = Iterator(*this, 0).find(node).to_string();
    
    completed = new char[strlen(str) + 2];
    strcpy(completed, str);
    strcat(completed, " \0");

    for (unsigned int i = 1; i < size; i++)
    {
        Rational_number rat = Iterator(*this, i).find(node);

        char* tmp = new char [strlen(completed) + 1];
        strcpy(tmp, completed);
        char* str_rat = rat.to_string();
        completed = new char [strlen(completed) + strlen(completed) + 2];
        strcpy(completed, tmp);
        strcat(completed, str_rat);
        strcat(completed, " \0");
        delete[] tmp;
        delete[] str_rat;
    }

    return completed;
}

