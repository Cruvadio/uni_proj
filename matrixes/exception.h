#pragma once

#ifndef EXEPTION_H_
#define EXEPTION_H_
#include "matrix.h"
#include <cstdio>
#include <cstring>

class Overflow;
class Zerodivide;
class WrongArgument;
class WrongMatrixSize;
class WrongVectorSize;
class OutOfRangeMatrix;
class OutOfRangeVector;

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

template <class T>
class Matherr : public Exception
{
    public:
        const T& lv,& rv;
         char* op;
        Matherr(const char* op, const T &lv,const T & rv);
        Matherr(const Matherr & m);
        void debug_print();
        ~Matherr();
};

class Overflow : public Matherr<Rational_number>
{
   
    public:
        Overflow(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0) : Matherr(op, lv, rv) {}
        Overflow(const Overflow & ov) : Matherr(ov) {}

        void debug_print();
};

class Zerodivide : public Matherr<Rational_number>
{
    public:
        Zerodivide(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0) : Matherr(op, lv, rv) {}
        Zerodivide(const Zerodivide & zr) : Matherr(zr) {}
        void debug_print();
};

class WrongVectorSize : public Matherr<Vector>
{
    public:
        WrongVectorSize(const char* op = 0,
                        const Vector lv = Vector((unsigned int)0),
                        const Vector rv = Vector((unsigned int)0)) 
                        : Matherr(op, lv, rv) {}
        WrongVectorSize(const WrongVectorSize& mul) : Matherr(mul) {}

        void debug_print();
};

class WrongMatrixSize : public Matherr<Matrix>
{
    public:
        WrongMatrixSize(const char* op = 0, const Matrix& lv = Matrix(0, 0), const Matrix& rv = Matrix(0, 0)) 
                        : Matherr(op, lv, rv) {}
        WrongMatrixSize(const WrongMatrixSize& mat) : Matherr(mat) {}

        void debug_print();
};

class WrongArgument : public Exception
{
    public:
        
        void debug_print();
};

class OpenFileError : public Exception
{
    char* file_name;
    public:
        OpenFileError(const char* file);

        void debug_print();
};

class OutOfRangeVector : public Exception
{
    const Vector& vec;
    unsigned int index;
    public:
        OutOfRangeVector(const Vector& v, unsigned int i) : vec(v), index (i) {}
        OutOfRangeVector(const OutOfRangeVector& v) : vec(v.vec), index(v.index) {}

        void debug_print();
};

class OutOfRangeMatrix : public Exception
{
    const Matrix& mat;
    unsigned int index;
    public:
        OutOfRangeMatrix(const Matrix& m, unsigned int i) : mat(m), index(i) {}
        OutOfRangeMatrix(const OutOfRangeMatrix& m): mat(m.mat), index(m.index) {}

        void debug_print();
};


template <class T>
void Matherr<T>::debug_print()
{
    fprintf(stderr, "Catched exception of mathematical nature\n");
}

template<class T>
Matherr<T>::Matherr(const char* op, const T& lv, const T& rv)
{
    this->lv = lv;
    this->rv = rv;
    if (op)
    {
        this->op = new char[strlen(op) + 1];
        strcpy(this->op, op);
    }
    else op = 0;
}

template<class T>
Matherr<T>::Matherr (const Matherr<T>& m)
{
    lv = m.lv;
    rv = m.rv;
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
