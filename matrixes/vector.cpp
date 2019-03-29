#include "rational.h"
#include "exception.h"
#include "vector.h"
#include "node.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

Vector::~Vector()
{
    for (unsigned int i = 0; i < size; i++)
    {
        if (Accessor(*this, i).find(node) != 0) node = Node<Rational_number>::remove(i, node);
    } 
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
    {
        Rational_number& location = provide();
        location = num;
    }
    return num;
}

Rational_number Vector::Iterator::operator+= (const Rational_number& num)
{
    Rational_number &location = provide();
    location += num;
    Rational_number res = location;
    if (res == 0) remove();

    return res;
}


Rational_number Vector::Iterator::operator-= (const Rational_number& num)
{
    Rational_number &location = provide();
    location -= num;
    Rational_number res = location;
    if (res == 0) remove();

    return res;
}


Rational_number Vector::Iterator::operator*= (const Rational_number& num)
{
    Rational_number &location = provide();
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

void Vector::calculations (Vector& vec, Node<Rational_number>* q, char op) const
{
    if (!q)  return;
    switch (op)
    {
        case '+':
            vec(q->return_key()) += q->value;
            break;
        case '-':
            vec(q->return_key()) -= q->value;
            break;
    }
    calculations(vec, q->return_left(), op);
    calculations(vec, q->return_right(), op);
}

void Vector::calculations (Vector& vec, Rational_number rat,Node<Rational_number>* q, char op) const
{
    if (!q)  return;
    switch (op)
    {
        case '*':
            vec(q->return_key()) *= rat;
            break;
        case '/':
            vec(q->return_key()) /= rat;
            break;
    }
    calculations(vec,rat, q->return_left(), op);
    calculations(vec,rat, q->return_right(), op);
}

void Vector::copy(Node<Rational_number>* p)
{
    if (!p) return;
    node = Node<Rational_number>::insert(p->return_key(), node, p->value);

    copy(p->return_left());
    copy(p->return_right());
}

void Vector::dot_product(Rational_number& rat, Node<Rational_number>* p, Node<Rational_number>* q) const
{
    if (!p) return;
    Rational_number ratio = Accessor(*this, p->return_key()).find(q);
    if (ratio == 0) return;

    rat += ratio * p->value;

    dot_product(rat, p->return_left(), q);
    dot_product(rat, p->return_right(), q);
}

Rational_number& Vector::Iterator::provide()
{
    Node<Rational_number>* head = master.node;
    if (find(master.node) == 0) master.node = head = Node<Rational_number>::insert(index, master.node, 0);
    while(true)
    {
        if (index == head->return_key()) return head->value;
        else if(index < head->return_key()) head = head->return_left();
        else head = head->return_right();
    }
}

Rational_number Vector::Accessor::find(Node<Rational_number>* p)
{
    if (!p) return 0;
    if (index == p->return_key()) return p->value;
    if (index < p->return_key()) return find(p->return_left());
    return find(p->return_right());
}

Vector::Accessor::operator Rational_number()
{
    return find(master.node);
}

void Vector::Iterator::remove()
{
    master.node = Node<Rational_number>::remove(index, master.node);
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
            (*this)(i)= 1;
        break;
    default:
        //throw Exception();
        break;
    }
}

Vector::Vector(const Vector& vec)
{
    node = 0;
    size = vec.size;
    
    copy(vec.node);
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

Vector::operator bool() const
{
    return node? true : false;
}
void Vector::write_node(FILE* file, Node<Rational_number>* p) const
{
    if (!p) return;

    write_node(file, p->return_left());

    char* str = p->value.to_string();
    fprintf(file, "%u %s\n", p->return_key(), str);
    delete[] str;

    write_node(file, p->return_right());
}

void Vector::write(const char* file_name) const
{
    FILE* f = fopen(file_name, "w");

    //if (f == NULL) throw Exception();
    fprintf(f, "vector %u\n", size);
    
    write_node(f, node);
    
    fclose(f);

}

Vector Vector::operator=(const Vector& rv)
{
    size = rv.size;

    copy(rv.node);

    return *this;
}

Vector Vector::operator+(const Vector& rv) const
{
    //if (size != rv.size) throw Exception();
    Vector res(*this);

    calculations(res, rv.node, '+');

    return res;
}


Vector Vector::operator-(const Vector& rv) const
{
    //if (size != rv.size) throw Exception();
    
    Vector res(*this);

    calculations(res, rv.node, '-');

    return res;
}

Vector Vector::operator*=(const Rational_number& rv)
{
    (*this) = (*this) * rv;
    return *this;
}

Vector Vector::operator/=(const Rational_number& rv)
{
    (*this) = (*this) / rv;
    return *this;
}

Vector operator* (const Vector& lv,const Rational_number& rv)
{
    Vector res (lv);
    res.calculations(res, rv, lv.node, '*');
    return res;
}

Vector operator/ (const Vector& lv,const Rational_number& rv)
{
    Vector res(lv);

    res.calculations(res, rv, lv.node, '/');
    
    return res;
}
Vector operator* (const Rational_number& lv,const Vector& rv)
{
    Vector res(rv);
    
    res.calculations(res, lv, rv.node, '*');

    return res;
}


Rational_number Vector::operator*(const Vector& rv) const
{
    //if (size != rv.size) throw Exception();
    Rational_number res;
    
    dot_product(res, node, rv.node);

    return res;
}

char* Vector::to_string() const
{
    char* completed = NULL;
    
    char* str = (*this)[0].to_string();
    
    completed = new char[strlen(str) + 2];
    strcpy(completed, str);
    strcat(completed, " \0");
    
    delete[] str;
    for (unsigned int i = 1; i < size; i++)
    {
        char* tmp = completed;
        str = (*this)[i].to_string();
        completed = new char [strlen(tmp) + strlen(str) + 2];
        strcpy(completed, tmp);
        strcat(completed, str);
        strcat(completed, " \0");
        delete[] tmp;
        delete[] str;
    }

    return completed;
}

