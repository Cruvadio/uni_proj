#include "rational.h"
#include "exception.h"
#include <cstdio>
#include <climits>

int main()
{
    try
    {
        Rational_number rat = Rational_number(USHRT_MAX);

        printf("%s\n", rat.to_string());
    }
    catch(Zerodivide &z)
    {
        z.debug_print();
    }
    catch(Overflow &o)
    {
        o.debug_print();
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
