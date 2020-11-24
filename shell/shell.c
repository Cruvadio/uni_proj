/*
    errors    +
    interpretator    +
    redirect streams   +
    signals    +
    background +
    conveyor  +
    enviroments  +
    cd   +
    pwd  +
    exit +
    variables +
    export  +
    jobs  +
    fg  +
    bg  +
    mgrep +
    msed  +
    mcat  +
*/
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include "shell.h"


#define SETENV_ERR(A);\
    if (A == -1)\
    {\
        perror("setenv");\
    }
    
#define BRIGHT_GREEN fprintf(stderr, "\x1b[32;1m")
#define BRIGHT_LIGHT_BLUE fprintf(stderr,"\x1b[34;1m")
#define BRIGHT_MAGENTA fprintf(stderr,"\x1b[35;1m")
#define CLEAR_COLOR fprintf(stderr,"\x1b[0m")
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
 * Environ
 */
extern char **environ;

/*
 * Jobs
 */
extern job *jobs;
extern int number_of_jobs;
extern job *active_jobs;
extern int number_of_active;

pid_t shell_pgid;
struct termios shell_tmodes;
/*
 * Shell's input file
 */
int shell_terminal;
/*
 * Bolean for checking - if the streams were redirected
 */
int shell_is_interactive;

/*
 * Commands and user variables
 */
char *commands = NULL;
char **variables = NULL;
int number_of_variables = 0;

void init_shell ();
void prompt_to_enter ();
char *get_shell_name ();
void set_env (char** arguments, int count);
void chld_handler(int signum);
void exit_handler (int signum);

int main (int argc, char *argv[])
{
    /*
     *   Initialize shell
     */
    init_shell(); 
    
    while (1)
    {
        int i;
        
        /*
         *  Setting enviroments
         */
        set_env(argv, argc);
        
        /*
         * Promt to enter
         */
        if (commands == NULL)
            prompt_to_enter();
        if (commands == NULL)
        {
            fprintf (stderr, "Can't read command\n");
            continue;
        }
        /*
         *  Seperating command string into job structure
         */
        jobs = commands_separator(commands, &number_of_jobs);
        if (jobs == NULL)
        {
            /*
             * Error on separating jobs
             * or if the variables were inputed instead of command
             */
            commands = NULL;
            free(shell);
            free(pwd);
            continue;
        }
        
        /*
         * Launching jobs in cycle
         */
        for (i = 0; i < number_of_jobs; i++)
        {
            int err;
            err = launch_job(jobs + i);
        }
        /*
         * Cleaning jobs and some variables
         */
        clean_jobs(jobs, number_of_jobs);
        jobs = NULL;
        commands = NULL;
        free(shell);
        free(pwd);
    }
}
/*
 * Handler of child processes
 */

void chld_handler(int signum)
{
    int i;
    /*
     * Updating status of active jobs
     */
    update_status ();
    for (i = 0; i < number_of_active; i++)
    {
        /*
         * Check if active job is completed
         */
        if (job_is_completed (active_jobs + i)) 
        {
            /*
             * Removing only backgrond,fg jobs is removed in another place 
             */
            if (active_jobs[i].background)
            {
                format_job_info (active_jobs + i, "completed");
                remove_job(active_jobs[i]);
            }
            break;
        }
        else if (job_is_stopped (active_jobs + i)) 
        {
          /*
           * Checking if job is stopped
           */
            format_job_info (active_jobs + i, "stopped");
        }
    }
    signal(signum, chld_handler);
}


/*
 * Returning status in readable form
 */
char *return_status (int wstatus)
{
    if (WIFEXITED(wstatus))
        return "Normal terminating";
    if (WIFSIGNALED(wstatus))
        return "Terminated with signal";
    if(WIFSTOPPED(wstatus))
        return "Stopped";
    if (WIFCONTINUED(wstatus))
        return "Continued";
    return NULL;
}

