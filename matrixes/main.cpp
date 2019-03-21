#include "matrix.h"
#include <cstdio>
#include <climits>

int main()
{
    try
    {
        Rational_number rat("135", "573");
        Vector vec(10, Ones);

        vec(0)++;
        vec(5) = rat;

        vec(2) = rat*2;

        vec.write("vector.txt");

        char* str = vec.to_string();

        printf("%s\n", str);
        
        delete[] str;
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
