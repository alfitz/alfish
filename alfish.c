#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINE 80
#define READ_END 0
#define WRITE_END 1

typedef enum { 
    PIPE, 
    REDIR_STDOUT, 
    REDIR_STDERR, 
    REDIR_STDIN,
    REDIR_STDOUT_STDERR,
    APP_STDOUT, 
    APP_STDERR
} Redirect;

void print_help()
{
    printf("alfish: the Allison L. Fitzgerald shell\n"
    "><(((( >\n"
    "usages:\n"
    "\t * [command] [options] -- execute the command\n"
    "\t * [command] [options] & -- execute the command in background\n"
    "\t * [command1] [options] | [command2] [options] -- pipe command1 into command2\n"
    "\t * [command] [options] > [file] -- redirect stdout of command to file\n"
    "\t * [command] [options] 1> [file] -- redirect stdout of command to file\n"
    "\t * [command] [options] 2> [file] -- redirect stderr of command to file\n"
    "\t * [command] &> [file] -- redirect stdout and stderr to file\n"
    "\t * [command] < [file] -- redirect stdin of file to command\n"
    "\t * [command] >> [file] -- append stdout of command to file\n"
    "\t * [command] 2>> [file] -- append stderr of command to file\n");
            
}

Redirect get_redirect_type(char *arg)
{
    if (strcmp(arg, "|") == 0) {
        return PIPE;
    } else if (strcmp(arg, ">") == 0 || strcmp(arg, "1>") == 0) {
        return REDIR_STDOUT;
    } else if (strcmp(arg, "2>") == 0) {
        return REDIR_STDERR;
    } else if (strcmp(arg, "<") == 0) {
        return REDIR_STDIN;
    } else if (strcmp(arg, "&>") == 0) {
        return REDIR_STDOUT_STDERR;
    } else if (strcmp(arg, ">>") == 0) {
        return APP_STDOUT;
    } else if (strcmp(arg, "2>>") == 0) {
        return APP_STDERR;
    } else {
        fprintf(stderr, "REDIRECT OPTION NOT FOUND: %s\n", arg);
        exit(1);
    }
}

int find_redirect(char *arg)
{
    char *redirect_strings[8] = { "|", "<", ">", "&>", ">>", "1>", "2>", "2>>" };
    int i;

    for (i = 0; i < 8; i++) {
        if (strcmp(redirect_strings[i], arg) == 0) {
            return 1;
        }
    }

    return 0;
}

void execute_command(char *arg_array[], int arg_count)
{
    pid_t pid, sid;
    int bg_flag;

    if (strcmp(arg_array[arg_count - 1], "&") == 0) {
        bg_flag = 1;
        arg_array[arg_count - 1] = NULL;
    } else {
        bg_flag = 0;
    }

    pid = fork();

    if (pid < 0) { // fork failed
        fprintf(stderr, "Failed to fork process\n");
        exit(1);
    } else if (pid > 0) { // parent process
        if (bg_flag) {
            return;
        } else {
            wait(NULL);
        }
    } else { // child process
        if (bg_flag) {
            sid = setsid();
            close(fileno(stdout));
            execvp(arg_array[0], arg_array);
        } else {
            execvp(arg_array[0], arg_array);
        }
    }
}

void redirect_command(char *arg1[], char *arg2[], Redirect redirect_type)
{
    pid_t pid;
    int pipe_fd[2];
    int fd, direction;

    if (redirect_type == PIPE) {
        pipe(pipe_fd);
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Error forking the process\n");
            exit(1);
        } else if (pid > 0) { // parent
            dup2(pipe_fd[WRITE_END], 1);
            close(pipe_fd[READ_END]);
            execvp(arg1[0], arg1);
        } else { // child
            dup2(pipe_fd[READ_END], 0);
            close(pipe_fd[WRITE_END]);
            execvp(arg2[0], arg2);
        }
    } else {

        if (redirect_type == REDIR_STDOUT) {
            fd = open(arg2[0], O_WRONLY | O_CREAT | O_TRUNC, 0640);
            direction = fileno(stdout);
        } else if (redirect_type == REDIR_STDIN) {
            fd = open(arg2[0], O_RDONLY);
            direction = fileno(stdin);
        } else if (redirect_type == REDIR_STDERR || redirect_type == REDIR_STDOUT_STDERR) {
            fd = open(arg2[0], O_WRONLY | O_CREAT | O_TRUNC, 0640);
            direction = fileno(stderr);
        } else if (redirect_type == APP_STDOUT) {
            fd = open(arg2[0], O_WRONLY | O_CREAT | O_APPEND, 0640);
            direction = fileno(stdout);
        } else if (redirect_type == APP_STDERR) {
            fd = open(arg2[0], O_WRONLY | O_CREAT | O_APPEND, 0640);
            direction = fileno(stderr);
        }

        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Error forking the process\n");
            exit(1);
        } else if (pid > 0) { // parent
            wait(NULL);
        } else { // child
            dup2(fd, direction);
            if (redirect_type == REDIR_STDOUT_STDERR)
                dup2(fd, fileno(stdout));
            close(fd);
            execvp(arg1[0], arg1);
        }
    }
}

void copy_redir_arg(char *redir_arg[], char *arguments_array[], int lower, int upper)
{
    int i;
    int k = 0;

    for (i = lower; i < upper; i++, k++) {
        redir_arg[k] = malloc(MAX_LINE / 2 + 1);
        strcpy(redir_arg[k], arguments_array[i]);
    }
}

void free_redir_arg(char *redir_arg[], int count)
{
    int i;

    for (i = 0; i < count; i++) {
        free(redir_arg[i]);
    }
}

int main(int argc, char *argv[])
{
    char *arguments_array[MAX_LINE / 2 + 1];
    char *user_input = malloc(sizeof(char) * (MAX_LINE / 2 + 1));

    char *redir_arg1[MAX_LINE / 2 + 1];
    char *redir_arg2[MAX_LINE / 2 + 1];

    int run = 1;
    int arg_count, input_length, i, redir_flag;
    Redirect redirect;

    while (run) {
        printf("alfish> ");
        arg_count = 0;
        redir_flag = 0;
        fflush(stdout);

        if (fgets(user_input, MAX_LINE, stdin) != NULL) {

            // trim off the newline character
            input_length = strlen(user_input);
            user_input[input_length - 1] = '\0';

            if (strcmp(user_input, "help") == 0) {
                print_help();
            } else if (strcmp(user_input, "exit") == 0) {
                run = 0;
            } else {
                arguments_array[arg_count] = strtok(user_input, " ");

                while (arguments_array[arg_count] != NULL) {
                    arg_count++;
                    arguments_array[arg_count] = strtok(NULL, " ");
                }

                // redirect the command if applicable
                for (i = 0; i < arg_count; i++) {
                    redir_flag = find_redirect(arguments_array[i]);
                    if (redir_flag) {
                        copy_redir_arg(redir_arg1, arguments_array, 0, i);
                        copy_redir_arg(redir_arg2, arguments_array, i + 1, arg_count); 
                        redirect = get_redirect_type(arguments_array[i]);
                        break;
                    }
                }

                if (redir_flag) {
                    redirect_command(redir_arg1, redir_arg2, redirect);
                    free_redir_arg(redir_arg1, i);
                    free_redir_arg(redir_arg2, arg_count - i - 1);
                } else {
                    execute_command(arguments_array, arg_count);
                }
            }
        }
    }

    free(user_input);
    return 0;
}
