#define _GNU_SOURCE

#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include "shell.h"

/*
 * Variables
 */
extern char** arguments_of_shell;
extern char str_arguments_count_of_shell[6];
extern char* chld_status;
extern char* user;
extern char* home;
extern char* shell;
extern char str_uid[6];
extern char* pwd;
extern char str_pid_shell[6];
/*
 * Eviron and user variables
 */
extern char **environ;
extern char **variables;
extern int number_of_variables;

/*
 * Jobs
 */
job *jobs = NULL;
job *active_jobs = NULL;
int number_of_jobs = 0;
int number_of_active = 0;

int stopped_conveyor = 0;
/*
 * Launch process
 */
void launch_process (program *p, job *j)
{
    int pid = getpid();
    int err = -1; 
    /*
     * If shell is interactive - make its own process group
     */
    if (shell_is_interactive)
    {
        if (j->pgid == 0) 
                j->pgid = pid;
            setpgid(pid, j->pgid);
        /*
         * Setting signals to default
         */
        signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
    }
    /*
     * If it has its own file streams - then redirecting to them
     */
    err = redirect_stream(*p);
    if (err)
    {
        close_all(EXIT_FAILURE);
    }
    /*
     * Checking if it is an embedded program
     */
    err = is_embedded(*p);
    if (err)
    {
        err = execute_embedded(*p);
        if (!err)
            close_all(EXIT_SUCCESS);
        else
            close_all(EXIT_FAILURE);
    }
    /*
     * If not - execvpe it
     */

    execvpe(p->name, p->arguments, environ);
    perror(p->name);
    close_all(EXIT_FAILURE);
    
    
}

/*
 * Launching jobs
 */
int launch_job (job *j)
{
    int p[2];
    pid_t pid;
    int i = 0;
    int jid = -1;
    int input = STDIN_FILENO;
    int outfile = STDOUT_FILENO;

    int in = dup(STDIN_FILENO);
    
    /*
     * Add job to active
     */

    jid = add_job(j);

    printf ("JID = %d\n", jid);
    if (jid == -1)
    {
        fprintf(stderr, "Can not add job\n");
        return -1;
    }
    for (i = 0; i < j->number_of_programs; i++)
    {
        /*
         * Making pipe, if necessary
         */
        if (i < j->number_of_programs - 1)
        {
            if (pipe(p) < 0)
            {
                perror("Can't create pipe");
                return -1;
            }
            outfile = p[1];
        }
        /*
         * If we have only one program, some embedded programs should be run without fork()
         */
        if (strcmp(j->programs[i].name,"cd") == 0 && j->number_of_programs == 1)
        {
            int err;
            err = change_dir(j->programs[i]);
            j->pgid = shell_pgid;
            active_jobs[jid].programs[i].pid = getpid();
            active_jobs[jid].pgid = shell_pgid;
            return err;
        }
        if (strcmp(j->programs[i].name,"fg") == 0 && j->number_of_programs == 1)
        {
            int err;
            err = fg(j->programs[i]);
            j->pgid = shell_pgid;
            active_jobs[jid].programs[i].pid = getpid();
            active_jobs[jid].pgid = shell_pgid;
            return err;
        }
        if (strcmp(j->programs[i].name,"bg") == 0 && j->number_of_programs == 1)
        {
            int err;
            err = bg(j->programs[i]);
            j->pgid = shell_pgid;
            active_jobs[jid].programs[i].pid = getpid();
            active_jobs[jid].pgid = shell_pgid;
            return err;
        }
        if (strcmp(j->programs[i].name, "export") == 0 && j->number_of_programs == 1)
        {
            int err;
            err = export_vars(j->programs + i);
            j->pgid = shell_pgid;
            active_jobs[jid].programs[i].pid = getpid();
            active_jobs[jid].pgid = shell_pgid;
            return err;
        }
        /*
         * Making new process
         */
        pid = fork();
        if (pid == 0)
        {
            dup2(in, STDIN_FILENO);
            close(in);    
            if (input != STDIN_FILENO)
            {
                dup2(input, STDIN_FILENO);
                close(input);
            }
            if (outfile != STDOUT_FILENO)
            {
                dup2 (outfile, STDOUT_FILENO);
                close(outfile);
            }
            launch_process(&(j->programs[i]), j);
        }
        else if (pid < 0)
        {
            perror("Can't create new process");
            return -1;
        }
        else
        {
            /*
             * Setting active job with pgids and pids
             */
            active_jobs[jid].programs[i].pid = pid;
            if (shell_is_interactive)
            {
                if (!active_jobs[jid].pgid)
                active_jobs[jid].pgid = j->pgid = pid;
            }
            /*
             * Close pipes, if necessary
             */
            if (input != STDIN_FILENO)
                close(input);
            if (outfile != STDOUT_FILENO)
                close(outfile);

            /*
             * Make output of this program input of next one
             */
            input = p[0];
        }
    }
    
    /*
     * If background - show that it has launched
     */
    if (active_jobs[jid].background)
        format_job_info (active_jobs + jid, "launched");
    /*
     * If shell isn't interactive - wait fo jobs
     */
    if (!shell_is_interactive)
        wait_for_job(active_jobs + jid);
    /*
     * Else - launch them foreground or background depending on them
     */
    else if (!(active_jobs[jid].background))
        foreground(active_jobs + jid, 0);
    else
        background(active_jobs + jid, 0);

    dup2(in, STDIN_FILENO);
    close(in);
    return 0;
}