void init_shell()
{
     /* 
      * See if we are running interactively.
      */ 
     
    shell_terminal = open("/dev/tty", O_RDONLY);
    shell_is_interactive = isatty (shell_terminal);
    
    if (shell_is_interactive)
    {
      /* 
       * Loop until we are in the foreground.
       */
        while (tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp ()))
            kill (- shell_pgid, SIGTTIN);

      /* 
       * Ignore interactive and job-control signals.
       */
        signal (SIGINT, SIG_IGN);
        signal (SIGQUIT, SIG_IGN);
        signal (SIGTSTP, SIG_IGN);
        signal (SIGTTIN, SIG_IGN);
        signal (SIGTTOU, SIG_IGN);
        signal(SIGUSR1, exit_handler);
        signal(SIGCHLD, chld_handler);

      /* 
       * Put ourselves in our own process group.
       */
        shell_pgid = getpid ();
        if (setpgid (shell_pgid, shell_pgid) < 0)
        {
            perror ("Couldn't put the shell in its own process group");
            exit (1);
        }

      /* 
       * Grab control of the terminal. 
       */
        tcsetpgrp (shell_terminal, shell_pgid);

      /* 
       * Save default terminal attributes for shell.
       */
        tcgetattr (shell_terminal, &shell_tmodes);
    }
}
/*
 * Promt to enter
 */
void prompt_to_enter ()
{
    int err_str;
    
    while (tcgetpgrp (shell_terminal) != shell_pgid)
            kill (- shell_pgid, SIGTTIN);
    BRIGHT_MAGENTA;
    fprintf(stderr, "[BigShell]");
    BRIGHT_GREEN;
    fprintf(stderr,"%s", user);
    CLEAR_COLOR;
    fprintf(stderr,":");
    BRIGHT_LIGHT_BLUE;
    fprintf(stderr,"%s", pwd);
    CLEAR_COLOR;
    fprintf(stderr,"$ ");
    commands = read_str(stdin, &err_str);
    if (err_str == -1)
    {
        fprintf(stderr, "\n");
        kill(getpid(), SIGUSR1);
    }
    
}
/*
 * Exit handler
 */
void exit_handler (int signum)
{
    int i;
    signal(signum, exit_handler);
    for (i = 0; i < number_of_active; i++)
        if (active_jobs[i].pgid != shell_pgid)
                kill(-active_jobs[i].pgid, SIGKILL);
    while (wait(NULL) != -1)
    {
        /*
         * Wait for processes
         */
    }
    close_all(EXIT_SUCCESS);
}

/*
 * Getting shell's name
 */
char *get_shell_name ()
{
    char *buff;
    ssize_t nbytes;
    /*
     * Malloc with maximum length of path's names
     */
    buff = malloc (PATH_MAX); 
    if (buff == NULL) 
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    /*
     * Reading self link
     */
    nbytes = readlink ("/proc/self/exe", buff, PATH_MAX);
    if (nbytes == -1)
    {
        perror("readlink");
        exit(EXIT_FAILURE);
    }
    /*
     * Adding zero byte
     */
    buff[nbytes] = '\0';
    return buff;
}
/*
 * Setting enviroments
 */
void set_env (char** arguments, int count)
{
    struct passwd *pass;
    int err;
    int i;
    char buf[2];
    pid_t pid_shell = getpid();
    uid_t uid;
    /*
     * Setting uid
     */
    uid = geteuid();
    /*
     * Setting HOME and USER
     */
    pass = getpwuid(uid);
    user = pass->pw_name;
    home = pass->pw_dir;
    /*
     * Setting PWD
     */
    pwd = getcwd(NULL, 0);
    /*
     * Setting shell name, UID, PID and arguments of shell
     */
    shell = get_shell_name();
    sprintf(str_uid, "%d",(short)uid);
    sprintf(str_pid_shell, "%d", (short)pid_shell);
    sprintf(str_arguments_count_of_shell, "%d", (short) count);
    arguments_of_shell = arguments;
    /*
     * Setting all arguments in environ
     */
    err = setenv("USER", user, 1);
    SETENV_ERR (err);
    err = setenv("HOME", home, 1);
    SETENV_ERR (err);
    err = setenv("PWD", pwd, 1);
    SETENV_ERR (err);
    err = setenv("SHELL", shell, 1);
    SETENV_ERR (err);
    err = setenv ("PID", str_pid_shell, 1);
    SETENV_ERR (err);
    err = setenv ("UID", str_uid, 1);
    SETENV_ERR (err);
    if (chld_status != NULL)
    {
        err = setenv ("?", chld_status, 1);
        SETENV_ERR (err);
    }
    err = setenv("#", str_arguments_count_of_shell, 1);
    SETENV_ERR (err);
    
    for (i = 0; i < count; i++)
    {
        sprintf(buf,"%d", i);
        setenv(buf, arguments[i], 1);
        SETENV_ERR (err);
    }
}
