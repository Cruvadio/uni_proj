#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct list
{
    char str[1024];
    struct list* left;
    struct list* right;
}list;

list *remove_space (list* lst)
{
    int i, count = 0;
    list* phead = lst;
    while (lst)
    {
        i = 0;
        
        for (; i < strlen(lst->str); i++)
        {
            if ((lst->str[i] == ' ' || lst->str[i] == '\r' || lst->str[i] == '\t') && i != 0)
                i++;
            while (lst->str[i] == ' ' || lst->str[i] == '\r' || lst->str[i] == '\t')
            {
                memmove(lst->str + i,lst-> str + i + 1, strlen(lst->str) - i);
            }

        }
        if (!strcmp(lst->str, "\0"))
        {
            list *tmp = NULL;
            count++;
            if (lst->left == NULL)
            {
                lst->right->left = NULL;
                phead = lst->right;
            }
            else
                lst->left->right = lst->right;
            tmp = lst->right;
            free(lst);
            lst = tmp;
        }else
            lst = lst->right;
    }
    
    if (count)
    {
        lst = phead;
        while (lst->right != NULL)
        {
            lst = lst->right;
        }
        lst->right =(list *) malloc(sizeof(list));
        if (lst->right == NULL)
        {
            fprintf(stderr, "Can't allocate memory\n");
            return NULL;
        }
        lst->right->right = NULL;
        lst->right->left = lst;
        sprintf(lst->right->str, "\\* deleted - %d *\\", count);

    }
    return phead;
}

void output_list (list * phead)
{
    while (phead)
    {
        printf("%s\n", phead->str);
        phead = phead->right;
    }
}

list * add_element(list* phead, const char* str)
{
    list* lst = NULL;

    lst = (list*) malloc(sizeof(list));
    lst->right = phead;
    if (phead != NULL)
        phead->left = lst;
    lst->left = NULL;
    strcpy(lst->str, str);
    return lst;

}

int main()
{
    list *lst = NULL;

    lst = add_element(lst, "  aaa \t\r bbb");
    lst = add_element(lst, "      ");
    lst = add_element(lst, "Hello    world");
    lst = add_element(lst, "");
    lst = add_element(lst, "    \t\ta aaa   ddsdsdsd");
    

    output_list(lst);

    remove_space(lst);
    
    output_list(lst);
    return 0;

}
