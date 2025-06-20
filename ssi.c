#define _DEFAULT_SOURCE // adding this cause gethostname() does not work on my machine otherwise
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <wait.h>
#include <signal.h> 

#define MAX_CMD_WORDS 1024
#define MAX_NUM_TOKENS 9
#define MAX_NUM_COMMANDS 4
#define PWD_SIZE 100
// store the history of the processes in a dynamic array
// add piping 
// perhaps add a help command

typedef struct background_process{
    pid_t pid;
    char cmd[MAX_CMD_WORDS];
    char pwd[PWD_SIZE];
    struct background_process *next;
    // what if I make a linked list since not limited, must make dynamic
} background_process;

int read_console(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], char token[MAX_NUM_COMMANDS][MAX_NUM_TOKENS][MAX_CMD_WORDS]);
void close_pipes(int lo_pipes[MAX_NUM_COMMANDS - 1][2], int commands_count);
void create_pipes(int lo_pipes[MAX_NUM_COMMANDS - 1][2], int commands_count);
void tokenize_spaces_console(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], char token[MAX_NUM_COMMANDS][MAX_NUM_TOKENS][MAX_CMD_WORDS], int commands_count, char *p);
bool check_if_changed_dir(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], char *pwd);
bool exit_program(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS]);
void init_background_node(pid_t pid, char *lo_command[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], background_process **head, char *new_pid);
bool new_background_process(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], background_process **head);
bool bglist_asked_for(char *lo_command[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], background_process *head);
void checking_background_processes(background_process **head);// make a func that frees the linked list

int main() {
    // init all the variables needed to execute the code
    //char *envp[] = { 0 };
    int status;
    signal(SIGINT, SIG_IGN); // init it so that cntrl+C does not work
    

    char hostname[100]; // perhaps make this dynamic later
    gethostname(hostname, sizeof(hostname));
    char *login = getlogin();
    char pwd[PWD_SIZE];
    getcwd(pwd, PWD_SIZE);
    //init the bg linked list
    background_process *head = NULL;
    char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS];
    char token[MAX_NUM_COMMANDS][MAX_NUM_TOKENS][MAX_CMD_WORDS];
    while (1) {
        checking_background_processes(&head);
        // infinite loop untill initialize exit of code through terminal
        char system_profile[256]; //change size maybe bash has a max size idk yet have to google it
        snprintf(system_profile, sizeof(system_profile), "%s@%s: %s > ", login, hostname, pwd);
        write(1, system_profile, strlen(system_profile));
        // check list of background processes here,
        int commands_count = read_console(lo_commands, token);
        if (commands_count == 0) continue;
        if (check_if_changed_dir(lo_commands, pwd)) continue;
        if (exit_program(lo_commands)) return 0; // do some other stuff like freeing freeing memory usage etc
        if (new_background_process(lo_commands, &head)) continue;
        checking_background_processes(&head);
        if (bglist_asked_for(lo_commands, head)) continue;
        int lo_pipes[MAX_NUM_COMMANDS - 1][2];
        create_pipes(lo_pipes, commands_count);
            
        pid_t last_pid = -1;
        int i = 0;
        for (; i < commands_count; i++) {
            pid_t pid;
            if ((pid = fork()) == 0) { 
                signal(SIGINT, SIG_DFL);
                if (i > 0) dup2(lo_pipes[i-1][0], 0); // redirects to the previous pipe if not the first command
                if (i < commands_count - 1) dup2(lo_pipes[i][1], 1); // redirects to current pipe to write
                if (commands_count > 1) close_pipes(lo_pipes, commands_count); // closing these pipes just in case

                execvp(lo_commands[i][0], lo_commands[i]);
                fprintf(stderr, "%s: No such file or directory\n", lo_commands[i][0]);
                exit(EXIT_FAILURE);
                    // impliment a exit command
            } else if (pid < 0) exit(EXIT_FAILURE);
            else last_pid = pid;
        }
        close_pipes(lo_pipes, commands_count);
        if (last_pid > 0) waitpid(last_pid, &status, 0);
    }
}
int read_console(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], char token[MAX_NUM_COMMANDS][MAX_NUM_TOKENS][MAX_CMD_WORDS]) {
    int commands_count = 0;
    char cmd[MAX_CMD_WORDS];
    if (fgets(cmd, sizeof(cmd), stdin) == NULL) return 0;
 
    cmd[strcspn(cmd, "\n")] = 0;
    char *save_p;
    char *p = strtok_r(cmd, "|", &save_p); //I've changed to strtok_r since I kept overriding the ptr
    while (p != NULL) {
        // adding a pipe of | tokenization
        while (*p == ' ') p++;
        if (*p == '\0') { // if all spaces, prevents seg fault
            p = strtok_r(NULL, "|", &save_p);
            continue;
        }
        char p_copy[MAX_CMD_WORDS];
        strncpy(p_copy, p, sizeof(p_copy) - 1);
        p_copy[sizeof(p_copy) - 1] = '\0';
        tokenize_spaces_console(lo_commands, token, commands_count, p_copy);
        commands_count++; 
        p = strtok_r(NULL, "|", &save_p); 
    } return commands_count;
}

