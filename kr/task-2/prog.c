#include <stdio.h>

unsigned long long swap_bits(unsigned long long num, int *err) 
{
    if (sizeof(num) < sizeof(unsigned long long))
    {
        *err = -1;
        return 0;
    }

    int i = 0;
    const int size = (int)(8 * sizeof(unsigned long long));
    unsigned long long reversed_bits[size];
    unsigned long long one = 1;
    for(; i < size; i++)
    {
        reversed_bits[i] = ((num & (edinichka << i)) ? 1 : 0);
    }

    i = 11;
    for(; i < 18; i++)
    {
        swap(&reversed_bits[i], &reversed_bits[i + 32]);
    }

    num = 0;
    i = 63;
    for(; i >= 0; i--)
    {
        num |= (reversed_bits[i] << i);
    }

    return num;
}

void swap(int* a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main()
{
    unsigned long long num = 0;
    int err = NULL;
    err = (int) malloc(sizeof(int));
    err = 1;
    while ((scanf("%llu", &num)) != EOF)
    {
        num = swap_bits(num, err);
        if (err == -1) 
        {
            printf("gg");
            return 1;
        }
        printf("%llu\n", num);
    }
    return 0;
}
