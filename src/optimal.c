#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "optimal.h"

// forward declarations.
static int build(int argc, char **argv);
static struct optimal_command_builder *add_command(char *name);
static struct optimal_command_builder *add_arg(char short_name, char *long_name,
                                               enum optimal_value take_value,
                                               enum optimal_type value_type);

static struct optimal_command_builder *add_handler(
    int (*handler)(struct optimal_param_table *, int, char **));

// static buffer for the builder.
static struct optimal_builder int_optimal_builder;
static struct optimal_builder *const p_builder = &int_optimal_builder;

// store optargs etc.
static uint64_t data_idx = 0;
static uint8_t data[OPTIMAL_MAX_STATIC_DATA];

// Try to allocate size bytes (aligned on 16 byte boundaries) and point dest to
// it. Returns 0 on success, -1 on failure.
static int allocate(void **dest, size_t size)
{
    const int boundary_mask = 0x0f;
    uint64_t diff = ((uintptr_t)(data + data_idx)) & boundary_mask;
    data_idx += diff;
    data_idx = ((uintptr_t)data + data_idx + 15) & (uintptr_t)~boundary_mask;
    if (data_idx + size > sizeof(data))
        return -1;
    *dest = data + data_idx;
    data_idx += size;
    return 0;
}

static struct optimal_command_builder *find_command(char *firstarg)
{
    if (firstarg[0] == '-')
        firstarg = "";

    for (int i = 0; i < p_builder->num_commands; i++)
    {
        char *command_name = p_builder->commands[i].command_name;
        if (strcmp(firstarg, command_name) == 0)
        {
            return p_builder->commands + i;
        }
    }
    return NULL;
}

static int parse_opts(int argc, char **argv,
                      struct optimal_command_builder *command)
{
    for (int i = 0; i < argc; i++)
    {
        
    }
}

// Does most of the work.
static int build(int argc, char **argv)
{
    struct optimal_command_builder *command;
    int restc = 0;
    char **restv = NULL;

    command = find_command(argc > 1 ? argv[1] : "");
    if (!command)
        return -1; // print help?

    // we know the command, lets parse the options.
    parse_opts(argc - 2, argv + 2);

    // arguments parsed, run the handler!
    if (!command->handler)
        return -1;
    return command->handler(&p_builder->param_table, restc, restv);
}

struct optimal_builder *optimal_builder(void)
{
    static int init = 0;
    if (!init)
    {
        int_optimal_builder.build = build;
        int_optimal_builder.add_command = add_command;
        int_optimal_builder.num_commands = 0;

        init++;
    }
    return &int_optimal_builder;
}

static struct optimal_command_builder *add_command(char *name)
{
    struct optimal_command_builder *current;
    current = p_builder->commands + p_builder->num_commands++;

    if (!name)
        name = "";

    strncpy(current->command_name, name, OPTIMAL_MAX_COMMAND_NAME);
    current->command_name[OPTIMAL_MAX_COMMAND_NAME] = '\0';

    current->num_args = 0;
    current->handler = NULL;

    current->add_arg = add_arg;
    current->add_handler = add_handler;

    return current;
}

static struct optimal_command_builder *add_arg(char short_name, char *long_name,
                                               enum optimal_value take_value,
                                               enum optimal_type value_type)
{
    struct optimal_command_builder *current;
    struct optimal_arg *current_arg;

    current = p_builder->commands + p_builder->num_commands - 1;
    current_arg = current->args + current->num_args++;

    current_arg->short_name[0] = short_name;
    current_arg->short_name[1] = '\0';

    if (!long_name)
    {
        current_arg->long_name[0] = '\0';
    }
    else
    {
        strncpy(current_arg->long_name, long_name, OPTIMAL_MAX_ARG);
        current_arg->long_name[OPTIMAL_MAX_ARG] = '\0';
    }

    current_arg->take_value = take_value;
    current_arg->type = value_type;

    return current;
}

static struct optimal_command_builder *add_handler(
    int (*handler)(struct optimal_param_table *, int, char **))
{
    struct optimal_command_builder *current;

    current = p_builder->commands + p_builder->num_commands - 1;
    current->handler = handler;

    return current;
}