void close_pipes(int lo_pipes[MAX_NUM_COMMANDS - 1][2], int commands_count) {
    int i = 0;
    for (; i < commands_count - 1; i++) { 
        close(lo_pipes[i][0]);
        close(lo_pipes[i][1]);
    } return;
}

void create_pipes(int lo_pipes[MAX_NUM_COMMANDS - 1][2], int commands_count) {
    int i = 0;
    for (; i < commands_count - 1; i++) {
        if(pipe(lo_pipes[i]) == -1) exit(EXIT_FAILURE);
    } return;
}

void tokenize_spaces_console(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], char token[MAX_NUM_COMMANDS][MAX_NUM_TOKENS][MAX_CMD_WORDS], int commands_count, char *p) {
    int num_tokens = 0;  
    // perhaps copy this to another string without refrenece      
    char *t = strtok(p, " ");
    while (t != NULL && num_tokens < MAX_NUM_TOKENS - 1) {
        strcpy(token[commands_count][num_tokens], t);
        lo_commands[commands_count][num_tokens] = token[commands_count][num_tokens];
        // the above: this part prevents the override of the token memory slot cause it takes from cmd, and it gets overrides, ahhh pointers... 
        num_tokens++;
        t = strtok(NULL, " ");
    } lo_commands[commands_count][num_tokens] = NULL;
}

bool check_if_changed_dir(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], char *pwd) {
    if (strcmp(lo_commands[0][0], "cd") == 0) {
        char *dir = lo_commands[0][1];
        if (dir == NULL || strcmp(dir, "~") == 0) dir = getenv("HOME");
        if (chdir(dir) != 0) printf("cd: %s: No such file or directory\n", dir);
        else getcwd(pwd, PWD_SIZE);
        return true;
    } return false;
}

bool exit_program(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS]) {
    if (strcmp(lo_commands[0][0], "exit") == 0) return true;
    return false;
}

void init_background_node(pid_t pid, char *lo_command[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], background_process **head, char *new_pwd) {
    background_process *new_background = malloc(sizeof(background_process));
    snprintf(new_background->pwd, PWD_SIZE, "%s", new_pwd);
    new_background->pid = pid;
    new_background->cmd[0] = '\0';
    new_background->next = NULL;
    int i = 1;
    char pwd[MAX_CMD_WORDS];
    getcwd(pwd, MAX_CMD_WORDS);
    // strncpy(new_background->cmd, pwd, MAX_CMD_WORDS);
    // strncat(new_background->cmd, "/", sizeof(new_background->cmd) - strlen(new_background->cmd) - 1);
    for (; lo_command[0][i] != NULL; i++) {
        strncat(new_background->cmd, lo_command[0][i], sizeof(new_background->cmd) - strlen(new_background->cmd) - 1); // places this right at the end where the command of the last pipe was
        strncat(new_background->cmd, " ", sizeof(new_background->cmd) - strlen(new_background->cmd) - 1); // adding a space the actual spaces, etc
    } if (*head == NULL) *head = new_background;
    else {
        background_process *curr = *head;
        while(curr->next != NULL) curr = curr->next;
        curr->next = new_background;
    }
}

