#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "shell.h"

/*
 * Tokens
 */
const char arguments_sep[] = " ";
const char command_sep[] = ";";
const char conveyor_sep[] = "|&";
/*
 * User variables
 */
extern char **variables;
extern int number_of_variables;

/*
 * Variables
 */
char** arguments_of_shell = NULL;
char str_arguments_count_of_shell[6];
char* chld_status = NULL;
char* user = NULL;
char* home = NULL;
char* shell = NULL;
char str_uid[6];
char* pwd = NULL;
char str_pid_shell[6];

/*
 * Reading string from stream
 */
char *read_str (FILE *file, int *err)
{
    int ch = 0;
    int i = 0;
    char* str = NULL;
    char* temp = NULL;
    do
    {
        /*
         * Getting char from stream
         */
        ch = fgetc(file);
        /*
         * If EOF - leaving
         */
        if (ch == EOF)
        {
            *err = -1;
            return NULL;
        }
        /*
         * Starting screening
         */
        if (ch == '\\')
        {
            ch = fgetc(file);
            if (ch == '\n' || ch == '\r')
            {
                printf("> ");
                continue;
            }
            else if (ch == '\t')
            {
                continue;
            }
            else
            {
                temp =(char*) realloc(str,sizeof(char)*(i + 1));
                if (temp == NULL)
                {
                    perror("Allocation memory error");
                    free(str);
                    *err = -2;
                    break;
                }
                str = temp;
                str[i++] = '\\';
            }
        }
        temp =(char*) realloc(str,sizeof(char)*(i + 1));
        if (temp == NULL)
        {
            perror("Allocation memory error");
            free(str);
            *err = -2;
            break;
        }
        str = temp;
        /*
         * Adding char to str if not \n, or adding \0 instead
         */
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
    while (1);

    return NULL;
}


char * ignoring_comments (char* str)
{
    int comm_start = 0;
    
    while (comm_start < strlen(str))
    {
        int size = comm_start;
        char* check = str + comm_start;
        
        if (str[comm_start] == '"')
        {
            /*
             * If we met '"' in command - go to next '"' and make replacement of variables
             */
            size = strcspn(++check, "\"") + comm_start;
            while (1)
            {
                if (str[size] != '\\')
                    break;
                check = str + size + 1;
                size += strcspn(++check, "\"") + 1;
            }
            comm_start = 2 + size;
            continue;
        }
        else if (str[comm_start] == '\'')
        {
            /*
             * If we met '"' in command - go to next '"' and make replacement of variables
             */
            size = strcspn(++check, "'") + comm_start;
            while (1)
            {
                if (str[size] != '\\')
                    break;
                check = str + size + 1;
                size += strcspn(++check, "'") + 1;
            }
            comm_start = 2 + size;
            continue;
        }
        else if (str[comm_start] == '#')
        {
            if (comm_start > 0 && (str[comm_start - 1] == '\\' || str[comm_start - 1] == '$'))
            {
                comm_start++;
                continue;
            }
            str[comm_start] = '\0';
            return str;
        }
        else
        {
            comm_start++;
            continue;
        }
        
    }
    return str;
}
/*
 * Function for cutting strings
 */
char *strcut (char *str, int begin, int end)
{
    int i = begin;
    char * buff = NULL;
    char *tmp = NULL;
    int j = 0;
    if (begin >= end || end > strlen(str) || begin < 0)
        return NULL;
    for (;i < end; i++, j++)
    {
        tmp = realloc (buff, sizeof(char) * (j + 1));
        if (tmp == NULL)
        {
            perror("Allocation memory error");
            free(buff);
            return NULL;
        }
        buff = tmp;
        buff[j] = str[i];
    }
    tmp = realloc (buff, sizeof(char) * (j + 1));
    if (tmp == NULL)
    {
        perror("Allocation memory error");
        free(buff);
        return NULL;
    }
    buff = tmp;
    buff[j] = '\0';
    return buff;
}

/*
 * Separating command into jobs structures
 */

job *commands_separator (char *commands, int *count)
{
    int i = 0;
    char *command = NULL;
    char * comm = NULL, **str_jobs = NULL;
    job *jobs = NULL;
    comm = strdup(commands);
    comm = ignoring_comments(comm);
    
    /*
     * Saparating command with ';' token into strings
     */
    command = strtok(comm, command_sep);
    for (; command != NULL; command = strtok(NULL, command_sep))
    {
        char **tmp = NULL;
        if (strstr(command, "&") != NULL)
            if ((strstr(command, "&") - command) < (strlen(command) - 2))
            {
                INCORRECT_COMMAND(command);
                fprintf (stderr, "'&' must be placed at the end of the command.\n");
                continue;
            }
        tmp = realloc(str_jobs, sizeof(char*) * (i + 1));
        if (tmp == NULL)
        {
            perror("Allocation memory error");
            free(str_jobs);
            return NULL;
        }
        str_jobs = tmp;
        str_jobs[i] = command;
        i++;
    }
    *count = i;
    /*
     * Separating this tokens to job structure
     */
    for (i = 0; i < *count; i++)
    {
        job* tmp = NULL;
        job j;
        j.number_of_programs = 0;
        /*
         * Separator of '|' token and making program structure
         */
        j.programs = conveyor_separator(str_jobs[i], &(j.number_of_programs));
        if (j.programs == NULL || j.number_of_programs == 0)
        {
            i--;
            (*count)--;
            continue;
        }
        tmp = realloc(jobs, sizeof(job) * (i + 1));
        if (tmp == NULL)
        {
            fprintf (stderr, ALLOCATION_ERROR, (unsigned long)sizeof(job) * (i + 1));
            free(jobs);
            return NULL;
        }
        jobs = tmp;
        jobs[i] = j;
        jobs[i].command = strdup(str_jobs[i]);
        if (jobs[i].programs == NULL)
            continue;
        /*
         * Adding background
         */
        if (strstr(str_jobs[i], "&") != NULL)
        {
            jobs[i].background = 1;
        }
        else
            jobs[i].background = 0;
        jobs[i].pgid = 0;
    } 
    free(str_jobs);
    free(comm);
    free(commands);
    return jobs;
    
}
/*
 * Separator of '|' token and making program structure
 */

program* conveyor_separator (char *command, int *count)
{
    int i = 0, err_arg = -1;
    char *p = NULL;
    program *programs = NULL;
    char * comm = NULL, ** str_programs = NULL;
    comm = strdup(command);
    
    p = strtok(comm, conveyor_sep);
    for (; p != NULL; p = strtok(NULL, conveyor_sep))
    {
        
        char** tmp = NULL;
        tmp = realloc(str_programs, sizeof(char*) * (i + 1));
        if (tmp == NULL)
        {
             perror("Allocation memory error");
             free(str_programs);
             return NULL;
        }
        str_programs = tmp;
        str_programs[i] = p;
        i++;      
    }
    *count = i;
    for (i = 0; i < *count; i++)
    {
        program pr;
        program *tmp = NULL;
        /*
         * Making program structure with separating command to arguments and streams
         */
        err_arg = arguments_separator(str_programs[i], &pr);
        if (err_arg == -1)
        {
             i--;
             (*count)--;
             continue;
        }
        tmp = realloc(programs, sizeof(program) * (i + 1));
        if (tmp == NULL)
        {
             perror("Allocation memory error");
             free(programs);
             return NULL;
        }
        programs = tmp;
        programs[i] = pr;
    }
    free(comm);
    free(str_programs);
    return programs;
}

/*
 * Function for inserting one string in certain place of another
 */
char *insert (char* str, int start, int end, char* command)
{
    char* out = NULL;
    char* before, *after;
    
    before = strcut(command, 0, start);
    after = strcut(command, end, strlen(command));
    out = malloc (sizeof(char) *(strlen(str) + ((before != NULL) ? strlen(before) : 0) + ((after != NULL) ? strlen(after) : 0) + 1));
    if (out == NULL)
    {
        perror("Allocation memory error");
        return NULL;
    }
    if (before != NULL)
    {
        strcpy(out, before);
        strcat(out, str);
    }
    else
        strcpy(out, str);
    
    if (after != NULL)
        strcat(out, after);
    
    if (before != NULL)
        free(before);
    if (after != NULL)
        free(after);
    
    return out;
    
}


char *find_last (char* str, char* needle)
{
    char *check = strstr(str, needle);
    while (check < str + strlen(str))
    {
        if (check != NULL && check == str + (strlen(str) - strlen(needle)))
        {
            return check;
        }
        else if (check != NULL)
        {
            check+= strlen(needle);
            check = strstr(check, needle);
        }
        else
        {
            return NULL;
        }
    }
    
    return NULL;
}

char *find_first (char* str, char* needle)
{
    char *check = strstr (str, needle);
    
    if (check != NULL && check == str)
        return check;
    return NULL;
}
/*
 * Function for replacing all neadles in str
 */
char *replace_all (char* str, char *needle, char* replacement)
{
    char *checker = str;
    int index = 0;
    
    if (replacement == NULL)
        return str;
    
    while (checker < str + strlen(str))
    {
        checker = strstr(checker, needle);
        if (checker == NULL)
            break;
        index = (int) (checker - str);
        if (checker != NULL)
        {
            char *temp = NULL;
                    
            temp = insert(replacement, index , index +  strlen(needle), str);
            if (temp == NULL)
            {
                fprintf(stderr, "Can't replace template\n");
                return NULL;
            }
            free(str);
            str = temp;
            checker = str;
            checker+= index + strlen(replacement);
        }
    }
    
    return str;
}

/*
 * Replacing envioroment variables
 */

char* double_quotes_variable_changer (char* command)
{
    int i = 0;
    int num = atoi(str_arguments_count_of_shell);
    command = replace_all (command, "$#", str_arguments_count_of_shell);
    command = replace_all (command, "$?", chld_status);
    command = replace_all (command, "${USER}", user);
    command = replace_all (command, "${HOME}", home);
    command = replace_all (command, "${SHELL}", shell);
    command = replace_all (command, "${UID}", str_uid);
    command = replace_all (command, "${PWD}", pwd);
    command = replace_all (command, "${PID}", str_pid_shell);
    
    for (i = 0 ; i < num; i++)
    {
        char str[4];
        sprintf(str, "$%d", i);
        command = replace_all (command, str, arguments_of_shell[i]);
    }
    return command;
}
/*
 * Finding variable in massive of variables
 */
char* find_var (char* key)
{
    int i;
    for (i = 0; i < number_of_variables; i++)
    {
        if (strstr(variables[i], key)!= NULL && strstr(variables[i], key) < strstr(variables[i], "="))
            return variables[i];
    }
    return NULL;
}
/*
 * Adding variable to massive of variables
 */
int add_to_variables (char* string)
{
    char **tmp = NULL;
    tmp = realloc(variables, sizeof(char*) * (number_of_variables + 1));
    if (tmp == NULL)
    {
        perror("Allocation memory error");
        free(variables);
        return -1;/* ALLOCATION_ERROR */
    }
    variables = tmp;
    variables[number_of_variables++] = string;
    return 0;
}

/*
 * Arguments Separator
 */
int arguments_separator (char *command, program *p_prog)
{
    int arg_start = 0;
    int count = 0;
    char** tmp = NULL;
    char* comm = NULL;
    comm = strdup(command);
    char*  input_file = NULL, *output_file = NULL;
    int output_type = 0;
    
    /*
     * Check if a lot of spaces in front of command
     */
    p_prog->arguments = NULL;
    if (comm[0] == ' ')
    {
        arg_start++;
        if (comm[1] == ' ')
        {
            fprintf (stderr, "Invalid syntax: two much space before command\n");
            return -1;
        }
    }
    while (arg_start < strlen(comm))
    {
        char* argument;
        int size = arg_start;
        char* check = comm + arg_start;
        if (comm[arg_start] == '"')
        {
            /*
             * If we met '"' in command - go to next '"' and make replacement of variables
             */
            size = strcspn(++check, "\"") + arg_start;
            if (size == strlen(comm))
            {
                fprintf(stderr, "Invalid syntax: double qoutes must be closed!\n");
                break;
            }
            while (1)
            {
                if (comm[size] != '\\' || comm[size - 1] == '\\')
                    break;
                check = comm + size + 1;
                size += strcspn(++check, "\"") + 1;
            }
            check = comm + size + 1;
            argument = strcut(comm, ++arg_start, ++size);
            arg_start = 1 + size + strcspn(check, " ");
            argument = double_quotes_variable_changer(argument);
            argument = replace_all(argument, "\\\"", "\"");
        }
        else if (comm[arg_start] == '\'')
        {
            /*
             * If we met '\'' in ccommand - go to next '\''
             */
            size = strcspn(++check, "'") + arg_start;
            if (size == strlen(comm))
            {
                fprintf(stderr, "Invalid syntax: qoutes must be closed!\n");
                break;
            }
            while (1)
            {
                if (comm[size] != '\\' || comm[size - 1] == '\\')
                    break;
                check = comm + size + 1;
                size += strcspn(++check, "'") + 1;
            }
            check = comm + size + 1;
            argument = strcut(comm, ++arg_start, ++size);
            argument = replace_all(argument, "\\'", "'");
            arg_start = 1 + size + strcspn(check, " ");
        }
        /*
         * If we met input stream sign - add it to input_file
         */
        else if (comm[arg_start] == '<')
        {
            if (arg_start + 1 >= strlen(comm))
            {
                fprintf(stderr, "Invalid syntax: file name was expexted!\n");
                fprintf(stderr, "Stdin won't be redirected!\n");
                break;
            }
            if (comm[arg_start + 1] == ' ')
            {
                check+=2;
                arg_start+=2;
            }
            else
            {
                check++;
                arg_start++;
            }
            size = strcspn(check, " ") + arg_start;
            input_file = strcut(comm, arg_start, size++);
            arg_start = size;
            continue;
            
        }
        /*
         * Do this also for output stream sign
         */
        else if (comm[arg_start] == '>')
        {
            if (arg_start + 1 >= strlen(comm))
            {
                fprintf(stderr, "Invalid syntax: file name was expexted!\n");
                fprintf(stderr, "Stdout won't be redirected!\n");
                break;
            }
            /*
             * Check if second sign exists to make append mode
             */
            if (comm[arg_start + 1] == '>')
            {
                if (comm[arg_start + 2] == ' ')
                {
                    arg_start+=3;
                    check+=3;
                }
                else
                {
                    arg_start+=2;
                    check+=2;
                }
                output_type = 2;
            }
            else
            {
                if (comm[arg_start + 1] == ' ')
                {
                    check+=2;
                    arg_start+=2;
                }
                else
                {
                    check++;
                    arg_start++;
                }
                output_type = 1;
            }
            size = strcspn(check, " ") + arg_start;
            output_file = strcut(comm, arg_start, size++);
            arg_start = size;
            continue;
            
        }
        else
        {
            /*
             * If no signs were met - go to next space-character
             */
            size = strcspn(check, " ") + arg_start;
            argument = strcut(comm, arg_start, size++);
            if (count < 1 && strstr(argument, "=") != NULL)
            {
                add_to_variables(argument);
                free(comm);
                return -1;
                
            }
            arg_start = size;
            argument = double_quotes_variable_changer(argument);
        }
        
        /*
         * Add it to arguments of program
         */
        
        tmp = realloc (p_prog->arguments, sizeof(char*) * (count + 1));
        if (tmp == NULL)
        {
            perror("Allocation memory error");
            free(p_prog->arguments);
            return -1;/* ALLOCATION_ERROR */
        }

        
        p_prog->arguments = tmp;
        p_prog->arguments[count] = argument;
        count++;
    }
    /*
     * Setting other sructure's fields
     */
    p_prog->number_of_arguments = count;
    tmp = realloc (p_prog->arguments, sizeof(char*) * (count + 1));
    if (tmp == NULL)
    {
        perror("Allocation memory error");
        free(p_prog->arguments);
        return -1;/* ALLOCATION_ERROR */
    }
    p_prog->status = 0;
    p_prog->completed = 0;
    p_prog->stopped = 0;
    p_prog->pid = 0;
    p_prog->input_file = input_file;
    p_prog->output_file = output_file;
    p_prog->output_type = output_type;
    p_prog->arguments = tmp;
    p_prog->name = strdup(p_prog->arguments[0]);
    p_prog->arguments[count] = NULL;
    free(comm);
    return 0;
}
