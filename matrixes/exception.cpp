#include "exception.h"
#include "rational.h"
#include <cstdio>
#include <cstring>

// 
// --------------------------------------OVERFLOW-----------------------------------
//
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

//
// ------------------------------------ZERODIVIDE------------------------------------
//


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

//
//------------------------------------WRONG_LEXEME-----------------------------------
//
 
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

//
// ----------------------------------WRONG_VECTOR_SIZE---------------------------------
//

void WrongVectorSize::debug_print()
{
    if (!op)
        fprintf(stderr, "Wrong size of vector met in the constructor!\n");
    else 
    {
        fprintf(stderr, "Can't make '%s'-operation because size of left and right vectors are incorrect!\n",op);
        fprintf(stderr, "Size of vectors should be equal.\n");
    }
}

//
// ----------------------------------WRONG_MATRIX_SIZE---------------------------------
//

void WrongMatrixSize::debug_print()
{
    if (!op)
    {
        fprintf(stderr, "Wrong size of matrix met in the constructor!\n");
        fprintf(stderr, "Rows and columns must be equal.\n");
    }

    else if (!strcmp(op, "^")) fprintf (stderr, "Matrix must have equal rows and columns\n");
    else
    {
        fprintf(stderr, "Can't make '%s'-operation because size of left and right matrix are incorrect!\n",op);
        if (!strcmp(op, "*"))
            fprintf(stderr, "Columns of left matrix and rows of right matrix must be equal");
        else
            fprintf(stderr, "Matrxes must have equal sizes.\n");
    }

}

//
// -----------------------------------OUT_OF_RANGE----------------------------------------
//

void OutOfRangeVector::debug_print()
{
    char* str = vec.to_string();

    fprintf(stderr, "'%u' is out of range of %s\n",index, str);

    delete[] str;
}

void OutOfRangeMatrix::debug_print()
{
    char* str = mat.to_string();

    fprintf(stderr, "'%u' is out of range of \n%s", index, str);

    delete[] str;
}