bool new_background_process(char *lo_commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], background_process **head) {
    if (strcmp(lo_commands[0][0], "bg") != 0) return false;

    if (lo_commands[0][1] == NULL) {
        //printf("bash: bg: current: no such job\n");
        return false;
    }
    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null == -1) exit(EXIT_FAILURE);

    pid_t pid;

    int p[2];
    if (pipe(p) == -1) {
        exit(EXIT_FAILURE);
     }
    
    if ((pid = fork()) == 0) { 
        signal(SIGINT, SIG_DFL);
        close(p[0]);
        if (fcntl(p[1], F_SETFD, FD_CLOEXEC) == -1) exit(EXIT_FAILURE);
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null == -1) exit(EXIT_FAILURE);
        dup2(dev_null, STDOUT_FILENO);
        close(dev_null);
        //close(p[1]);
        execvp(lo_commands[0][1], &lo_commands[0][1]);
        fprintf(stderr, "%s: No such file or directory\n", lo_commands[0][1]);
        fflush(stderr);
        write(p[1], "f", 1); // this is a bit scuffed sorry, but it works
        close(p[1]);

        //fflush(stderr);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        //trying to get no gibberish input to goto node: TODO!
        close(p[1]);
        char buf;
        ssize_t n = read(p[0], &buf, 1);
        close(p[0]);
        char pwd[PWD_SIZE];
        getcwd(pwd, sizeof(pwd));
        if (n == 0) init_background_node(pid, lo_commands, head, pwd);
        else waitpid(pid, NULL, 0);
    } else {
        close(p[0]);
        close(p[1]);
        exit(EXIT_FAILURE);
        return false;
    }
    return true;
}

bool bglist_asked_for(char *lo_command[MAX_NUM_COMMANDS][MAX_NUM_TOKENS], background_process *head) {
    if (strcmp(lo_command[0][0], "bglist") != 0) return false;
    if (head == NULL) return true; // just in case lol
    int count = 0;
    background_process *curr = head;
    while(curr) {
        char pid_buffer[32];
        snprintf(pid_buffer, sizeof(pid_buffer), "%d: ", curr->pid);
        write(1, pid_buffer, strlen(pid_buffer));
        write(1, curr->pwd, strlen(curr->pwd));
        write(1, "/", 1);
        write(1, curr->cmd, strlen(curr->cmd));
        write(1, "\n", 1);
        // these multiple writes prevents my compiler to yell at me for a warning of buffer overflow
        curr = curr->next;
        count++;
    }
    char total[MAX_CMD_WORDS];
    snprintf(total, sizeof(total), "Total Background jobs: %d\n", count);
    write(1, total, strlen(total));
    return true;
}

void checking_background_processes(background_process **head) {
    background_process *curr = *head;
    background_process  *prev = NULL;
    // Note: WHOHANG returns if no status change, hue hue hue kkkkkkkkkkkkk
    while (curr != NULL) {
        int status;
        pid_t process = waitpid(curr->pid, &status, WNOHANG);
        // if (WEXITSTATUS(status) == 1) {
        //     if (prev == NULL) {
        //         *head = curr->next;
        //         free(curr);
        //         curr = *head;
        //     } else {
        //         prev->next = curr->next;
        //         free(curr);
        //         curr = prev->next;
        //     } continue;   
        // }
        if (process == curr->pid) {
            //char terminated_msg[MAX_CMD_WORDS + 64]; // change this later
            char pid_buffer[32];
            snprintf(pid_buffer, sizeof(pid_buffer), "%d: ", curr->pid);
            write(1, pid_buffer, strlen(pid_buffer));
            write(1, curr->pwd, strlen(curr->pwd));
            write(1, "/", 1);
            write(1, curr->cmd, strlen(curr->cmd));
            write(1, "has terminated.\n", 17); 
            //snprintf(terminated_msg, sizeof(terminated_msg), "%d: %s has terminated.\n", curr->pid, curr->cmd);
            //write(1, terminated_msg, strlen(terminated_msg));

            // now atlas we must free up that pesky space and refactor the linked list
            if (prev == NULL) {
                *head = curr->next;
                free(curr);
                curr = *head;
            } else {
                prev->next = curr->next;
                free(curr);
                curr = prev->next;
            }
        } else if (process == 0) {
            prev = curr;
            curr = curr->next;
        } else {
            if (prev == NULL) {
                *head = curr->next;
                free(curr);
                curr = *head;
            } else {
                prev->next = curr->next;
                free(curr);
                curr = prev->next;
            }
        }
    }
    
}

