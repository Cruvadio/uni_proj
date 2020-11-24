#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "shell.h"

/*
 * Making finite state automation for pattern of mgrep
 * AUTOMAT_SIZE is fixed
 */
Automat automat[AUTOMAT_SIZE];

/*
 * Compiling pattern to finite automation
 */
Automat *compile (char* pattern)
{
    int i, state = 0;
    int len = strlen(pattern);
    for (i = 0; i < len; i++)
    {
        /*
         * If we meating '.' than check if next token of pattern is '*' or '+'
         */
        if (pattern[i] == '.' && i < strlen(pattern) - 1)
        {
            switch(pattern[++i])
            {
                case '*':
                    /*
                     * If it is '*' then writing next states like this one or next one
                     */
                    automat[state].ch = '*';
                    automat[state].next1 = state + 1;
                    automat[state].next2 = state++;
                    while (i + 1 < strlen(pattern) - 1 &&  pattern[i + 1] == '*')
                        i++;
                    while (i + 1 < strlen(pattern) - 1 && (pattern[i + 1] == '+' || pattern[i + 1] == '.'))
                    {
                        automat[state].ch = '+';
                        i++;
                    }
                break;
                case '+':
                    /*
                     * If it is '+' then writing next states like this one or next one
                     */
                    automat[state].ch = '+';
                    automat[state].next1 = state + 1;
                    automat[state].next2 = state++;
                    while (i + 1 < strlen(pattern) - 1 && (pattern[i + 1] == '.' || pattern[i + 1] == '*' || pattern[i + 1] == '+'))
                        i++;
                break;
                default:
                    /*
                     * Just making a '.' token that means we are waiting any symbol instead
                     */
                    automat[state].ch = '.';
                    automat[state].next1 = automat[state].next2 = ++state;
                    automat[state].ch = pattern[i];
                    automat[state].next1 = automat[state].next2 = ++state;
                break;
            }            
        }
        else
        {
            /*
             * If there were no '.' token - write regular symbol to automation
             */
            automat[state].ch = pattern[i];
            automat[state].next1 = automat[state].next2 = ++state;
        }
        if (state >= AUTOMAT_SIZE)
        {
            fprintf(stderr, "Regex error: too big pattern\n");
            return NULL;
        }
    }
    /*
     * Making zero token at the end
     */
    automat[state].ch = '\0';
    automat[state].next1 = automat[state].next2 = 0;
    
    return automat;
}

int search_all (char* str, Automat automat[], int mode)
{
    int i = 0, j = 0;
    int is_not_first_time = 0;
    int state = 0;
    int stop = 0;
    int size = 0;
    /*
     * Making size of the automation
     */
    while (automat[state].ch != '\0')
        state++;
    size = state;
    state = 0;
    while (j < strlen(str))
    {
        /*
         * While j is less then lenth of string
         * Check the string to automation
         * If we neen serching only at the beginnig, than we will break after
         * first interation
         */
        i = j;
        stop = 0;
        if (j > 0 && mode == START)
            break;
        while ( i < strlen(str) && !stop)
        {
            /*
             * Checking string at the start i = j
             */
            switch (automat[state].ch)
            {
                case '.':
                    /*
                     * If we met '.' token - skip to next char
                     */
                    state++;
                    i++;
                    continue;
                break;
                case '*':
                    /*
                     * If we met '*' - skip until we met next token after '*'
                     */
                    if (str[i] != automat[automat[state].next1].ch)
                    {
                        i++;
                        state = automat[state].next2;
                    }
                    else
                    {
                        state++;
                    }
                break;
                case '+':
                    /*
                     * If we met '+' - skip until we met next token after '+'
                     * But if next token met instantly - break
                     */
                    if (!is_not_first_time && str[i] == automat[automat[state].next1].ch)
                    {
                        state = 0;
                        stop = 1;
                        break;
                    }
                    else if (str[i] == automat[automat[state].next1].ch && is_not_first_time)
                    {
                        is_not_first_time = 0;
                        state++;
                    }
                    else
                    {
                        state = automat[state].next2;
                        i++;
                        is_not_first_time = 1;
                    }
                break;
                case '\0':
                    /*
                     * We met the end of automation
                     */
                    stop = 1;
                break;
                default:
                    /*
                     * Check if token of automation is equals to char of string
                     * if not - break
                     * else continue
                     */
                    if (str[i] != automat[state].ch)
                    {
                        state = 0;
                        stop = 1;
                        break;
                    }
                    else 
                    {
                        i++;
                        state++;
                    }
                break;
            }
        }
        /*
         * Check if we have a FULL mode, so the whole string should match pattern
         */
        if (i < strlen(str) && mode == FULL)
            break;
        /*
         * If we met the end of automation - we have match the pattern, so return 1
         */
        if (state == size)
             return 1;
        /*
         * Else - we will start check from the next char
         */
        j++;
    }
    
    return 0;
}

/*
 * The same function for checking pattern at the end
 */

int search_at_the_end (char* str, Automat *automat)
{
    int i = 0;
    int is_not_first_time = 0;
    int state = 0;
    int size = 0;
    
    while (automat[state].ch != '\0')
        state++;
    size = --state;
    i = strlen(str) - 1;
    do
    {
        switch (automat[state].ch)
        {
            case '.':
                state--;
                i--;
                continue;
            break;
            case '*':
                if (str[i] != automat[state - 1].ch)
                {
                    i--;
                }
                else
                {
                    state--;
                }
            break;
            case '+':
                if (!is_not_first_time && str[i] == automat[state - 1].ch)
                {
                    return 0;
                }
                else if (str[i] == automat[state - 1].ch && is_not_first_time)
                {
                    is_not_first_time = 0;
                    state--;
                }
                else
                {
                    i--;
                    is_not_first_time = 1;
                }
            break;
            default:
                if (str[i] != automat[state].ch)
                {
                   return 0;
                }
                else 
                {
                    i--;
                    state--;
                }
            break;
        }
    }while (state >=0 && i >=0);
        
        if (state <= 0)
             return 1;    
    return 0;
}


