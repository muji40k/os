#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "cli.h"

#define PROMPT "$ "
#define START_STRING_SIZE 10
#define INCREMENT_STRING  2

#define HELP_TITLE "HELP FOR FUNCTION \"%s\"\n"
#define HELP_DESCRIPTION_TITLE "DESCRIPTION:\n"
#define HELP_NO_DESCRIPTION_MESSAGE "No description found\n"

typedef struct _cli_list cli_list_t;

struct _cli_list
{
    char *entry_name;
    char *description;
    cli_entry_t entry;
    void *arg;
    cli_list_t *next;
};

struct _cli
{
    char *header;

    prompt_func_t prompt;
    void *prompt_arg;

    size_t amount;
    cli_list_t *head;
};

static cli_list_t *cli_list_init(void);
static void cli_list_free(cli_list_t **const list);
static void cli_list_free_all(cli_list_t **const list);

static int entry_mainloop(const cli_list_t *const list, const size_t n, 
                          char **args);

static void help(const cli_t *const cli, const size_t n, char **args);
static void menu_print(const cli_t *const cli);
static void menu_print_certain(const cli_t *const cli, const char *const name);
static void menu_print_item(const char *const name, const char *const desc,
                            const size_t size, const char *const offset);
static size_t print_line(const char *const line);

static int read_line(char **const line, size_t *const size);
static int args_read(char ***const args, size_t *const amount);
static int args_parse(const size_t size, const char *const line, 
                      char ***const args, size_t *const amount);
static void args_free(char ***const args, size_t *const amount);

cli_t *cli_init(char *header)
{
    cli_t *new = malloc(sizeof(cli_t));

    if (new)
    {
        new->header = header;

        new->prompt = NULL;
        new->prompt_arg = NULL;

        new->amount = 0;
        new->head = NULL;
    }

    return new;
}

int cli_set_prompt(cli_t *const cli, prompt_func_t prompt, void *prompt_arg)
{
    if (!cli || !prompt)
        return ERROR_NULL_POINTER_CLI;

    cli->prompt = prompt;
    cli->prompt_arg = prompt_arg;

    return EXIT_SUCCESS;
}

int cli_add_entry(cli_t *const cli, char *name, char *description, 
                  cli_entry_t entry, void *arg)
{
    if (!cli || !name || !entry)
        return ERROR_NULL_POINTER_CLI;

    cli_list_t *new = cli_list_init();

    if (!new)
        return ERROR_ALLOCATION_CLI;

    int rc = EXIT_SUCCESS;
    new->entry = entry;
    new->entry_name = name;
    new->description = description;
    new->arg = arg;

    cli->amount++;

    if (cli->head)
    {
        cli_list_t *current = cli->head;

        for (; current->next && EXIT_SUCCESS == rc; current = current->next)
            if (!strcmp(current->entry_name, new->entry_name))
                rc = ERROR_ENTRY_DUPLICATE_NAME;

        if (EXIT_SUCCESS == rc)
            current->next = new;
    }
    else
        cli->head = new;

    if (EXIT_SUCCESS != rc)
        cli_list_free(&new);

    return rc;
}

int cli_mainloop(const cli_t *const cli)
{
    if (!cli)
        return ERROR_NULL_POINTER_CLI;

    if (cli->header)
        printf("%s", cli->header);

    printf(MENU_HEADER);
    menu_print(cli);

    int rc = EXIT_SUCCESS;
    bool run = true;
    char **args = NULL;
    size_t amount = 0;

    while (EXIT_SUCCESS == rc && run)
    {
        if (cli->prompt)
            cli->prompt(cli->prompt_arg);
        else
            printf(PROMPT);

        rc = args_read(&args, &amount);

        if (EXIT_SUCCESS == rc && args)
        {
            if (!strcmp(OPT_QUIT, args[0]))
                run = false;
            else if (!strcmp(OPT_HELP, args[0]))
                help(cli, amount - 1, (0 == amount - 1) ? NULL : args + 1);
            else
                rc = entry_mainloop(cli->head, amount, args);
        }
        else if (feof(stdin))
            rc = ERROR_EOF_CLI;
        else if (ERROR_EMPTY_STRING_CLI == rc)
            rc = EXIT_SUCCESS;

        args_free(&args, &amount);
    }

    if (ERROR_EOF_CLI == rc)
    {
        rc = EXIT_SUCCESS;
        printf("\n");
    }

    return rc;
}

