#include "exception.h"
#include "rational.h"
#include <cstdio>
#include <cstring>

void Matherr::debug_print()
{
    fprintf(stderr, "Catched exception of mathematical nature\n");
}

Matherr::~Matherr()
{}

Overflow::Overflow(const char* op, const Rational_number& lv, const Rational_number& rv)
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
Overflow::Overflow(const Overflow &ov)
{
    lv = ov.lv;
    rv = ov.rv;
    if (ov.op)
    {
        op = new char[strlen(op) + 1];
        strcpy(op, ov.op);
    }
    else op = 0;
}
void Overflow::debug_print()
{
    if (op == 0)
    {
        fprintf(stderr, "Overflow caught at the constructor of 'Rational_number'\n");
        return;
    }
    if (!strcmp(op, "(short)") || !strcmp(op, "(int)") || !strcmp(op, "(long)"))
    {
        char* str_lv = lv.to_string();
        fprintf(stderr, "Can't cast '%s' to %s because of overflow\n", str_lv, op);
        delete[] str_lv;
        return;
    }
    char* str_lv = lv.to_string();
    char* str_rv = rv.to_string();
    fprintf(stderr, "Cought overflow in operation: (%s %s %s)\n", str_lv, op, str_rv);
    delete[] str_lv;
    delete[] str_rv;
}

Overflow::~Overflow()
{
    if (op) delete[] op;
}

Zerodivide::Zerodivide(const char* op, const Rational_number& lv, const Rational_number& rv)
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

Zerodivide::Zerodivide(const Zerodivide &zr)
{
    lv = zr.lv;
    rv = zr.rv;
    if (zr.op)
    {
        op = new char[strlen(op) + 1];
        strcpy(op, zr.op);
    }
    else op = 0;
}

void Zerodivide::debug_print()
{
    if (!op)
    {
        fprintf(stderr, "Zero met at the denominator in constructor\n");
        return;
    }
    char *str_lv = lv.to_string();
    char *str_rv = rv.to_string();

    fprintf(stderr, "Dividing zero in operation in operation: (%s %s %s)\n", str_lv, op , str_rv);
    delete[] str_lv;
    delete[] str_rv;
}

Zerodivide::~Zerodivide()
{
    if (op) delete[] op;
}

