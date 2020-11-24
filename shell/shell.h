#ifndef SHELL_H_

#define SHELL_H_
#include <termios.h>


#define ALLOCATION_ERROR "Can't allocate memory of size: %lu\n"
#define STRCUT_ERROR "Error cutting string '%s'\n"
#define INCORRECT_COMMAND(A) fprintf(stderr, "\"%s\" is incorrect command\n", A)

#define START 1
#define FULL 2
#define ALL 3
#define AUTOMAT_SIZE 32

typedef struct
{
    int ch;
    int next1;
    int next2;
}Automat;

typedef struct program
{
    char *name;
    int number_of_arguments;
    pid_t pid;
    int status;
    int stopped;
    int completed;
    char** arguments;
    char *input_file, *output_file;
    int output_type; /* 1 - rewrite, 2 - append */
}program;

typedef struct job
{
    char *command;
    int background;
    pid_t pgid;
    program *programs;
    int number_of_programs;
}job;

char** arguments_of_shell;
char str_arguments_count_of_shell[6];
char* chld_status;
char* user;
char* home;
char* shell;
char str_uid[6];
char* pwd;
char str_pid_shell[6];

char **variables;
int number_of_variables;

job *jobs;
int number_of_jobs;
int number_of_active;
job *active_jobs;


pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;


int bg (program pr);
int fg (program pr);
int add_to_variables (char* string);
char* find_var (char* key);
int launch_job (job *j);
char *return_status (int wstatus);
int print_working_directory ();
void mark_job_as_running (job *j);
int add_job (job *j);
job* find_bg_or_stopped_job (int jid);
void format_job_info (job *j, const char *status);
int mark_process_status (pid_t pid, int status);
void update_status (void);
void remove_job (job j);
void do_job_notification (void);
job *find_job (pid_t pgid);
int job_is_stopped (job *j);
int job_is_completed (job *j);
void wait_for_job (job *j);
int foreground (job *j, int cont);
int background (job *j, int cont);
int redirect_stream (program pr);
int execute_embedded(program pr);
void list_jobs ();
int mcat (program *pr);
int msed (program *pr);
int mgrep (program *pr);
char *return_status (int wstatus);
int change_dir(program cd);
int export_vars(program *pr);
int is_embedded (program pr);
void clean_jobs (job * jobs, int number_of_jobs);
void clean_job (job job_to_clean);
void close_all (int exit_status);

char *replace_all (char*, char* , char*);
char *find_first (char* str, char* needle);
char *find_last (char* str, char* needle);
char *insert (char* str, int start, int end, char* command);
char *strcut (char *str, int begin, int end);
char *read_str (FILE *file, int *err);
job *commands_separator (char *commands, int *count);
program* conveyor_separator (char *command, int *count);
int arguments_separator (char *command, program *p_prog);

Automat *compile (char* pattern);
int search_all (char* str, Automat *automat, int mode);
int search_at_the_end (char* str, Automat *automat);

#endif