void cli_free(cli_t **const cli)
{
    if (!cli || !(*cli))
        return;

    cli_list_free_all(&((*cli)->head));
    free(*cli);
    *cli = NULL;
}

static cli_list_t *cli_list_init(void)
{
    cli_list_t *new = malloc(sizeof(cli_list_t));

    if (new)
    {
        new->entry_name = NULL;
        new->entry = NULL;
        new->next = NULL;
    }

    return new;
}

static void cli_list_free(cli_list_t **const list)
{
    if (!list || !(*list))
        return;

    free(*list);
    *list = NULL;
}

static void cli_list_free_all(cli_list_t **const list)
{
    if (!list || !(*list))
        return;

    for (cli_list_t *current = *list, *tmp = NULL; current;)
    {
        tmp = current->next;
        cli_list_free(&current);
        current = tmp;
    }
}

static int entry_mainloop(const cli_list_t *const list, const size_t n, 
                          char **args)
{
    if (!list || !args || 0 == n)
        return ERROR_NULL_POINTER_CLI;

    int rc = EXIT_SUCCESS;
    const cli_list_t *current = list;

    for (; current && strcmp(args[0], current->entry_name); 
         current = current->next);

    if (!current)
        printf(NO_ENTRY_MESSAGE, args[0]);
    else
        rc = current->entry(n - 1, (0 == n - 1) ? NULL : args + 1, 
                            current->arg);

    return rc;
}

static void help(const cli_t *const cli, const size_t n, char **args)
{
    if (!cli)
        return;

    if (0 == n)
    {
        menu_print(cli);

        return;
    }

    for (size_t i = 0; n > i; i++)
        menu_print_certain(cli, args[i]);
}

static void menu_print(const cli_t *const cli)
{
    if (!cli)
        return;

    size_t command_size = 0;
    size_t current_size = strlen(OPT_QUIT);

    if (current_size > command_size)
        command_size = current_size;

    current_size = strlen(OPT_HELP);

    if (current_size > command_size)
        command_size = current_size;

    for (cli_list_t *current = cli->head; current; current = current->next)
    {
        current_size = strlen(current->entry_name);

        if (current_size > command_size)
            command_size = current_size;
    }

    char *offset = calloc(command_size + 1, sizeof(char));

    if (offset)
        for (size_t i = 0; command_size > i; offset[i++] = ' ');

    for (cli_list_t *current = cli->head; current; current = current->next)
        menu_print_item(current->entry_name, current->description, 
                        command_size, offset);

    menu_print_item(OPT_HELP, HELP_DESCRIPTION, command_size, offset);
    menu_print_item(OPT_QUIT, QUIT_DESCRIPTION, command_size, offset);

    free(offset);
}

static void menu_print_certain(const cli_t *const cli, const char *const name)
{
    if (!cli || !name)
        return;

    if (!strcmp(OPT_HELP, name))
    {
        printf(HELP_TITLE "\n" HELP_DESCRIPTION_TITLE, name);
        printf("%s\n", HELP_DESCRIPTION);

        return;
    }
    if (!strcmp(OPT_QUIT, name))
    {
        printf(HELP_TITLE "\n" HELP_DESCRIPTION_TITLE, name);
        printf("%s\n\n", QUIT_DESCRIPTION);

        return;
    }

    cli_list_t *current = cli->head;

    for (; current && strcmp(current->entry_name, name); 
           current = current->next);

    if (!current)
        printf(NO_ENTRY_MESSAGE, name);
    else
    {
        printf(HELP_TITLE "\n" HELP_DESCRIPTION_TITLE, name);

        if (current->description)
            printf("%s", current->description);
        else
            printf(HELP_NO_DESCRIPTION_MESSAGE);

        printf("\n");
    }
}

static void menu_print_item(const char *const name, const char *const desc,
                            const size_t size, const char *const offset)
{
    if (!name)
        return;

    if (!desc)
    {
        printf("%-*s |\n", (int)size, name);

        return;
    }

    if (!offset)
        printf("%-*s | %s\n", (int)size, name, desc);
    else
    {
        printf("%-*s | ", (int)size, name);
        size_t length = strlen(desc);
        size_t n = print_line(desc);

        while (length > n)
        {
            printf("%s | ", offset);
            n += print_line(desc + n);
        }

        if ('\n' == desc[length - 1])
            printf("%s | ", offset);

        printf("\n");
    }
}

