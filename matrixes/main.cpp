#include "rational.h"
#include <stdio.h>


int main()
{
    Rational_number rat, rat2(1, 2);
    Rational_number rat3 = 6;
    rat+= 3;

    rat++;

    rat+= rat2;

    rat = 6 - rat2;

}
