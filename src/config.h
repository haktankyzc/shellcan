/*
Config file syntax:

alias xy = "cd .."
theme robbyrussel

*/

#ifndef CONFIG_H
#define CONFIG_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CONF_LINE_LN 100
#define MAX_ALIAS_LN 10
#define MAX_CMD_LN 90
#define MAX_ALIASES 100

#define ALIAS_BUFSIZE 64
#define ALIAS_DELIM " \t\r\n"

bool USR_CONFIG_ENABLED = false;

typedef struct
{
  char name[MAX_ALIAS_LN];
  char command[MAX_CMD_LN];
} Alias;

Alias aliases[MAX_ALIASES];
int alias_count = 0;

const char *conf_keywords[] = {
    "alias",
    "theme",
};

// TODO: Iterate between possible configuration paths
char *conf_paths[] = {"shcanrc", NULL};

void add_alias(const char *name, const char *command)
{
  assert(strlen(name) < MAX_ALIAS_LN);
  assert(strlen(command) < MAX_CMD_LN);

  if (alias_count >= MAX_ALIASES)
  {
    fprintf(stderr, "ERROR: Too many aliases!\n");
    exit(EXIT_FAILURE);
  }
  strcpy(aliases[alias_count].name, name);
  strcpy(aliases[alias_count].command, command);
  alias_count++;
}

char **split_char2args(const char *input)
{
  if (!input)
    return NULL;

  char *input_copy = strdup(input);
  if (!input_copy)
    return NULL;

  char **args = NULL;
  char *token;
  char *saveptr;
  int size = 0;

  token = strtok_r(input_copy, " ", &saveptr);
  while (token)
  {
    char **temp = realloc(args, (size + 1) * sizeof(char *));
    if (!temp)
    {
      free(args);
      free(input_copy);
      return NULL;
    }
    args = temp;
    args[size++] = strdup(token);
    token = strtok_r(NULL, " ", &saveptr);
  }

  char **temp = realloc(args, (size + 1) * sizeof(char *));
  if (!temp)
  {
    free(args);
    free(input_copy);
    return NULL;
  }
  args = temp;
  args[size] = NULL;
  //*count = size;

  free(input_copy);
  return args;
}

char **resolve_alias(char *name)
{
  for (int i = 0; i < alias_count; i++)
  {
    if (strcmp(aliases[i].name, name) == 0)
    {
      //printf("alias known as : %s\n", aliases[i].command);
      char **args = split_char2args(aliases[i].command);
      return args;
    }
  }
  return NULL;
}

int load_config()
{
  /*
  if (conf_paths[1] == NULL) {
    fprintf(stderr, "ERROR: USR_Config path is not loaded \n");
    return EXIT_FAILURE;
  }
  */
  FILE *file;

  // TODO: Iterate between possible configuration paths
  file = fopen(conf_paths[1], "r");
  if (file == NULL)
  {
    fprintf(stderr, "ERROR: Could not open config file -> %s\n", conf_paths[1]);
    return EXIT_FAILURE;
  }

  char line[MAX_CONF_LINE_LN];
  while (fgets(line, sizeof(line), file))
  {

    line[strcspn(line, "\n")] = '\0';
    char *trimmed_line = line;

    // Comment line 
    if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#')
    {
      continue;
    }

    if (strncmp(trimmed_line, "alias", 5) == 0)
    {
      char *name_start = trimmed_line + 6;
      while (*name_start == ' ')
        name_start++;

      char *equals_pos = strchr(name_start, '=');
      if (equals_pos != NULL)
      {
        char *name_end = equals_pos - 1;
        while (*name_end == ' ' &&
               name_end >= name_start)
        {
          *name_end = '\0';
          name_end--;
        }

        *equals_pos = '\0';

        char *command_start = equals_pos + 1;
        while (*command_start == ' ' || *command_start == '"')
          command_start++;

        char *command_end = command_start + strlen(command_start) - 1;
        while ((*command_end == ' ' || *command_end == '"') &&
               command_end >= command_start)
        {
          *command_end = '\0';
          command_end--;
        }

        add_alias(name_start, command_start);
      }
    }
    if (strncmp(trimmed_line, "theme", 5) == 0)
    {
      continue;
    }
  }

  fclose(file);
  return EXIT_SUCCESS;
}

void load_usr_conf_path()
{
  char *usr_name = getenv("USER");
  char path[256];

  snprintf(path, sizeof(path), "/home/%s/.config/shellcan/shcanrc", usr_name);
  conf_paths[1] = strdup(path);
}

int main2()
{
  load_usr_conf_path();

  printf("CONF_PATH: %s\n", conf_paths[1]);
  load_config();

  /*
   */
  for (int i = 0; i < alias_count; i++)
  {
    printf("Alias: '%s' = '%s'\n", aliases[i].name, aliases[i].command);
  }
  resolve_alias("..");
  free(conf_paths[1]);
  return EXIT_SUCCESS;
}

#endif
