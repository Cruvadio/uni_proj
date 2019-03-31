#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "matrix.h"

char* ReadFile::read_str (FILE* file, int& err)
{
    int i = 0;
    char* str = NULL;
    do
    {
        int ch = fgetc(file);

        char* temp =(char*) realloc(str,sizeof(char)*(i + 1));
        if (temp == NULL)
        {
            free(str);
            //throw Exception();
            break;
        }
        str = temp;
        if (ch == '#')
        {
            while(ch != '\n' && ch != EOF) 
            {
                ch = fgetc(file);
            }
            str[i] = '\0';
            err = (ch == EOF) ? -1 : 0;
            return str;
        }

        if (ch == EOF)
        {
            str[i] = '\0';
            err = -1;
            return str;
        }
        if (ch == '\n')
        {
            err = 0;
            str[i] = '\0';
            return str;
        }
        else
            str[i] =(char)ch;
        i++;
    }
    while (true);

    return NULL;
}

bool ReadFile::is_number (const char* str)
{
    for (int i = 0; i < (int)strlen(str); i++)
    {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}


void ReadFile::read_vector_size(Vector& vec, FILE* file)
{
    while(true)
    {
        int err = -1;
        char* str = read_str(file, err);
        if (err || !str)
        {
            if (str)
                free(str);
            break;
        }
        char* tmp = str;
        char* p = strtok(tmp, " \t\r");
        if (!p) 
        {
            free(str);
            continue;
        }

        if (!strcmp(p, "vector"))
        {
            p = strtok(NULL, " \t\r");
            
            if (!p) throw WrongVectorSize();
            if (!is_number(p)) throw NotANumber(p);

            unsigned int size = atoi(p);

            if (!size) throw WrongVectorSize();

            vec.size = size;
            
            free(str);
            break;
        }

       throw WrongLexeme(str); 
    }
}

void ReadFile::read_vector_file(Vector& vec, FILE* file)
{
    read_vector_size(vec, file);

    while (true)
    {
        int err = -1;
        char* str = read_str(file, err);

        if (err || !str)
        {
            if (str)
                free(str);
            break;
        }
        
        char* tmp = str;
        char* p = strtok(tmp, " \t\r");

        if (!p) 
        {
            free(str);
            continue;
        }
        
        if (!p) throw NotANumber("(null)");
        if (!is_number(p)) throw NotANumber(p);

        unsigned int index = atoi(p);

        p = strtok(NULL, " \t\r");

        if (!p) throw NotANumber("(null)");

        vec.node = Node<Rational_number>::insert(index,vec.node, Rational_number(p));

        free(str);
    }
}


void ReadFile::read_matrix_size(Matrix& mtr, FILE* file)
{
    while(true)
    {
        int err = -1;
        char* str = read_str(file, err);
        if (err || !str)
        {
            if (str)
                free(str);
            break;
        }
        char* tmp = str;
        char* p = strtok(tmp, " \t\r");
        if (!p) 
        {
            free(str);
            continue;
        }

        if (!strcmp(p, "matrix"))
        {
            p = strtok(NULL, " \t\r");
            
            if (!p) throw WrongMatrixSize();
            if (!is_number(p)) throw NotANumber(p);

            unsigned int rows = atoi(p);

            if (!rows) throw WrongMatrixSize();
            
            p = strtok(NULL, " \t\r");

            if (!p) throw WrongMatrixSize();
            if (!is_number(p)) throw NotANumber(p);

            unsigned int cols = atoi(p);

            if (!cols) throw WrongMatrixSize();
            mtr.cols = cols;
            mtr.rows = rows;
            
            free(str);
            break;
        }

       throw WrongLexeme(str); 
    }
}

void ReadFile::read_matrix_file(Matrix& mtr, FILE* file)
{
    read_matrix_size(mtr, file);

    while (true)
    {
        int err = -1;
        char* str = read_str(file, err);

        if (err || !str)
        {
            if (str)
                free(str);
            break;
        }
        
        char* tmp = str;
        char* p = strtok(tmp, " \t\r");

        if (!p) 
        {
            free(str);
            continue;
        }
        
        if (!p) throw NotANumber("(null)");
        if (!is_number(p)) throw NotANumber(p);

        unsigned int row = atoi(p);

        p = strtok(NULL, " \t\r");
        if (!p) throw NotANumber("(null)");
        if (!is_number(p)) throw NotANumber(p);
    
        unsigned int col = atoi(p);
        
        p = strtok(NULL, " \t\r");
        if (!p) throw NotANumber("(null)");

        mtr(row, col) = p;   

        free(str);
    }

}
