#include "rational.h"
#include "exception.h"
#include <cstring>
#include <stdint.h>
#include <cstdlib>
#include <climits>
#include <cstdio>


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
}

Rational_number::Rational_number (const long num)
{
    if (num < 0) 
    {
        sign = -1;
        if (-num > UINT32_MAX) throw Overflow();
        numerator = -num;
        denominator = 1;
    }
    else
    {
        sign = 1;
        if (num > UINT32_MAX) throw Overflow();
        numerator = num;
        denominator = 1;
    }
}


Rational_number::Rational_number (const char* ratio)
{
    int i = 0;

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
                    num[k] = '\0';
                    k = -1;
                }
                else 
                {
                    denom[k] = ratio[j];
                }
               
            }
            denom[strlen(ratio) - i - 1] = '\0';

            if (!is_number(num) || !is_number(denom)) throw NotARational(ratio);

            numerator = atoi(num);
            denominator = atoi(denom);
           
            if (denominator == 0) throw Zerodivide();
            delete[] num;
            delete[] denom;
            return;
        }
        i++;
    }
    if (sign < 0)
    {
        const char* str = ratio + 1;
        if (!is_number(str)) throw NotARational(ratio);
        numerator = atoi(str);
    }
    else numerator = atoi(ratio);
    denominator = 1;
}

bool Rational_number::is_number (const char* str)
{
    for (int i = 0; i < (int)strlen(str); i++)
    {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

Rational_number::Rational_number (const char* num,const char* denom)
{
    if (!is_number(num)) throw NotARational (num);
    if (!is_number(denom)) throw NotARational(denom);
    numerator = atoi(num);
    denominator = atoi(denom);
    sign = 1;
    if (!denominator) throw Zerodivide();
}

Rational_number::Rational_number (const Rational_number & ratio)
{
    numerator = ratio.numerator;
    denominator = ratio.denominator;
    if (!denominator) throw Zerodivide();
    sign = ratio.sign;
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
    int64_t num;

    if (((lv.sign > 0 && rv.sign > 0) || (lv.sign < 0 && rv.sign < 0)) 
        &&((uint64_t)lv.numerator *(uint64_t)rv.denominator 
        + (uint64_t)rv.numerator * (uint64_t)lv.denominator) > (uint64_t)UINT32_MAX) 
        throw Overflow("+", lv, rv);

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
    if (lv.denominator * rv.denominator > UINT32_MAX) throw Overflow("+", lv, rv);
    sum.denominator = lv.denominator * rv.denominator;
    return sum;
}

const Rational_number operator- (const Rational_number lv, const Rational_number rv)
{
    Rational_number sub;
    int64_t num;
    
    if (((lv.sign < 0 && rv.sign > 0) || (lv.sign > 0 && rv.sign < 0)) 
        &&((uint64_t)lv.numerator *(uint64_t)rv.denominator 
        + (uint64_t)rv.numerator * (uint64_t)lv.denominator) > (uint64_t)UINT32_MAX) 
        throw Overflow("-", lv, rv);

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
    if (lv.denominator * rv.denominator > UINT32_MAX) throw Overflow("-", lv, rv);
    sub.denominator = lv.denominator * rv.denominator;
    return sub;
}

const Rational_number operator* (const Rational_number lv, const Rational_number rv)
{
    Rational_number mul;
    
    if ((uint64_t)lv.numerator * (uint64_t)rv.numerator > (uint64_t)UINT32_MAX || lv.denominator * rv.denominator > UINT32_MAX)
        throw Overflow("*", lv, rv);
    mul.numerator = lv.numerator * rv.numerator;
    mul.sign = lv.sign * rv.sign;
    mul.denominator = lv.denominator * rv.denominator;

    return mul;
}

const Rational_number operator/ (const Rational_number lv, const Rational_number rv)
{
    Rational_number div;
    if (!rv.numerator) throw Zerodivide("/", lv, rv);
    if (lv.numerator * rv.denominator > UINT32_MAX || lv.denominator * rv.numerator > UINT32_MAX)
        throw Overflow("/", lv, rv);
    div.numerator = lv.numerator * rv.denominator;
    div.sign = lv.sign * rv.sign;
    div.denominator = lv.denominator * rv.numerator;

    return div;
}

Rational_number& Rational_number::operator+()
{
    return *this;
}

Rational_number Rational_number::operator-()
{
    Rational_number tmp = *this;
    tmp.sign = -sign;
    return tmp;
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

Rational_number Rational_number::floor ()
{
    return get_number_part();
}

Rational_number Rational_number::round()
{   
    double fractional_part = (double) numerator / (double) denominator;
    if ( fractional_part >= 0.5)
    {
        return get_number_part() + 1;
    }
    return get_number_part();
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

char* Rational_number::to_string () const
{
    int num = numerator,denom = denominator,i, j;
    for (i = 0; num != 0; i++)
    {
        num /= 10;
    }

    for (j = 0; denom != 0; j++)
    {
        denom /= 10;
    }

    char *str = new char[i + ((denominator != 1)? j + 1 : 0) + ((sign < 0) ? 3 : 2)];
    
    if (denominator != 1)
    {
        if (sign > 0)
            sprintf(str, "%u/%u", (unsigned int)numerator, (unsigned int)denominator);
        else
            sprintf(str, "-%u/%u", (unsigned int)numerator, (unsigned int) denominator);
    }
    else
    {
        if (sign < 0) 
            sprintf(str, "-%u", (unsigned int)numerator);
        else
            sprintf(str, "%u", (unsigned int) numerator);
    }
    return str;
}

void Rational_number::make_canonical ()
{
    int c = gcd(numerator, denominator);
    numerator /= c;
    denominator /= c;
}
Rational_number::operator int()
{
    Rational_number rat = get_number_part();

    if (rat.numerator * sign > INT_MAX ||(int64_t) sign * rat.numerator < INT_MIN) 
        throw Overflow("(int)", *this);
    return rat.numerator * sign;
}

Rational_number::operator short()
{
    Rational_number rat = get_number_part();
    if (rat.numerator * sign > SHRT_MAX || (int64_t)rat.numerator * sign < SHRT_MIN) 
        throw Overflow("(short)", *this);
    return rat.numerator * sign;
}

Rational_number::operator long()
{
    Rational_number rat = get_number_part();
    if ((int64_t)rat.numerator * sign > LONG_MAX || (int64_t)rat.numerator * sign < LONG_MIN)
        throw Overflow("(long)", *this);
    return rat.numerator * sign;
}
Rational_number::operator double()
{
    return (double) numerator/ (double) denominator;
}

Rational_number::~Rational_number()
{
}
