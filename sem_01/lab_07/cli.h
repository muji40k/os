#ifndef _CLI_H_
#define _CLI_H_

#include <stdlib.h>

#define ERROR_NULL_POINTER_CLI     100
#define ERROR_NO_NULL_POINTER_CLI  101
#define ERROR_ALLOCATION_CLI       102
#define ERROR_INPUT_CLI            103
#define ERROR_EMPTY_STRING_CLI     104
#define ERROR_ENTRY_DUPLICATE_NAME 105
#define ERROR_EOF_CLI              106

#define OPT_QUIT "quit"
#define OPT_HELP "help"

#define MENU_HEADER      "Menu:\n"
#define HELP_DESCRIPTION "Shows help message for a command.\n"         \
                         "If command isn't specified, displays list\n" \
                         "of all commands.\n"
#define NO_ENTRY_MESSAGE "Option \"%s\" is incompatible\n"
#define QUIT_DESCRIPTION "Quit"

typedef struct _cli cli_t;
typedef int (*cli_entry_t)(const size_t n, char **args, void *arg);
typedef void (*prompt_func_t)(void *arg);

cli_t *cli_init(char *header);

int cli_set_prompt(cli_t *const cli, prompt_func_t prompt, void *prompt_arg);

int cli_add_entry(cli_t *const cli, char *name, char *description, 
                  cli_entry_t entry, void *arg);

int cli_mainloop(const cli_t *const cli);

void cli_free(cli_t **const cli);

#endif

