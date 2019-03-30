#include "matrix.h"
#include <cstdio>
#include <climits>

int main()
{
    try
    {
        Matrix mtr(10, 10, Elementary);

        mtr *= 5;

        Matrix mtr1 = mtr + mtr; 
        printf("%s",mtr1.to_string());

        mtr.write("matr.txt");

        mtr(0, 1) = 0;

        mtr.write("matr1.txt");
        /*
        ++vec(0);
        vec(5) = rat;
        

        vec(2) = rat*2;
        
        Vector vec2 = vec * (Rational_number)2;
        vec.write("vector.txt");
        
        vec(2) = 0;
        
        vec(3) = Rational_number(55, 13);
        vec(8) = -30 + rat;
        vec(2) -= rat1;

        vec2 = vec + vec2;

        vec.write("vector.txt");
        vec = vec * (Rational_number)2;
        char* str = vec.to_string();

        printf("%s\n", str);
        vec2.write("vector2.txt");
        delete[] str;
        */
    }
    catch(NotARational &n)
    {
        n.debug_print();
    }
    catch(Matherr &m)
    {
        m.debug_print();
    }
    catch(...)
    {
        throw;
    }
    return 0;
}
