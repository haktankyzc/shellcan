#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curses.h>
#include <pwd.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/ioctl.h>

#include "auto-comp.h"
#include "config.h"
#include "currentTime.h"
#include "themes.h"

#define SH_BUFSIZE 64
#define SH_DELIM " \t\r\n"
#define SH_ERR(err) fprintf(stderr, TERM_RED("\nSH_ERR: " err "\n"))

#define DEBUG_CONFIG 0

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
int sh_cd(char ** args);
int sh_help(char ** args); //Args is unused...
int sh_exit(char ** args); //Args is unused...
int sh_history(char **args); //Args is unused...

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
    SH_ERR("empty args err(from cd)");
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
  printf(TERM_YEL(" -----------------------*** SHELLCAN ***----------------------\n\n"));
  printf(TERM_BLU("'This is a basic shell written in pure C for educational purposes...'\n\n"));
  printf(TERM_MAG("Builtin Commands:\n"));

  for(int i = 0; i < num_builtins(); i++){
    printf(TERM_GRN(" * %s\n"),builtins_str[i]);
  }
  printf(TERM_MAG("\nKnown aliases from the config file:\n"));
  for (int i = 0; i < alias_count; i++) {
    printf(TERM_YEL("'%s' = '%s'\n"), aliases[i].name, aliases[i].command);
  }
  printf("\n");

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
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      SH_ERR(TERM_RED("Command not found !"));
      exit(EXIT_FAILURE);
    }
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return EXIT_SUCCESS;
}

int sh_launch(char **args){
  if (args[0] == NULL) {
    return EXIT_SUCCESS;
  }

  //*Check whether the command is a builtin one
  for (int i = 0; i < num_builtins(); i++) {
    if (strcmp(args[0], builtins_str[i]) == 0) {
      return (*builtins_func[i])(args);
    }
  }

  //printf("args[0}: '%s' \n",args[0]);
  //* Check whether the command is an alias from the config file
  char **solved = resolve_alias(args[0]);
  if (solved != NULL) {
#if DEBUG_CONFIG == 1
    for(int i = 0; i < 2 ; i++){
       printf("Resolved alias[%d] -> %s\n", i,solved[i]);
    }
#endif
    sh_launch(solved);
    free(solved);
    return EXIT_SUCCESS;
  }

  return sh_exec(args);
}

void sh_print_prompt(char *wdir) {

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int term_width = w.ws_col; 

  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);
  char time_str[13];
  strftime(time_str, sizeof(time_str), "at %H:%M:%S", tm_info); 
  //sh_color(MAG, time_str,time_str,sizeof(time_str));
  
  char prompt[256];
  snprintf(prompt, sizeof(prompt), "%s %s", wdir, TERM_GRN(">>>  "));

  int prompt_len = strlen(prompt);
  int time_len = strlen(time_str);
  int spaces = term_width - (prompt_len + time_len);

  if (spaces < 1) spaces = 1;

  printf("\n%s%*s%s\n", prompt, spaces, "", time_str);
}

void sh_loop() {
  char wdir[1024];
  char *buffer;
  char **args;
  int status = EXIT_SUCCESS;

  //AUTO COMPLETION FUNC
  rl_attempted_completion_function = custom_completion;

  do {
    // *SH_PRINT_PROMPT func
    if (getcwd(wdir, sizeof(wdir)) == NULL) {
      SH_ERR("getcwd failed");
    }
    sh_color(BLUE, wdir, wdir, sizeof(wdir));
    //printf("\n%s %s\n", wdir, TERM_GRN(">>>  "));
    sh_print_prompt(wdir);
    //////////*
    buffer = sh_get_line(wdir);
    args = sh_parse(buffer);
    status = sh_launch(args);

  } while (!status);
}

int main(void) {
  load_usr_conf_path();
  load_config();
  /*
  */
#if DEBUG_CONFIG == 1
  for (int i = 0; i < alias_count; i++) {
    printf("Alias: '%s' = '%s'\n", aliases[i].name, aliases[i].command);
  }
#endif
  //system("clear");
  
  sh_loop();
  return EXIT_SUCCESS;
}
