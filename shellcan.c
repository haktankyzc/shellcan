//PERF: Added builtin commands
//TODO: Add Keyboard Shortcuts (pure C only)
//TODO: Add autocompletion

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <curses.h>
#include <pwd.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "auto-comp.h"

//NOTE:  TERM_COLORS FOR STRING LITERALS
#define TERM_NRM "\x1B[0m"
#define TERM_RED(x) "\x1B[31m" x TERM_NRM
#define TERM_BLU(x) "\x1B[34m" x TERM_NRM
#define TERM_GRN(x) "\x1B[32m" x TERM_NRM
#define TERM_YEL(x) "\x1B[33m" x TERM_NRM
#define TERM_MAG(x) "\x1B[35m" x TERM_NRM
#define TERM_CYN(x) "\x1B[36m" x TERM_NRM
#define TERM_WHT(x) "\x1B[37m" x TERM_NRM

#define SH_DELIM " \t\r\n"
#define SH_ERR(err) fprintf(stderr, TERM_RED("\nSHELLCAN_ERROR: " err "\n"))
#define SH_BUFSIZE 64

//NOTE: Color options for stdout
enum TERM_COLORS {
  NRM,
  RED,
  BLUE,
  GRN,
  YEL,
  MAG,
  CYN,
  WHT,
};

//NOTE:  Makes given buffer colored for stdout
void sh_color(enum TERM_COLORS term_color, char *text, char *output, int output_size) {

  const char *color;

  switch (term_color) {
    case NRM: color = "\x1B[0m"; break;
    case RED: color = "\x1B[31m";break;
    case BLUE:color = "\x1B[34m";break;
    case GRN: color = "\x1B[32m";break;
    case YEL: color = "\x1B[33m";break;
    case MAG: color = "\x1B[35m";break;
    case CYN: color = "\x1B[36m";break;
    case WHT: color = "\x1B[37m";break;
    default:
      fprintf(stderr, "Invalid color enumeration\n");
      exit(EXIT_FAILURE);
  }
  snprintf(output, output_size, "%s%s%s", color, text, TERM_NRM);
}

char *sh_get_usr_name();

// NOTE: Builtin Commands

int sh_help(char ** args);
int sh_cd(char ** args);
int sh_exit(char ** args);
int sh_history(char **args);

char *builtins_str[] = {
  "help",
  "cd",
  "exit",
  "history"
}; 

int (*builtins_func[]) (char **) = {
  &sh_help,
  &sh_cd,
  &sh_exit,
  &sh_history
};

int num_builtins(){
  return sizeof(builtins_str) / sizeof(char *);
}

int sh_cd(char **args){
  char usr_path[256] = "/home/";
  const char *usr_name = sh_get_usr_name();

  if(args[0] == NULL){
    SH_ERR("empty args (from cd)");
    return EXIT_FAILURE;
  }
  if(args[1] == NULL){
    strcat(usr_path, usr_name);
    chdir(usr_path); 
    return EXIT_SUCCESS;
  } 
  if(chdir(args[1]) != 0){
    return EXIT_SUCCESS;
  }
  
  return EXIT_SUCCESS;
}

int sh_help(char ** args){
  printf(TERM_YEL(" ---------------- SHELLCAN ---------------\n\n"));
  printf(TERM_GRN("This is a basic shell written in pure C for educational purposes...\n"));
  printf("Built -In Commands:\n");

  for(int i = 0; i < num_builtins(); i++){
    printf(" * %s\n",builtins_str[i]);
  }
  printf("\n\n");

  return EXIT_SUCCESS;
}

int sh_history(char **args) {
  printf("asğdpl");
    HIST_ENTRY **history_l = history_list();
    if (history_l) {
        for (int i = 0; history_l[i]; i++) {
          printf("%d: %s\n", i + history_base, history_l[i]->line);
        }
    }
  return EXIT_SUCCESS;
}

int sh_exit(char **args){
  exit(EXIT_SUCCESS);
}

/* NOTE: sh_loop funcs */

char *sh_get_usr_name(){
  struct passwd *pw;
  uid_t uid = geteuid();
  pw = getpwuid(uid);
  return pw->pw_name;
}

char *sh_get_line(char *wdir) {
  char *line = NULL;
  line = readline("");

  if (line == NULL) {
    SH_ERR("readline failed");  
    exit(EXIT_FAILURE);
  }
  return line;
}
/*
int main() {
    rl_attempted_completion_function = custom_completion; // Enable auto-completion

    char *input;
    while ((input = readline("mysh> ")) != NULL) {
        if (*input) add_history(input); // Save command history

        char **args = sh_parse(input);
        if (args[0] != NULL) {
            int executed = 0;
            for (int i = 0; i < num_builtins(); i++) {
                if (strcmp(args[0], builtins_str[i]) == 0) {
                    (*builtins_func[i])(args);
                    executed = 1;
                    break;
                }
            }
            if (!executed) {
                sh_exec(args);
            }
        }

        free(input);
        free(args);
    }

    return 0;
}

*/

char **sh_parse(char *buff) {
  int bufsize = SH_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    SH_ERR("Allocation error");
    exit(EXIT_FAILURE);
  }

  token = strtok(buff, SH_DELIM);
  while (token != NULL) {
    tokens[position] = strdup(token);
    position++;

    if (position >= bufsize) {
      bufsize += SH_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        SH_ERR("Allocation error");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, SH_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int sh_exec(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid < 0) {
    SH_ERR("Fork failed");
    return EXIT_FAILURE;
  }
  if (pid == 0) { //NOTE:CHILD MORUK
    if (execvp(args[0], args) == -1) {
      perror(TERM_RED("shellcan: "));
      exit(EXIT_FAILURE);
    }
  } else { //* NOTE: Parent MORUK
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return EXIT_SUCCESS;
}

int sh_launch(char **args)
{
  if (args[0] == NULL) {
    return EXIT_SUCCESS;
  }

  for (int i = 0; i < num_builtins(); i++) {
    if (strcmp(args[0], builtins_str[i]) == 0) {
      return (*builtins_func[i])(args);
    }
  }

  return sh_exec(args);
}

void sh_loop() {
  char wdir[1024];
  char *buffer;
  char **args;
  int status = EXIT_SUCCESS;

  rl_attempted_completion_function = custom_completion;
  
  do {
    //NOTE:  Get current working dir moruk
    if (getcwd(wdir, sizeof(wdir)) == NULL) {
      SH_ERR("getcwd failed");
    }
    
    sh_color(CYN, wdir, wdir, sizeof(wdir));
    printf("\n%s\n%s", wdir, TERM_BLU(">>>  "));
    buffer = sh_get_line(wdir);
    args = sh_parse(buffer);
    status = sh_launch(args);
    
    /*
    free(buffer);
    for (int i = 0; args[i] != NULL; i++) {
      free(args[i]);
    }
    free(args); 
    */

  } while (!status);
}

int main(void) {
  system("clear");
  sh_loop();
  return EXIT_SUCCESS;
}
