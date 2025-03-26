#ifndef AUTO_COMP
#define AUTO_COMP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#define MAX_COMMANDS 4096

char *command_paths[] = {"/bin", "/usr/bin", "/usr/local/bin", NULL};
//TODO: void append_PATH();

char *git_args[] = {"checkout", "commit", "push", "pull", "status", "clone", NULL};

char **get_system_commands() {
    char **commands = malloc(MAX_COMMANDS * sizeof(char *));
    if (!commands) {
        fprintf(stderr,"malloc");
        exit(EXIT_FAILURE);
    }
    int index = 0;

    for (int i = 0; command_paths[i] != NULL; i++) {
        DIR *dir = opendir(command_paths[i]);
        if (!dir) continue;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (index < MAX_COMMANDS - 1) {
                commands[index++] = strdup(entry->d_name);
            }
        }
        closedir(dir);
    }
    commands[index] = NULL;
    return commands;
}

char *command_generator(const char *text, int state) {
    static int index;
    static char **commands = NULL;

    if (state == 0) {
        if (commands) {
            for (int i = 0; commands[i]; i++)
                free(commands[i]);
            free(commands);
        }
        commands = get_system_commands();
        index = 0;
    }

    while (commands[index]) {
        if (strncmp(commands[index], text, strlen(text)) == 0) {
            return strdup(commands[index++]);
        }
        index++;
    }
    return NULL;
}

char *file_generator(const char *text, int state) {
    static int index;
    static struct dirent **namelist = NULL;
    static int file_count = 0;

    if (state == 0) {
        if (namelist) { // Önceki belleği temizle
            for (int i = 0; i < file_count; i++)
                free(namelist[i]);
            free(namelist);
        }
        index = 0;
        file_count = scandir(".", &namelist, NULL, alphasort);
    }

    while (index < file_count) {
        struct dirent *entry = namelist[index++];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue; // "." ve ".." dizinlerini atla

        if (strncmp(entry->d_name, text, strlen(text)) == 0) {
            struct stat path_stat;
            stat(entry->d_name, &path_stat);

            // Eğer bir dizinse, sonuna "/" ekleyelim
            if (S_ISDIR(path_stat.st_mode)) {
                char *dir_name = malloc(strlen(entry->d_name) + 2);
                sprintf(dir_name, "%s/", entry->d_name);
                return dir_name;
            } else {
                return strdup(entry->d_name);
            }
        }
    }
    return NULL;
}

char *git_arg_generator(const char *text, int state) {
    static int index;
    if (state == 0) index = 0;

    while (git_args[index]) {
        if (strncmp(git_args[index], text, strlen(text)) == 0) {
            return strdup(git_args[index++]);
        }
        index++;
    }
    return NULL;
}

char **custom_completion(const char *text, int start, int end) {
    if (start == 4 && !strncmp(rl_line_buffer, "git ", 4)) {
        return rl_completion_matches(text, git_arg_generator);
    } else if (start > 0) {
        return rl_completion_matches(text, file_generator);
    }
    return rl_completion_matches(text, command_generator);
}


/*
 
int main() {
    rl_attempted_completion_function = custom_completion;
    char *input;

    while ((input = readline("mysh> ")) != NULL) {
        if (*input) add_history(input);
        printf("Executed: %s\n", input);
        free(input);
    }
    return 0;
}

 */

#endif //AUTO_COMP 
