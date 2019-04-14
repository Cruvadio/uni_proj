#include "matrix.h"
#include <cstdio>
#include <climits>

int main()
{
    try
    {    
       // mtr.write("matr1.txt");
        Matrix mtr1("matr1.txt");
        
        Rational_number rat = 12.6;

        mtr1(Matrix_col_coord(1)) *= rat;
        printf("%s\n", rat.to_string());
        mtr1.write("matr.txt");
        printf("\n%s", mtr1.to_string());

        
    }
    catch (WrongMatrixSize &ms)
    {
        ms.debug_print();
    }
    catch (OutOfRangeMatrix &om)
    {
        om.debug_print();
    }
    catch(NotANumber &n)
    {
        n.debug_print();
    }
    catch(...)
    {
        throw;
    }
    return 0;
}
