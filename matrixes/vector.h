#pragma once

#ifndef VECTOR_H_
#define VECTOR_H_

#include "rational.h"
#include "exception.h"

template <class T> class Node
{
    unsigned int key;

    Node<T>* left;
    Node<T>* right;
    static unsigned short return_height(Node<T>* p);
    static int balance_factor(Node<T>* p);

    static Node<T>* rotate_right (Node<T>* p);
    static Node<T>* rotate_left (Node<T>* p);

    static Node<T>* balance_tree(Node<T>* p);
    static Node<T>* find_min(Node<T>* p);
    static Node<T>* remove_min (Node<T>* p);

    public:
        T value;
        Node(const unsigned int& k,const T& val = 0) 
        : value(val), key(k) , left(0) , right(0) {}

        static Node<T>* remove (unsigned int k,Node<T>* p);
        static Node<T>* insert (unsigned int k, Node<T>* p, const T& value);

        unsigned int return_key() {   return key; }
        Node<T>* return_left() {    return left; }
        Node<T>* return_right(){    return right; }

};

enum States
{
    Zeros = 0,
    Ones,
    Elementary,
};

class Vector
{
    Node<Rational_number>* first; 
    unsigned int size;
    int references;

    public:
        class Iterator
        {
            friend class Vector;
            Vector& master;
            unsigned int index;

            Iterator(Vector& a_master, unsigned int ind) : master(a_master), index(ind) {}

            Rational_number& provide();
            Rational_number find(Node<Rational_number>* p);
            void remove();
            public:
                operator Rational_number();
                Rational_number operator=(const Rational_number &rat);
                Rational_number operator+=(const Rational_number &rat);
                Rational_number operator++();
                Rational_number operator++(int);


        };
        friend class Iterator;

        Vector(unsigned int size, States state = Zeros);
        Vector(const Vector& vec);
        Vector(const char* file_name);
        ~Vector();

        Iterator operator[](unsigned int index)
        {
            //if (index > size || index < 0??) throw Exception();
            return Iterator(*this, index);
        }
};

#endif