static size_t print_line(const char *const line)
{
    if (!line)
        return 0;

    size_t n = 0;

    while ('\n' != line[n] && '\0' != line[n])
        fputc(line[n++], stdout);

    if ('\n' == line[n])
        fputc(line[n], stdout);

    return ++n;
}

static int read_line(char **const line, size_t *const size)
{
    if (!line || !size)
        return ERROR_NULL_POINTER_CLI;

    if (*line)
        return ERROR_NO_NULL_POINTER_CLI;

    size_t allocated_size = START_STRING_SIZE, cur_size = 0;
    char *tmp = NULL;
    int rc = EXIT_SUCCESS;
    *line = calloc(START_STRING_SIZE, sizeof(char));

    if (!line)
        rc = ERROR_ALLOCATION_CLI;

    for (bool read = true; EXIT_SUCCESS == rc && read;)
    {
        if (NULL == fgets(*line + cur_size, allocated_size - cur_size, stdin))
            rc = ERROR_INPUT_CLI;

        if (EXIT_SUCCESS == rc)
        {
            cur_size = strlen(*line);

            if ('\n' == (*line)[cur_size - 1])
            {
                (*line)[--cur_size] = '\0';
                read = false;
            }
            else
            {
                allocated_size *= INCREMENT_STRING;
                tmp = realloc(*line, allocated_size);

                if (!tmp)
                    rc = ERROR_ALLOCATION_CLI;
                else
                    *line = tmp;
            }
        }
    }

    if (EXIT_SUCCESS == rc && allocated_size > cur_size + 1)
    {
        allocated_size = cur_size + 1;
        tmp = realloc(*line, allocated_size);

        if (!tmp)
            rc = ERROR_ALLOCATION_CLI;
        else
            *line = tmp;
    }

    if (EXIT_SUCCESS != rc)
    {
        free(*line);
        allocated_size = 0;
        *line = NULL;
    }

    *size = allocated_size;

    return rc;
}

static int args_read(char ***const args, size_t *const amount)
{
    if (!args || !amount)
        return ERROR_NULL_POINTER_CLI;

    if (*args)
        return ERROR_NO_NULL_POINTER_CLI;

    size_t size = 0, cur_amount = 0;
    char *line = NULL;
    int rc = read_line(&line, &size);

    if (EXIT_SUCCESS == rc)
        rc = args_parse(size, line, args, &cur_amount);

    free(line);
    *amount = cur_amount;

    return rc;
}

static int args_parse(const size_t size, const char *const line, 
                      char ***const args, size_t *const amount)
{
    if (0 == size || !line || !amount)
        return ERROR_NULL_POINTER_CLI;

    if (*args)
        return ERROR_NO_NULL_POINTER_CLI;

    size_t counter = 0;
    bool is_word = false;

    for (size_t i = 0; size - 1 > i; i++)
    {
        if (isspace(line[i]) && is_word)
            is_word = false;
        else if (!isspace(line[i]) && !is_word)
        {
            is_word = true;
            counter++;
        }
    }

    if (0 == counter)
        return ERROR_EMPTY_STRING_CLI;

    *args = calloc(counter, sizeof(char *));
    int rc = EXIT_SUCCESS;

    if (!(*args))
        rc = ERROR_ALLOCATION_CLI;

    for (size_t i = 0, j = 0, k = 0; EXIT_SUCCESS == rc && size - 1 > i;)
    {
        if (isspace(line[i]))
        {
            i++;

            continue;
        }

        j = i + 1;

        for (; size - 1 > j && !isspace(line[j]); j++);

        (*args)[k] = calloc(j - i + 1, sizeof(char));

        if (!((*args)[k]))
            rc = ERROR_ALLOCATION_CLI;

        if (EXIT_SUCCESS == rc)
            for (size_t l = i; j > i; i++)
                (*args)[k][i - l] = line[i];

        k++;
    }

    if (EXIT_SUCCESS != rc)
        args_free(args, &counter);
    *amount = counter;

    return rc;
}

static void args_free(char ***const args, size_t *const amount)
{
    if (!args || !amount)
        return;

    for (size_t i = 0; *amount > i; i++)
        free((*args)[i]);

    free(*args);
    *args = NULL;
    *amount = 0;
}

