#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

struct list
{
	char* str;

	struct list* nxt;
};

char *read_str (FILE *file, int *err)
{
    int ch = 0;
    int i = 0;
    char* str = NULL;
    char* temp = NULL;
    do
    {
        ch = fgetc(file);
        if (ch == EOF)
        {
            *err = -1;
            return NULL;
        }
        temp =(char*) realloc(str,sizeof(char)*(i + 1));
        if (temp == NULL)
        {
            fprintf(stderr, "Something bad at %lu", (unsigned long)(sizeof(char)*(i + 1)));
            free(str);
            *err = -2;
            break;
        }
        str = temp;

        if (ch == '\n')
        {
            *err = 0;
            str[i] = '\0';
	return str;
        }
        else
            str[i] =(char)ch;
        i++;
    }
    while (ch != '\n');

    return NULL;
}

struct list *add_element (struct list *phead, char *str)
{
	struct list *ph = phead;
	struct list *tmp;
	if (ph == NULL)
	{
		tmp = (struct list*)malloc (sizeof (struct list));
		if (tmp == NULL)
		{
			free(tmp);
			return NULL;
		}
		ph = tmp;
		ph->str = str;
		ph->nxt = NULL;
		return ph;
	}
	
	while (ph->nxt != NULL)
	{
		ph = ph->nxt;
	}
	tmp =(struct list*) malloc (sizeof (struct list));
	if (tmp == NULL)
	{
		free(tmp);
		return NULL;
	}
	ph->nxt = tmp;
	ph->nxt->str = str;
	ph->nxt->nxt = NULL;
	return phead;
}

int make_list (FILE *file,struct list **phead)
{
	struct list *lst = *phead;	
	int err=-2;
	char* str = NULL;
	while (1)
	{
		printf("Reading string...\n");
		str = read_str(file, &err);
       		if (err == -2)
			continue;

		printf("Adding element...\n");
		if (err == -1)
		{
			return 0;
		}
		*phead = add_element(*phead, str);
		if (*phead == NULL)
		{
			return -1;
		}
	}
	return -1;
}	


struct list* delete_return (struct list *phead)
{
	int count = 0;
	struct list *lst = phead, *tmp;
	char str[1000];
	char rtrn[] = "return";
	while (lst != NULL)
	{
		tmp = lst->nxt;
        if (tmp == NULL)
            break;
		if (strstr(tmp->str, rtrn) == NULL)
		{
            lst = lst->nxt;
			continue;
		}
		count++;
        lst->nxt = tmp -> nxt;
		lst = lst-> nxt;
		free(tmp->str);
		free(tmp);
	}
	    if (strstr(phead->str, rtrn) != NULL)
    {
        count++;
        tmp = phead;
        phead = phead->nxt;
        free(tmp->str);
        free(tmp);
    }
    lst->nxt = NULL;
    sprintf(str, "deleted - %d", count);
	phead = add_element(phead, str);
	return phead;
}

void show_lst (struct list *phead)
{
	struct list *lst = phead;
	while (lst != NULL)
	{
		printf("%s\n", lst->str);
		lst = lst->nxt;
	}
}


void clean_list (struct list *phead)
{
	struct list *lst = phead;
	struct list *tmp;
	while(lst->nxt != NULL)
	{
		tmp = lst->nxt;
		lst->nxt = NULL;
		free(lst->str);
		free(lst);
		lst = tmp;
	}
	free(lst);
}

int main (int argc, char* argv[])
{
	FILE *fp = NULL;
	struct list *phead = NULL;
	int err;
	char* str = NULL;
	if (argc < 2)
	{
		fprintf(stderr, "No args.\n");
		return 1;
	}
	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Can't open file %s.\n", argv[1]);
		return 1;
	}
	printf("Making list...\n");
	err = make_list(fp, &phead);
	if (err < 0)
	{
		fprintf(stderr, "Making list is impossible.\n");
		clean_list(phead);
		return 1;
	}
	printf("Showing list...\n");
	show_lst(phead);
	phead = delete_return(phead);
	show_lst(phead);

	/*printf("Cleaning strings...\n");
	free(lst->str);
	lst = lst->nxt;
	while (lst != phead)
	{
		free(phead->str);
	}*/
	printf("Cleaning lists...\n");
	clean_list(phead);
	fclose(fp);
	return 0;

}
