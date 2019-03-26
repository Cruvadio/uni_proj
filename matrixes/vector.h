#pragma once

#ifndef VECTOR_H_
#define VECTOR_H_

#include <cstdio>
#include "rational.h"
#include "exception.h"
#include "node.h"


class MathObject
{
    protected:
        char* read_str(FILE* file, int& err);
    public:
        virtual char* to_string() = 0;
        virtual void write(const char* file_name) = 0;
};

enum States
{
    Zeros = 0,
    Ones,
    Elementary,
};

class Vector : public MathObject
{
    Node<Rational_number>* node; 
    unsigned int size;
    int references;

    void calculations (Vector& vec,Node<Rational_number>* q, char op) const;
    void dot_product(Rational_number& rat, Node<Rational_number>* p, Node<Rational_number>* q) const;
    void calculations(Vector& vec,Rational_number rat, Node<Rational_number>* q, char op) const;
    void copy (Node<Rational_number>* p);
    void remove_all(Node<Rational_number>* p);
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
                Rational_number operator-=(const Rational_number &rat);
                Rational_number operator*=(const Rational_number &rat);
                Rational_number operator/=(const Rational_number &rat);

                Rational_number operator++();
                Rational_number operator++(int);
                Rational_number operator--();
                Rational_number operator--(int);



        };

        class Accessor
        {
            friend class Vector;

            const Vector& master;
            unsigned int index;
            Rational_number find(Node<Rational_number> *p);
            Accessor(const Vector& a_master, unsigned int i) : master(a_master), index(i) {}
            public:
                operator Rational_number();
        };
        friend class Iterator;
        friend class Accessor;
    
        Vector(unsigned int size, States state = Zeros);
        Vector(const Vector& vec);
        Vector(const char* file_name);

        void write_node(FILE* file, Node<Rational_number>* p);
        void write (const char* file_name);
        ~Vector();

        Rational_number operator[](unsigned int index) const
        {
            //if (index > size || index < 0??) throw Exception();
            return Node<Rational_number>::find(index, node);
        }

        Iterator operator() (unsigned int index)
        {
            return Iterator(*this, index);
        }

                
        operator bool();

        Vector operator=(const Vector& rv);

        Vector operator+(const Vector& rv) const;
        Vector operator-(const Vector& rv) const;
        Vector operator*= (const Rational_number& rv);
        Vector operator/=(const Rational_number& rv);

        friend Vector operator* (const Vector& lv,const Rational_number& rv);
        friend Vector operator/ (const Vector& lv,const Rational_number& rv);
        friend Vector operator* (const Rational_number& lv,const Vector& rv);
        
        Rational_number operator*(const Vector& rv) const;



        char* to_string();
};

#endif
