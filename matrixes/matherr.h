#pragma once

#ifndef MATHERR_H_
#define MATHERR_H_

#include <cstring>
#include <cstdio>
#include "matrix.h"

template <class T>
class Matherr
{
    public:
        const T& lv,& rv;
        char* op;
        Matherr(const char* op, const T &lv,const T & rv);
        Matherr(const Matherr & m);
        void debug_print();
        ~Matherr();
};


template <class T>
void Matherr<T>::debug_print()
{
    fprintf(stderr, "Catched exception of mathematical nature\n");
}

template<class T>
Matherr<T>::Matherr(const char* op, const T& a_lv, const T& a_rv): lv(a_lv) , rv(a_rv)
{
    if (op)
    {
        this->op = new char[strlen(op) + 1];
        strcpy(this->op, op);
    }
    else op = 0;
}

template<class T>
Matherr<T>::Matherr (const Matherr<T>& m) : lv(m.lv), rv(m.rv)
{
    if (m.op)
    {
        op = new char[strlen(op) + 1];
        strcpy(op, m.op);
    }
    else op = 0;

}

template<class T>
Matherr<T>::~Matherr()
{
    if (op) delete[] op;
}

#endif
