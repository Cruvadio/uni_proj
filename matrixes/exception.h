#pragma once

#ifndef EXEPTION_H_
#define EXEPTION_H_
#include "rational.h"

class Matherr;
class Overflow;
class Zerodivide;

class Matherr
{
    public:
        virtual void debug_print();
        virtual ~Matherr();
};

class Overflow : public Matherr
{
    Rational_number lv, rv;
    char* op;
    public:
        Overflow(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0);
        Overflow(const Overflow & ov);

        void debug_print();

        ~Overflow();
};

class Zerodivide : public Matherr
{
    Rational_number lv, rv;
    char* op;
    public:
        Zerodivide(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0);
        Zerodivide(const Zerodivide & zr);
        void debug_print();

        ~Zerodivide();
};

#endif
