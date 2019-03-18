#include "rational.h"
#include "exception.h"
#include "vector.h"

Vector::~Vector()
{
    while(first) first = Node<Rational_number>::remove(first->key, first);
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

Rational_number Vector::Iterator::find(Node<Rational_number>* p)
{
    if (!p) return 0;
    if (index == p->key) return p->value;
    if (index < p->key) return find(p->left);
    return find(p->right);
}

Vector::Iterator::operator Rational_number()
{
    return find(master.first);
}

Rational_number& Vector::Iterator::provide()
{
    Node<Rational_number>* head = master.first;
    while(true)
    {
        if (!head)
            head = Node<Rational_number>::insert(index, master.first, 0);
        if (index == head->key) return head->value;
        else if(index < head->key) head = head->left;
        else head = head->right;
    }
}

void Vector::Iterator::remove()
{
    Node<Rational_number>::remove(index, master.first);
}