/*
 * Redirecting stream whith files
 */
int redirect_stream (program pr)
{
    int fd;
    if (pr.input_file != NULL)
    {
        fd = open (pr.input_file, O_RDONLY, 0600);
        if (fd == -1)
        {
            fprintf(stderr, "Can't open file %s: %s", pr.input_file, strerror(errno));
            return -1;
        }
        dup2(fd, 0);
        close(fd);
    }
    if (pr.output_file != NULL)
    {
        fflush(stdout);
        if (pr.output_type == 1)
            fd = open (pr.output_file, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        else
            fd = open (pr.output_file, O_WRONLY | O_CREAT | O_APPEND, 0600);
        if (fd == -1)
        {
            fprintf(stderr, "Can't open file %s: %s", pr.input_file, strerror(errno));
            return -1;
        }
        dup2(fd, 1);
        close(fd);
    }
    return 0;
}

/*
 * "bg" - command
 */
int bg (program pr)
{
    int jid, err;
    job* j;
    if (pr.number_of_arguments < 2)
    {
        fprintf(stderr, "Not enough arguments!\nJob id expected!\n");
        return -1;
    }
    jid = atoi(pr.arguments[1]);
    j = find_bg_or_stopped_job(jid - 1);
    if (j == NULL)
    {
        fprintf (stderr, "Incorrect job id!\n");
        return -1;
    }
        
    mark_job_as_running (j);
    err = background(j, 1);
    return err;
}
/*
 * "fg" - command
 */
int fg (program pr)
{
    int jid, err;
    job* j;
    if (pr.number_of_arguments < 2)
    {
        fprintf(stderr, "Not enough arguments!\nJob id expected!\n");
        return -1;
    }
    jid = atoi(pr.arguments[1]);
    j = find_bg_or_stopped_job(jid - 1);
    if (j == NULL)
    {
        fprintf (stderr, "Incorrect job id!\n");
        return -1;
    }
    mark_job_as_running (j);
    err = foreground(j, 1);
    return err;
}

/*
 * Executing embedded programs
 */
int execute_embedded(program pr)
{
    int err = -1;
    if (!strcmp(pr.name,"cd"))
    {
        err = change_dir(pr);
        return err;
    }
    else if (!strcmp(pr.name,"pwd"))
    {
        print_working_directory();
        return 0;
    }
    else if(!strcmp(pr.name,"exit"))
    {
        kill( -shell_pgid, SIGUSR1);
        return 0;
    }
    else if (!strcmp(pr.name,"jobs"))
    {
        do_job_notification();
        return 0;
    }
    else if (!strcmp(pr.name,"bg"))
    {
        err = bg(pr);
        return err;
    }
    else if (!strcmp(pr.name, "fg"))
    {
        err = fg(pr);
        return err;
    }
    else if (!strcmp(pr.name, "mcat"))
    {
        return mcat(&pr);
    }
    else if (!strcmp(pr.name, "msed"))
    {
        return msed (&pr);
    }
    else if (!strcmp(pr.name, "mgrep"))
    {
        return mgrep (&pr);
    }
    else if (!strcmp(pr.name, "export"))
    {
        return export_vars(&pr);
    }
    return -1;
}
/*
 * Check if embedded
 */
int is_embedded (program pr)
{
    if (!strcmp(pr.name,"cd"))
    {
        return 1;
    }
    else if (!strcmp(pr.name,"pwd"))
    {
        return 1;
    }
    else if(!strcmp(pr.name,"exit"))
    {
        return 1;
    }
    else if (!strcmp(pr.name,"jobs"))
    {
        return 1;
    }
    else if (!strcmp(pr.name,"bg"))
    {
        return 1;
    }
    else if (!strcmp(pr.name,"fg"))
    {
        return 1;
    }
    else if (!strcmp(pr.name, "mcat"))
    {
        return 1;
    }
    else if (!strcmp(pr.name, "export"))
    {
        return 1;
    }
    else if (!strcmp(pr.name, "msed"))
    {
         return 1;
    }
    else if (!strcmp(pr.name, "mgrep"))
    {
         return 1;
    }
    return 0;
}
/*
 * Mark stopped job as running
 */
void mark_job_as_running (job *j)
{
    int i;
    for (i = 0; i < j->number_of_programs; i++)
        j->programs[i].stopped = 0;
}

/*
 * Format info about job
 */
void format_job_info (job *j, const char *status)
{
    fprintf (stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}
/*
 * Check and mark processes statuses
 */
int mark_process_status (pid_t pid, int status)
{
    int i;
    if (pid > 0)
        {
        /* 
         * Update the record for the process.  
         */
            for (i = 0; i < number_of_active; i++)
            {
                int j;
                for (j = 0; j < active_jobs[i].number_of_programs; j++)
                    if (active_jobs[i].programs[j].pid == pid)
                    {
                        active_jobs[i].programs[j].status = status;
                        if (WIFSTOPPED (status))
                            active_jobs[i].programs[j].stopped = 1;
                        else
                        {
                            active_jobs[i].programs[j].completed = 1;
                            if (WIFSIGNALED (status))
                            fprintf (stderr, "%d: Terminated by signal %d.\n",
                                    (int) pid, WTERMSIG (active_jobs[i].programs[j].status));
                        }
                        return 0;
                    }
            return -1;
            }
        }
    else if (pid == 0 || errno == ECHILD)
        /* 
         * No processes ready to report.  
         */
        return -1;
    /* 
     * Other weird errors. 
     */
    perror ("waitpid");
    return -1;
}
/*
 * Update status of process
 */
void update_status (void)
{
  int status;
  pid_t pid;

  do
    pid = waitpid (WAIT_ANY, &status, WUNTRACED|WNOHANG);
  while (!mark_process_status (pid, status));
}
/*
 * Add job to active
 */
int add_job (job *j)
{
    job* tmp = NULL;
    
    tmp = realloc(active_jobs, sizeof(job) * (number_of_active + 1));
    if (tmp == NULL)
    {
        perror("Allocation memory error");
        clean_jobs(active_jobs, number_of_active);
        return -1;
    }
    active_jobs = tmp;
    active_jobs[number_of_active].pgid = j->pgid;
    active_jobs[number_of_active].command = strdup(j->command);
    active_jobs[number_of_active].background = j->background;
    active_jobs[number_of_active].number_of_programs = j->number_of_programs;
    active_jobs[number_of_active++].programs = conveyor_separator(j->command, &(j->number_of_programs));
    return number_of_active - 1;
}
/*
 * Remove job from active
 */
void remove_job (job j)
{
    job *tmp;
    int i, k;
    
    tmp = malloc(sizeof(job) *(number_of_active - 1));
    if (tmp == NULL)
    {
        perror("Allocation memory error");
        return;
    }
    for (i = 0, k = 0; i < number_of_active; i++)
    {
        if (active_jobs[i].pgid == j.pgid)
            continue;
        tmp[k] = active_jobs[i];
        k++;
    }
    clean_job(j);
    free(active_jobs);
    active_jobs = tmp;
    number_of_active--;
}
/*
 * Find stopped or active background job
 */
job* find_bg_or_stopped_job (int jid)
{
    int i, j;
    for (i = 0, j = 0; i < number_of_active; i++)
    {
        if (active_jobs[i].background || job_is_stopped(active_jobs + i))
        {
            if (j == jid)
                return active_jobs + i;
            j++;
        }
    }
    return NULL;
}
/*
 * Do job notification if they are stopped or background
 */
void do_job_notification (void)
{
    int i, j;

    update_status ();
    for (i = 0, j = 0; i < number_of_active; i++)
    {
            if (job_is_completed (active_jobs + i)) 
            {
                if (active_jobs[i].background)
                {
                    fprintf(stderr, "[%d] ", j + 1);
                    format_job_info (active_jobs + i, "completed");
                    j++;
                }
                break;
            }

            else if (job_is_stopped (active_jobs + i)) 
            {
                    fprintf(stderr, "[%d] ", j + 1);
                    format_job_info (active_jobs + i, "stopped");
                    j++;
            }
            else
            {
                if (active_jobs[i].background)
                {
                    fprintf(stderr, "[%d] ", j + 1);
                    format_job_info(active_jobs + i, "executing");
                    j++;
                }
            }
    }
}
/*
 * Find job with pgid
 */
job *find_job (pid_t pgid)
{
    int i;
    if (number_of_active <= 0)
        return NULL;
    for (i = 0; i < number_of_active; i++)
        if (active_jobs[i].pgid == pgid)
        return (active_jobs + i);
    return NULL;
}

/*
 * Check if job is stopped
 */

int job_is_stopped (job *j)
{
    int i;
    if (j != NULL)
    {
        if (number_of_active <= 0)
            return 0;
        for (i = 0; i < j->number_of_programs; i++)
            if (!j->programs[i].completed && !j->programs[i].stopped)
            return 0;
        return 1;
    }
    return 0;
}

/*
 * Check if job is completed
 */
int job_is_completed (job *j)
{
    int i;
    if (j != NULL)
    {
        if (number_of_active <= 0)
            return 0;
        for (i = 0; i < j->number_of_programs; i++)
            if (!j->programs[i].completed)
                return 0;
        return 1;
    }
    return 0;
}
/*
 * Wait job
 */
void wait_for_job (job *j)
{
    int status;
    pid_t pid;

    do
        pid = waitpid (WAIT_ANY, &status, WUNTRACED);
    while (!mark_process_status (pid, status)
           && !job_is_stopped (j)
           && !job_is_completed (j));
    chld_status = return_status(status);
}
/*
 * Launch job in foreground
 */
int foreground (job *j, int cont)
{
    /*
     * Make it in foreground 
     */
    if (tcsetpgrp (shell_terminal, j->pgid) < 0)
    {
        perror("Can't change foreground job");
    }

    /*
     * If it was stopped - throw SIGCONT
     */
    if (cont)
    {
        if (kill (- j->pgid, SIGCONT) < 0)
        {
            perror ("kill (SIGCONT)");
            return -1;
        }
        
    }
    /*
     * Wait for it
     */
    wait_for_job(j);
    
    /*
     * Remove job from active one, because it is ended
     */
    
    remove_job(*j);

    /*
     * Return shell to foreground
     */
    tcsetpgrp (shell_terminal, shell_pgid);
    tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);
    
    return 0;
}
/*
 * Launch job in background
 */
int background (job *j, int cont)
{
    /*
     * If it was stopped - throw SIGCONT
     */
    if (cont)
        if (kill (-j->pgid, SIGCONT) < 0)
        {
            perror ("kill (SIGCONT)");
            return -1;
        }
    return 0;
}
/*
 * Change directory
 */
int change_dir(program cd)
{
    struct stat st;
    int err = -1;
    int wave = 0;
        
    
    if (cd.number_of_arguments < 2)
    {
        fprintf(stderr, "No arguments: name of directory expected!\n");
        return -1;
    }
    wave = strcspn(cd.arguments[1], "~");
    if (wave < strlen(cd.arguments[1]))
    {
        char *tmp = NULL;
        tmp = insert(home, wave, wave + 1, cd.arguments[1]);
        if (tmp == NULL)
        {
            fprintf(stderr, "Can't replace the symbol '~' into home directory\n");
        }
        else
        {
            free(cd.arguments[1]);
            cd.arguments[1] = tmp;
        }
    }
    if( stat(cd.arguments[1] , &st ) < 0 )
    {
        fprintf( stderr, "%s is not exist\n", cd.arguments[1] );
        return -1;
    }

    if (S_ISDIR(st.st_mode))
    {
        err =  chdir (cd.arguments[1]);
        if (err < 0)
        {
            fprintf(stderr, "Can't change directory to %s.\n", cd.arguments[1]);
            perror(NULL);
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Incorrect command: %s is not a directory!\n", cd.arguments[1]);
        return -1;
    }
    return 0;
}
/*
 * Print working directory
 */
int print_working_directory ()
{
    printf("%s\n", pwd);
    return 0;
}

/*
 * Msed
 */
int msed (program *pr)
{
    char *str = NULL, **tmp = NULL;
    char **text = NULL;
    int err = -2;
    int i, size = 0;
    if (pr->number_of_arguments < 3)
    {
        fprintf(stderr, "Not enough arguments!\n");
        fprintf(stderr, "Template and replacement were expected!\n");
        return -1;
    }
            
    pr->arguments[2] =replace_all(pr->arguments[2], "\\n", "\n");
    pr->arguments[2] =replace_all(pr->arguments[2], "\\t", "\t");
    pr->arguments[2] =replace_all(pr->arguments[2], "\\r", "\r");
    
    while (1)
    {
        str = read_str(stdin, &err);
        if (str == NULL)
        {
            break;
        }
        if (err == -1)
        {
            break;
        }
        if ((pr->arguments[1][0] == '^') && (pr->arguments[1][strlen(pr->arguments[1]) - 1] == '$'))
        {
            char *template = strcut(pr->arguments[1], 1, strlen(pr->arguments[1]) - 1);
            
            if (!strcmp(str, template))
            {
                free(str);
                str = strdup(pr->arguments[2]);
            }
            
            free(template);
        }
        else if (pr->arguments[1][0] == '^')
        {
            char *template = strcut(pr->arguments[1], 1, strlen(pr->arguments[1]));
            char *check = find_first(str, template);
            
            if (check != NULL)
            {
                char *temp = NULL;
                temp = insert(pr->arguments[2], 0, strlen(pr->arguments[2]), str);
                if (temp == NULL)
                {
                    fprintf(stderr, "Can't replace template\n");
                    return -1;
                }
                free(str);
                str = temp;
            }
            
            free(template);
        }
        else if (pr->arguments[1][strlen(pr->arguments[1]) - 1] == '$')
        {
            char *template = strcut(pr->arguments[1], 0, strlen(pr->arguments[1]) - 1);
            char *check = find_last(str, template);
            
            if (check != NULL)
            {
                char *temp = NULL;
                int start = strlen(str) - strlen(template);
                temp = insert(pr->arguments[2], start, start + strlen(pr->arguments[2]), str);
                if (temp == NULL)
                {
                    fprintf(stderr, "Can't replace template\n");
                    return -1;
                }
                free(str);
                str = temp;
                check = NULL;
            }
            free(template);
        }
        else 
        {
            str =replace_all(str, pr->arguments[1], pr->arguments[2]);
        }
        
        tmp = realloc(text, sizeof(char*) *(size + 1));
        if (tmp == NULL)
        {
            perror("Allocation memory error");
            free(text);
            return -1;
        }
        text = tmp;
        text[size++] = str;
    }
    
    for (i = 0; i < size; i++)
    {
        fprintf(stdout, "%s\n", text[i]);
        free(text[i]);
    }
    free(text);
    return 0;
}

int mgrep (program *pr)
{
    char *str = NULL, **tmp = NULL;
    char **text = NULL;
    int err = -2;
    int i, size = 0;
    
    if (pr->number_of_arguments < 2)
    {
        fprintf(stderr, "Not enough aguments: template was expected\n");
        return -1;
    }
    
    while (1)
    {
        str = read_str(stdin, &err);
        if (str == NULL)
        {
            break;
        }
        if (err == -1)
        {
            break;
        }
        if ((pr->arguments[1][0] == '^') && (pr->arguments[1][strlen(pr->arguments[1]) - 1] == '$'))
        {
            char *template = strcut(pr->arguments[1], 1, strlen(pr->arguments[1]) - 1);
            Automat *automat = compile (template);
            
            if (!search_all(str, automat, FULL))
            {
                free(str);
                free(template);
                continue;
            }
            
        }
        else if (pr->arguments[1][0] == '^')
        {
            char *template = strcut(pr->arguments[1], 1, strlen(pr->arguments[1]));
            Automat *automat = compile (template);
            
            if (!search_all(str, automat, START))
            {
                free(str);
                free(template);
                continue;
            }
            
            free(template);
        }
        else if (pr->arguments[1][strlen(pr->arguments[1]) - 1] == '$')
        {
            char *template = strcut(pr->arguments[1], 0, strlen(pr->arguments[1]) - 1);
            Automat *automat = compile (template);
            
            if (!search_at_the_end(str, automat))
            {
                free(str);
                free(template);
                continue;
            }
            free(template);
        }
        else 
        {
            Automat *automat = compile (pr->arguments[1]);
            
            if (!search_all(str, automat, ALL))
            {
                free(str);
                continue;
            }
        }
        
        tmp = realloc(text, sizeof(char*) *(size + 1));
        if (tmp == NULL)
        {
            perror("Allocation memory error");
            free(text);
            return -1;
        }
        text = tmp;
        text[size++] = str;
    }
    
    for (i = 0; i < size; i++)
    {
        fprintf(stdout, "%s\n", text[i]);
        free(text[i]);
    }
    free(text);
    return 0;
}
/*
 * mcat
 */
int mcat (program *pr)
{
    FILE* input;
    int ch = 0;
    /*
     * If no arguments - read from stdin
     */
    if (pr->number_of_arguments < 2)
    {
        input = stdin;
    }
    else
    {
        input = fopen(pr->arguments[1], "r");
        if (input == NULL)
        {
            return -1;
        }
    }
    do
    {
        ch = fgetc(input);
        if (ch == EOF)
            break;
        fprintf(stdout, "%c", (char)ch);
    }while (1);
    if (input != stdin)
        fclose(input);
    return 0;
}
/*
 * Export user variables to environ
 */
int export_vars(program *pr)
{
    int err;
    char *var = NULL;
    char *str = NULL;
    if (pr->number_of_arguments < 2)
    {
        int index = 0;
        while (environ[index])
        {
            printf("declare -x %s\n", environ[index]);
            ++index;
        }
        return 0;
    }
    var = strdup(pr->arguments[1]);
    
    str = find_var (var);
    if (str == NULL)
    {
        add_to_variables(var);
    }
    else
    {
        free(var);
        var = str;
    }
    err = putenv(var);
    if (err < 0)
    {
        perror("export: can't add variable");
        return -1;
    }
    return 0;
}
/*
 * Clean one job
 */
void clean_job (job job_to_clean)
{
    int j;
    if (job_to_clean.programs != NULL)
    {
        for (j = 0; j < job_to_clean.number_of_programs; j++)
        {
            int k;
            free(job_to_clean.programs[j].name);
            for (k = 0; k < job_to_clean.programs[j].number_of_arguments; k++)
            {
                free(job_to_clean.programs[j].arguments[k]);
            }
            if (job_to_clean.programs[j].input_file != NULL)
                free(job_to_clean.programs[j].input_file);
            if (job_to_clean.programs[j].output_file != NULL)
                free(job_to_clean.programs[j].output_file);
            free(job_to_clean.programs[j].arguments);
        }
        free(job_to_clean.command);
        free(job_to_clean.programs);
    }
}
/*
 * Clean massive of jobs
 */
void clean_jobs (job * jobs, int number_of_jobs)
{
    int i;
    if (jobs != NULL)
    {
        for (i = 0; i < number_of_jobs; i++)
        {
            clean_job(jobs[i]);
        }
        free(jobs);
    }
}

/*
 * Free everything before exit
 */
void close_all (int exit_status)
{
    if (variables != NULL)
    {
        int i;
        for (i = 0; i < number_of_variables; i++)
        {
            if (variables[i] != NULL)
                free(variables[i]);
        }
        free(variables);
    }
    free(shell);
    free(pwd);
    clean_jobs(jobs, number_of_jobs);
    clean_jobs(active_jobs, number_of_active);
    exit(exit_status);
}
