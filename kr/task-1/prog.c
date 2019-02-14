#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
            fprintf(stderr, "Somthing bad at %lu\n", (unsigned long)(sizeof(char)*(i)));
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

char** delete_comments(char** text, int size)
{
	int i, j;
	char buff[32], *tmp;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < strlen(text[i]) - 1; j++)
		{
			if (text[i][j] == '/' && text[i][j + 1] == '/')
			{
				text[i][j + 1] = '*';
				sprintf(buff, "%d", i+1);
                tmp = realloc(text[i], sizeof(char) * (strlen(text[i]) + strlen(buff) + 4));
                if (tmp == NULL)
                {
                    exit(1);
                }
                text[i] = tmp;
                text[i] = strcat(text[i]," \0");
				text[i] = strcat(text[i], buff);
				text[i] = strcat(text[i], "*/\0");
                break;
			}
		}
				
	}
	
	return text;
}


int main (int argc, char** argv)
{	
	FILE* out_file;
	FILE* in_file;
	char **strings = NULL, *str;
	int i = 0, size, err;

	if (argc < 3)
	{
		fprintf(stderr, "No arguments\n Expected two files\n");
		return 1;
	}

	out_file = fopen(argv[1], "r");
	if (out_file == NULL)
	{
		fprintf(stderr, "Can't open file %s", argv[1]);
		return 1;
	}
	in_file = fopen(argv[2], "w");
	if (in_file == NULL)
	{
		fprintf(stderr, "Can't open file %s", argv[2]);
		return 1;
	}
	

	while(1)
	{
		char **temp = NULL;
		temp = (char**) realloc(strings, sizeof(char*)*(i+1));
		if (temp == NULL)
		{
			fprintf(stderr, "Something bad at %lu\n", (unsigned long) sizeof(char*)*(i + 1));
			free(strings);
			return 1;
		}
		strings = temp;
		str = read_str(out_file, &err);
		if (err == -2)
		{
			return 1;
		}
		if (err == -1)
		{
			break;
		}
		strings[i] = str;
		i++;
	}
	size = i;
	strings = delete_comments(strings, size);
	for (i = 0; i < size; i++)
	{
		fprintf(in_file,"%s\n", strings[i]);
		free(strings[i]);
	}
	fclose(out_file);
    fclose(in_file);
	free(strings);
	return 0;



}
