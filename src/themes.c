//TODO:  Implement themes on shell

//printf("\033[38;2;%d;%d;%dm%s\033[0m\n", r, g, b, text);

#include <stdio.h>

typedef struct {
    const char* PTH_COLOR;
    const char* ENTY_COLOR;
} Theme;

enum Themes {
    MONOKAI_DARKq
};
