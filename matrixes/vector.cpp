#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "matrix.h"


//
// ---------------------------------------CONSTRUCTORS------------------------------------
//

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
        throw WrongArgument();
        break;
    }
}

Vector::Vector(const Vector& vec)
{
    node = 0;
    size = vec.size;
    
    Node<Rational_number>::copy(node, vec.node);
}

Vector::Vector(const char* file_name) : node(0)
{
    FILE* f = fopen(file_name, "r");
    if (f == NULL) throw OpenFileError(file_name); 
    ReadFile::read_vector_file(*this, f);
    fclose(f);
}


//
// ------------------------------------ITERATOR_CLASS---------------------------------
//

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


Rational_number& Vector::Iterator::provide()
{
    Node<Rational_number>* p = Node<Rational_number>::find(index, master.node);

    if (p) return p->value;

    master.node = Node<Rational_number>::insert(index,master.node, 0);

    return Node<Rational_number>::find(index, master.node)->value;
}

void Vector::Iterator::remove()
{
    master.node = Node<Rational_number>::remove(index, master.node);
}


Vector::Iterator::operator Rational_number()
{
    Node<Rational_number>* p = Node<Rational_number>::find(index, master.node);
    return p ? p->value : 0;
}

//
// --------------------------------RECURSIVE_CALCULATIONS---------------------------------
//

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

void Vector::dot_product(Rational_number& rat, Node<Rational_number>* p,const Vector& vec) const
{
    if (!p) return;

    rat += p->value * vec[p->return_key()];

    dot_product(rat, p->return_left(), vec);
    dot_product(rat, p->return_right(), vec);
}

//
// -------------------------------------OPERATORS--------------------------------
//

Vector::operator bool() const
{
    return node? true : false;
}


Vector::Iterator Vector::operator()(unsigned int index)
{
    if (index >= size) throw OutOfRangeVector(*this, index);

    return Iterator(*this, index);
}

Rational_number Vector::operator[] (unsigned int index) const
{
    if (index >= size) throw OutOfRangeVector(*this, index);
    
    Node<Rational_number>* p = Node<Rational_number>::find(index, node);
    return p ? p->value : 0;
}

Vector Vector::operator=(const Vector& rv)
{
    size = rv.size;
    
    for (unsigned int i = 0; i < size; i++)
        if (Node<Rational_number>::find(i, node))
            node = Node<Rational_number>::remove(i, node);

    node = 0; 
    Node<Rational_number>::copy(node, rv.node);

    return *this;
}

Vector Vector::operator+(const Vector& rv) const
{
    if (size != rv.size) throw WrongVectorSize("+", *this, rv);
    Vector res(*this);

    calculations(res, rv.node, '+');

    return res;
}


Vector Vector::operator-(const Vector& rv) const
{
    if (size != rv.size) throw WrongVectorSize("-", *this, rv);
    
    Vector res(*this);

    calculations(res, rv.node, '-');

    return res;
}

Vector Vector::operator-=(const Vector& rv)
{
    if (size != rv.size) throw WrongVectorSize("-=", *this, rv);
    
    *this = *this - rv;

    return *this;
}

Vector Vector::operator+=(const Vector& rv)
{
    if (size != rv.size) throw WrongVectorSize("+=", *this, rv);

    
    *this = *this + rv;

    return *this;
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
    if (size != rv.size) throw WrongVectorSize("*", *this, rv);
    Rational_number res;
    
    dot_product(res, node, rv);

    return res;
}

//
// ---------------------------------OTHER---------------------------
//


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

    if (f == NULL) throw OpenFileError(file_name);
    fprintf(f, "vector %u\n", size);
    
    write_node(f, node);
    
    fclose(f);

}

//
// ------------------------------------DESTRUCTOR-------------------------------------
//

Vector::~Vector()
{
    for (unsigned int i = 0; i < size; i++)
    {
        if (Node<Rational_number>::find(i, node)) 
            node = Node<Rational_number>::remove(i, node);
    } 
}

