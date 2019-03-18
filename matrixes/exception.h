#pragma once

#ifndef EXEPTION_H_
#define EXEPTION_H_
#include "rational.h"

class Matherr;
class Overflow;
class Zerodivide;

class Exception
{
    public:
        virtual void debug_print() = 0;
        virtual ~Exception();
};

class WrongLexeme :public Exception
{
    protected:
        char* lexeme;
    public:
        WrongLexeme(const char* lexeme = 0);
        void debug_print();
        ~WrongLexeme();
};

class NotARational : public WrongLexeme
{
    public:
        NotARational(const char* lexeme = 0) : WrongLexeme(lexeme) {}
        void debug_print();
};
class Matherr : public Exception
{
    protected:
         Rational_number lv, rv;
         char* op;
    public:
        Matherr(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0);
        Matherr(const Matherr & m);
        void debug_print();
        ~Matherr();
};

class Overflow : public Matherr
{
   
    public:
        Overflow(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0) : Matherr(op, lv, rv) {}
        Overflow(const Overflow & ov) : Matherr(ov) {}

        void debug_print();
};

class Zerodivide : public Matherr
{
    public:
        Zerodivide(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0) : Matherr(op, lv, rv) {}
        Zerodivide(const Zerodivide & zr) : Matherr(zr) {}
        void debug_print();
};

#endif
