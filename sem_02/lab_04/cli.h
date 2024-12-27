#ifndef _CLI_H_
#define _CLI_H_

#include <stdlib.h>
#include <wchar.h>

#define ERROR_NULL_POINTER_CLI         100
#define ERROR_NO_NULL_POINTER_CLI      101
#define ERROR_ALLOCATION_CLI           102
#define ERROR_INPUT_CLI                103
#define ERROR_EMPTY_STRING_CLI         104
#define ERROR_ENTRY_DUPLICATE_NAME_CLI 105
#define ERROR_INPUT_EOF_CLI            106
#define ERROR_EMPTY_LIST_CLI           107
#define ERROR_INCOMPLETE_ARG_CLI       108
#define ERROR_WRONG_ESCAPE_CLI         109

#define OPT_QUIT L"quit"
#define OPT_HELP L"help"

#define MENU_HEADER      L"Menu:\n"
#define HELP_DESCRIPTION L"Shows help message for a command.\n"         \
                         L"If command isn't specified, displays list\n" \
                         L"of all commands.\n"
#define NO_ENTRY_MESSAGE L"Option \"%S\" is incompatible\n"
#define QUIT_DESCRIPTION L"Quit"

typedef struct _cli cli_t;
typedef int (*cli_entry_t)(const size_t n, wchar_t **args, void *arg);
typedef void (*prompt_func_t)(void *arg);

cli_t *cli_init(wchar_t *header);

int cli_set_prompt(cli_t *const cli, prompt_func_t prompt, void *prompt_arg);

int cli_add_entry(cli_t *const cli, wchar_t *name, wchar_t *description, 
                  cli_entry_t entry, void *arg);

int cli_mainloop(const cli_t *const cli);

void cli_free(cli_t **const cli);

int read_line(wchar_t **const line, size_t *const size);

#endif

