#include "matr.h"
#include "rational.h"
#include "node.h"
#include "vector.h"
#include "exception.h"
#include <cstdio>

Rational_number Matrix::Iterator_Rat::operator= (const Rational_number& rat)
{
    Vector& vec = provide();

    Rational_number res = vec(col) = rat;

    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator*=(const Rational_number& rat)
{
   Vector& vec = provide();
    
    vec(col) *= rat;
    Rational_number res = vec[col];
    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator/=(const Rational_number& rat)
{
    Vector& vec = provide();

    vec(col) /= rat;
    Rational_number res = vec[col];
    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator+=(const Rational_number& rat)
{
    Vector& vec = provide();

    vec(col) += rat;
    Rational_number res = vec[col];

    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator-=(const Rational_number& rat)
{
    Vector& vec = provide();

    vec(col) -= rat;
    Rational_number res = vec[col];
    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator++()
{
    Vector& vec = provide();

    Rational_number res = ++vec(col);

    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator++(int)
{
    Vector& vec = provide();

    Rational_number res = vec(col)++;

    if (!vec) remove();

    return res;

}

Rational_number Matrix::Iterator_Rat::operator--()
{
    Vector& vec = provide();

    Rational_number res = --vec(col);

    if (!vec) remove();

    return res;

}

Rational_number Matrix::Iterator_Rat::operator--(int)
{
    Vector& vec = provide();

    Rational_number res = vec(col)--;

    if (!vec) remove();

    return res;
}



Vector& Matrix::Iterator_Rat::provide()
{
    Node<Vector>* head = master.node;
    if (Node<Vector>::find(row, master.node)) 
        master.node = head = Node<Vector>::insert(row, master.node, Vector(master.cols));
    while(true)
    {
        if (row == head->return_key()) return head->value;
        else if (row < head->return_key()) head = head->return_left();
        else head = head = head->return_right();
    }
}

void Matrix::Iterator_Rat::remove()
{
    master.node = Node<Vector>::remove(row, master.node);
}
