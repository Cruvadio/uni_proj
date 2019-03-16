#include "rational.h"
#include <string.h>
#include <stdlib.h>


Rational_number::Rational_number (const int num)
{
    if (num < 0) 
    {
        sign = -1;
        numerator = -num;
        denominator = 1;
    }
    else
    {
        sign = 1;
        numerator = num;
        denominator = 1;
    }
    string = NULL;
}

Rational_number::Rational_number (const short num)
{
    if (num < 0) 
    {
        sign = -1;
        numerator = -num;
        denominator = 1;
    }
    else
    {
        sign = 1;
        numerator = num;
        denominator = 1;
    }
    string = NULL;
}

Rational_number::Rational_number (const long num)
{
    if (num < 0) 
    {
        sign = -1;
        numerator = -num;
        denominator = 1;
    }
    else
    {
        sign = 1;
        numerator = num;
        denominator = 1;
    }
    string = NULL;
}


Rational_number::Rational_number (const char* ratio)
{
    int i = 0;

    string = new char[strlen(ratio) + 1];

    strcpy(string, ratio);

    if (ratio[0] == '-')
    {
        sign = -1;
        i++;
    }
    else sign = 1;

    char *num, *denom;
    while (i < (int)strlen(ratio))
    {
        if (ratio[i] == '/')
        {
            num = new char[i + 1];
            denom = new char[strlen(ratio) - i];

            for (int j = (sign < 0) ? 1 : 0,k = 0; j <= (int) strlen(ratio); j++, k++)
            {
                if (j < i)
                {
                    num[k] = ratio[j];
                }
                else if (j == i)
                {
                    k = -1;
                }
                else 
                {
                    denom[k] = ratio[j];
                }
               
            }
            numerator = atoi(num);
            denominator = atoi(denom);
            
            delete[] num;
            delete[] denom;
            return;
        }
        i++;
    }
    if (sign < 0)
    {
        const char* str = ratio + 1;
        numerator = atoi(str);
    }
    else numerator = atoi(ratio);
    denominator = 1;
}

Rational_number::Rational_number (const char* num,const char* denom)
{
    numerator = atoi(num);
    denominator = atoi(denom);
    sign = 1;
    string = new char[strlen(num) + strlen(denom) + 2];
    strcpy(string, num);
    strcat(string, "/");
    strcat(string, denom);
   // if (!denominator) throw ERROR;
}

Rational_number::Rational_number (const Rational_number & ratio)
{
    numerator = ratio.numerator;
    denominator = ratio.denominator;
    sign = ratio.sign;
    string = new char[strlen(ratio.string) + 1];
    strcpy(string, ratio);
}

Rational_number& Rational_number::operator=(const Rational_number& rv)
{
    numerator = rv.numerator;
    denominator = rv.denominator;
    sign = rv.sign;

    return *this;
}

const Rational_number operator+ (const Rational_number lv, const Rational_number rv)
{
    Rational_number sum;
    int num;
    num = lv.sign * lv.numerator * rv.denominator + rv.sign * rv.numerator * lv.denominator;
    if (num < 0) 
    {
        sum.sign = -1;
        sum.numerator = -num;
    }
    else 
    {
        sum.sign = 1;
        sum.numerator = num;
    }
    sum.denominator = lv.denominator * rv.denominator;
    return sum;
}

const Rational_number operator- (const Rational_number lv, const Rational_number rv)
{
    Rational_number sub;
    int num;
    num = lv.sign * lv.numerator * rv.denominator - rv.sign * rv.numerator * lv.denominator;
    if (num < 0) 
    {
        sub.sign = -1;
        sub.numerator = -num;
    }
    else 
    {
        sub.sign = 1;
        sub.numerator = num;
    }
    sub.denominator = lv.denominator * rv.denominator;
    return sub;
}

const Rational_number operator* (const Rational_number lv, const Rational_number rv)
{
    Rational_number mul;
    
    mul.numerator = lv.numerator * rv.numerator;
    mul.sign = lv.sign * rv.sign;
    mul.denominator = lv.denominator * rv.denominator;

    return mul;
}

const Rational_number operator/ (const Rational_number lv, const Rational_number rv)
{
    Rational_number div;
    
    div.numerator = lv.numerator * rv.denominator;
    div.sign = lv.sign * rv.sign;
    div.denominator = lv.denominator * rv.numerator;

    return div;
}

bool operator> (const Rational_number& lv, const Rational_number& rv)
{
    return ((lv - rv).sign > 0) && ((lv - rv).numerator != 0);   
}

bool operator< (const Rational_number& lv, const Rational_number& rv)
{
    return ((lv - rv).sign < 0) && ((lv - rv).numerator != 0);   
}

bool operator>= (const Rational_number& lv, const Rational_number& rv)
{
    return ((lv - rv).numerator == 0) || ((lv - rv).sign > 0);   
}

bool operator<= (const Rational_number& lv, const Rational_number& rv)
{
    return ((lv - rv).numerator == 0) || ((lv - rv).sign < 0);   
}
bool operator== (const Rational_number& lv, const Rational_number& rv)
{
    return ((lv - rv).numerator == 0);   
}

bool operator!= (const Rational_number& lv, const Rational_number& rv)
{
    return ((lv - rv).numerator != 0);   
}

Rational_number&  Rational_number::operator+=(const Rational_number& rv)
{
    *this = *this + rv;
    return *this;
}

Rational_number&  Rational_number::operator-=(const Rational_number& rv)
{
    *this = *this - rv;
    return *this;
}

Rational_number&  Rational_number::operator*=(const Rational_number& rv)
{
    *this = *this * rv;
    return *this;
}

Rational_number&  Rational_number::operator/=(const Rational_number& rv)
{
    *this = *this / rv;
    return *this;
}

Rational_number& Rational_number::operator++ ()
{
    *this+=1;
    return *this;
}

Rational_number Rational_number::operator++(int)
{
    Rational_number old = *this;
    *this+=1;
    return old;
}

Rational_number& Rational_number::operator-- ()
{
    *this-=1;
    return *this;
}

Rational_number Rational_number::operator--(int)
{
    Rational_number old = *this;
    *this-=1;
    return old;
}

void Rational_number::floor ()
{
    numerator /= denominator;
    denominator = 1;
}

void Rational_number::round()
{
    uint32_t number_part = numerator / denominator;
    numerator -= number_part * denominator;
    
    double fractional_part = (double) numerator / (double) denominator;
    if ( fractional_part >= 0.5)
    {
        numerator = number_part + 1;
    }
    else
    {
        numerator = number_part;
    }
    denominator = 1;
}

Rational_number Rational_number::get_number_part()
{
    Rational_number number_part(numerator/denominator, 1);
    return number_part;
}

Rational_number Rational_number::get_fractional_part()
{
    Rational_number fractional_part(numerator - (denominator * get_number_part().numerator), denominator);
    return fractional_part;
}

char* Rational_number::to_string ()
{

}

Rational_number::~Rational_number()
{
    if (string != NULL) delete[] string;
}
