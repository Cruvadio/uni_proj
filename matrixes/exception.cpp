#include "exception.h"
#include "rational.h"
#include <cstdio>
#include <cstring>

void Matherr::debug_print()
{
    fprintf(stderr, "Catched exception of mathematical nature\n");
}

Matherr::Matherr(const char* op, const Rational_number& lv, const Rational_number& rv)
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

Matherr::Matherr (const Matherr& m)
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

Matherr::~Matherr()
{
    if (op) delete[] op;
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

void Zerodivide::debug_print()
{
    if (!op)
    {
        fprintf(stderr, "Zero met at the denominator in constructor\n");
        return;
    }
    char *str_lv = lv.to_string();
    char *str_rv = rv.to_string();

    fprintf(stderr, "Dividing zero in operation: (%s %s %s)\n", str_lv, op , str_rv);
    delete[] str_lv;
    delete[] str_rv;
}

WrongLexeme::WrongLexeme(const char* lexeme)
{
    if (!lexeme) this->lexeme = 0;
    else
    {
        this->lexeme = new char[strlen(lexeme) + 1];
        strcpy(this->lexeme, lexeme);
    }
}

void WrongLexeme::debug_print()
{
    fprintf(stderr, "Wrong lexeme '%s'!\n", lexeme);
}

WrongLexeme::~WrongLexeme()
{
    if (lexeme) delete[] lexeme;
}

void NotARational::debug_print()
{
    fprintf(stderr, "'%s' is not a rational number!\n", lexeme);
}

Exception::~Exception()
{
}
