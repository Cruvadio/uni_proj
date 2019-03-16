#pragma once

#ifndef _RATIONAL_H
#define _RATIONAL_H

#include <stdint.h>




class Rational_number
{
    uint32_t numerator;
    uint32_t denominator;
    int sign;

    char* string;

    unsigned int gcd (unsigned int a, unsigned int b)
    {
        return (b == 0) ? a : gcd(b, a%b);
    }

    public:
        Rational_number() : numerator(0), denominator(1) , sign(1), string(NULL) {}
        Rational_number(const Rational_number& rat);
        Rational_number(const char* ratio);
        Rational_number(const char* num, const char* denom);
    
        Rational_number(const uint32_t num, const uint32_t denom) : numerator(num), denominator(denom), sign(1) {}
        Rational_number(const int num);
        Rational_number(const short num);
        Rational_number(const long num);

        Rational_number& operator=(const Rational_number& rv);

        friend const Rational_number operator+(const Rational_number lv, const Rational_number rv); 
        friend const Rational_number operator-(const Rational_number lv, const Rational_number rv); 
        friend const Rational_number operator*(const Rational_number lv, const Rational_number rv); 
        friend const Rational_number operator/(const Rational_number lv, const Rational_number rv); 

        friend bool operator>(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator<(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator>=(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator<=(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator==(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator!=(const Rational_number& lv, const Rational_number& rv);


        Rational_number& operator+=(const Rational_number& rv);    

        Rational_number& operator-=(const Rational_number& rv);

        Rational_number& operator*=(const Rational_number& rv);

        Rational_number& operator/=(const Rational_number& rv);


        Rational_number& operator++();
        Rational_number operator++(int);

        Rational_number& operator--();
        Rational_number operator--(int);

        operator short();
        operator int();
        operator long int();
        operator double();

        Rational_number get_number_part();
        Rational_number get_fractional_part();

        void round();
        void floor();
        void make_canonical();

        char* to_string();

        ~Rational_number();
    
};


#endif
