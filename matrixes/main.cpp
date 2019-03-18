#include "matrix.h"
#include <cstdio>
#include <climits>

int main()
{
    try
    {
        Rational_number rat("135", "573");
        
        rat = rat / Rational_number();
        printf("%s\n", rat.to_string());
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
