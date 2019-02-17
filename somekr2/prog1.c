#include <stdio.h>
#include <math.h>
#include <string.h>

typedef double(*funcptr_t)(double);

struct FunctionTable
{
    char name [256];
    funcptr_t func;
};


struct FunctionTable * add_element (struct FunctionTable *table,const char* str, funcptr_t func)
{
    static int size = 0;
    struct FunctionTable *tmp;
    tmp = (struct FunctionTable *) realloc(table, sizeof(struct FunctionTable) * (size + 1));
    if (tmp == NULL)
    {
        perror("realloc");
        if (table != NULL)
            free(table);
        return NULL;
    }
    table = tmp;
    table[size].func = func;
    strcpy(table[size].name, str);
    return table;
}


int main ()
{

}
