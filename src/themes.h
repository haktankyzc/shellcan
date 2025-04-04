//TODO:  Implement themes on shell

//* Themes are indicated with their byte codes that coloring the stdout

//printf("\033[38;2;%d;%d;%dm%s\033[0m\n", r, g, b, text);
//NOTE:  TERM_COLORS FOR STRING LITERALS
#define TERM_NRM "\x1B[0m"
#define TERM_RED(x) "\x1B[31m" x TERM_NRM
#define TERM_BLU(x) "\x1B[34m" x TERM_NRM
#define TERM_GRN(x) "\x1B[32m" x TERM_NRM
#define TERM_YEL(x) "\x1B[33m" x TERM_NRM
#define TERM_MAG(x) "\x1B[35m" x TERM_NRM
#define TERM_CYN(x) "\x1B[36m" x TERM_NRM
#define TERM_WHT(x) "\x1B[37m" x TERM_NRM

typedef struct {
    const char* PTH_COLOR;
    const char* ENTY_COLOR;
} Theme;

Theme DEFAULT = {
    .ENTY_COLOR = "\x1B[32m",
    .PTH_COLOR = "\x1B[34m"
};

Theme MONOKAI = {
    .ENTY_COLOR = "",
    .PTH_COLOR = ""
};

Theme MAGENTA = {
    .ENTY_COLOR = "",
    .PTH_COLOR = ""
};

extern char* PTH_COLOR;
extern char* ENTRY_COLOR;