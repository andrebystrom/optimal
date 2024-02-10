#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include "optimal.h"

// forward declarations.
static int build(int argc, char **argv);
static struct optimal_command_builder *add_command(char *name);
static struct optimal_command_builder *add_command_description(char *desc);
static struct optimal_command_builder *add_flag(
    char short_name, char *long_name);
static struct optimal_command_builder *add_arg(
    char short_name, char *long_name,
    enum optimal_qualifier take_value,
    enum optimal_type value_type);
static struct optimal_command_builder *add_arg_description(char *desc);
static struct optimal_command_builder *add_handler(
    int (*handler)(struct optimal_param_table *, int, char **));

// static buffer for the builder.
static struct optimal_builder int_optimal_builder;
static struct optimal_builder *const p_builder = &int_optimal_builder;

// store optargs etc.
static uint64_t data_idx = 0;
static uint8_t data[OPTIMAL_MAX_STATIC_DATA];

// djb2
static uint64_t hash(char *short_name, char *long_name)
{
    uint64_t hash = 5381;
    char c;
    if (short_name)
        while ((c = *short_name++))
            hash = ((hash << 5) + hash) + c;
    if (long_name)
        while ((c = *long_name++))
            hash = ((hash << 5) + hash) + c;
    return hash;
}

// Try to allocate size bytes (aligned on 16 byte boundaries) and point dest to
// it. Returns 0 on success, -1 on failure.
static int allocate(void **dest, size_t size)
{
    const int boundary_mask = 0x0f;
    data_idx = (((uintptr_t)data + data_idx + 15) & (uintptr_t)~boundary_mask) -
               (uintptr_t)data;
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

    struct optimal_command_builder *empty_cmd = NULL;

    for (int i = 0; i < p_builder->num_commands; i++)
    {
        char *command_name = p_builder->commands[i].command_name;
        if (strcmp(firstarg, command_name) == 0)
            return p_builder->commands + i;
        if (strcmp("", command_name) == 0)
            empty_cmd = &p_builder->commands[i];
    }

    return empty_cmd;
}

static struct optimal_arg *find_arg_from_long_name(
    struct optimal_command_builder *command, char *long_name)
{
    for (int i = 0; i < command->num_args; i++)
    {
        if (strcmp(command->args[i].long_name, long_name) == 0)
            return command->args + i;
    }
    return NULL;
}

static int param_insert(char *short_name, char *long_name, void *val)
{
    uint64_t h = hash(short_name, long_name) % OPTIMAL_PARAM_TABLE_SIZE;
    struct optimal_param_table *table = &p_builder->param_table;
    struct optimal_bucket *base = table->buckets;
    struct optimal_bucket *curr_bucket = table->buckets + h;
    while (curr_bucket->long_name != NULL ||
           curr_bucket->short_name != NULL)
    {
        h = (h + 1) % OPTIMAL_PARAM_TABLE_SIZE;
        curr_bucket = base + h;
    }
    curr_bucket->short_name = short_name;
    curr_bucket->long_name = long_name;
    curr_bucket->val = val;

    return 0;
}

static struct optimal_arg *find_arg_from_short_name(
    struct optimal_command_builder *command, char short_name)
{
    for (int i = 0; i < command->num_args; i++)
    {
        if (command->args[i].short_name[0] == short_name)
            return command->args + i;
    }
    return NULL;
}

static int insert_flag(struct optimal_arg *arg)
{
    bool *val;
    if (allocate((void **)&val, sizeof(bool)) < 0)
        return -1;
    *val = true;
    if (param_insert(arg->short_name, arg->long_name, val) < 0)
        return -1;
    return 0;
}

static int insert_int(struct optimal_arg *arg, int val)
{
    int *ins;
    if (allocate((void **)&ins, sizeof(int)) < 0)
        return -1;
    *ins = val;
    if (param_insert(arg->short_name, arg->long_name, ins) < 0)
        return -1;
    return 0;
}

static int insert_float(struct optimal_arg *arg, float val)
{
    float *ins;
    if (allocate((void **)&ins, sizeof(float)) < 0)
        return -1;
    *ins = val;
    if (param_insert(arg->short_name, arg->long_name, ins) < 0)
        return -1;
    return 0;
}

static int insert_arg(struct optimal_arg *arg, char *str)
{
    int len = strlen(str);
    char *endptr;
    int int_val;
    float float_val;
    char *str_val;
    switch (arg->type)
    {
    case OPTIMAL_INT:
        int_val = strtol(str, &endptr, 10);
        errno = 0;
        if (errno != 0 || endptr - str != len + 1)
            return -1;
        return insert_int(arg, int_val);
    case OPTIMAL_FLOAT:
        errno = 0;
        float_val = strtof(str, &endptr);
        if (errno != 0 || endptr - str != len + 1)
            return -1;
        return insert_float(arg, float_val);
    case OPTIMAL_STRING:
        if (allocate((void **)&str_val, len + 1) < 0)
            return -1;
        strncpy(str_val, str, len);
        str_val[len] = '\0';
        return param_insert(arg->short_name, arg->long_name, str_val);
    default:
        // fall through
        break;
    }

    return -1;
}

static void shift_to_back(int argc, char **argv, int pos)
{
    char *tmp;
    for (int i = pos; i < argc - 1; i++)
    {
        tmp = argv[i];
        argv[i] = argv[i + 1];
        argv[i + 1] = tmp;
    }
}

// fills in the params_table with the supplied arguments.
// returns < 0 on error, otherwise the number of unconsumed args.
static int parse_opts(int argc, char **argv,
                      struct optimal_command_builder *command)
{
    int num_moved = 0;
    struct optimal_arg *arg = NULL;

    for (int i = 0; i < argc - num_moved; i++)
    {
        char *curr = argv[i];
        if (!arg && *curr == '-')
        {
            int is_long = curr[1] == '-';
            if (is_long)
            {
                struct optimal_arg *tmp;
                if (!(tmp = find_arg_from_long_name(command, curr + 2)))
                    return -1;
                if (tmp->type == OPTIMAL_FLAG)
                {
                    if (insert_flag(tmp) < 0)
                        return -1;
                }
                else
                {
                    arg = tmp;
                }
                continue;
            }
            char c;
            curr++;
            while ((c = *curr++))
            {
                struct optimal_arg *tmp;
                if (!(tmp = find_arg_from_short_name(command, c)))
                    return -1;
                if (tmp->type == OPTIMAL_FLAG)
                {
                    if (insert_flag(tmp) < 0)
                        return -1;
                }
                else if (*curr)
                {
                    int arg_start = curr - argv[i];
                    if (insert_arg(tmp, argv[i] + arg_start) < 0)
                        return -1;
                    break;
                }
                else
                {
                    arg = tmp;
                }
            }
        }
        else if (!arg)
        {
            shift_to_back(argc, argv, i);
            num_moved++;
            // --HACK-- decrement i, so we use the same index in the next
            // iteration
            i--;
        }
        else
        {
            if (insert_arg(arg, argv[i]) < 0)
                return -1;
            arg = NULL;
        }
    }

    return num_moved;
}

// Make sure required options are present, and set unset flags to false.
static int validate_options(struct optimal_command_builder *command)
{
    for (int i = 0; i < command->num_args; i++)
    {
        struct optimal_arg *arg = command->args + i;
        if (arg->qualifier != OPTIMAL_REQUIRED || arg->type != OPTIMAL_FLAG)
            continue;
        if (!param_get(&p_builder->param_table,
                       arg->short_name[0], arg->long_name))
        {
            if (arg->type != OPTIMAL_FLAG)
                return -1;
            bool *flag;
            if (allocate((void **)&flag, sizeof(bool)) < 0)
                return -1;
            *flag = false;
            if (param_insert(arg->short_name, arg->long_name, flag) < 0)
                return -1;
        }
    }
    return 0;
}

static int print_help(void)
{
    printf("%s\n", p_builder->description);
    struct optimal_command_builder *command;
    struct optimal_arg *arg;
    printf("commands:\n");
    for (int i = 0; i < p_builder->num_commands; i++)
    {
        command = p_builder->commands + i;
        printf("%s: %s\n", command->command_name, command->description);
        for (int j = 0; j < command->num_args; j++)
        {
            arg = command->args + j;
            printf("  -%s | --%s: %s\n", arg->short_name, arg->long_name,
                   arg->description);
        }
    }

    return -1;
}

// Does most of the work.
static int build(int argc, char **argv)
{
    struct optimal_command_builder *command;
    int restc = 0;
    char **restv = NULL;

    command = find_command(argc > 1 ? argv[1] : "");
    if (!command)
        return print_help();

    // we know the command, lets parse the options.
    memset(&p_builder->param_table, 0, sizeof(p_builder->param_table));
    if (command->command_name[0] == '\0')
        restc = parse_opts(argc - 1, argv + 1, command);
    else
        restc = parse_opts(argc - 2, argv + 2, command);

    if (restc < 0)
        return print_help();
    restv = argv + (argc - restc);

    // validate options.
    if (validate_options(command) < 0)
        return print_help();
    // options parsed and validated, run the handler!
    if (!command->handler)
        return print_help();
    return command->handler(&p_builder->param_table, restc, restv);
}

struct optimal_builder *optimal_builder(char *description)
{
    static int init = 0;
    if (!init)
    {
        int_optimal_builder.build = build;
        int_optimal_builder.add_command = add_command;
        int_optimal_builder.num_commands = 0;
        if (description)
        {
            strncpy(int_optimal_builder.description, description,
                    OPTIMAL_MAX_DESCRIPTION);
            int_optimal_builder.description[OPTIMAL_MAX_DESCRIPTION] = '\0';
        }
        else
        {
            int_optimal_builder.description[0] = '\0';
        }

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
    current->description[0] = '\0';

    current->num_args = 0;
    current->handler = NULL;

    current->add_arg = add_arg;
    current->add_flag = add_flag;
    current->add_handler = add_handler;
    current->add_command_description = add_command_description;
    current->add_arg_description = add_arg_description;

    return current;
}

static struct optimal_command_builder *add_command_description(char *desc)
{
    struct optimal_command_builder *current;
    current = p_builder->commands + p_builder->num_commands - 1;
    if (!desc)
        return current;
    strncpy(current->description, desc, OPTIMAL_MAX_DESCRIPTION);
    current->description[OPTIMAL_MAX_DESCRIPTION] = '\0';

    return current;
}

static struct optimal_command_builder *add_flag(
    char short_name, char *long_name)
{
    // qualifiers are ignored for flags, will be set to false if not present.
    return add_arg(short_name, long_name, OPTIMAL_REQUIRED, OPTIMAL_FLAG);
}

static struct optimal_command_builder *add_arg(
    char short_name, char *long_name,
    enum optimal_qualifier qualifier,
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

    current_arg->qualifier = qualifier;
    current_arg->type = value_type;

    current_arg->description[0] = '\0';

    return current;
}

static struct optimal_command_builder *add_arg_description(char *desc)
{
    struct optimal_command_builder *current;
    struct optimal_arg *current_arg;

    // add for current command and current arg.
    current = p_builder->commands + p_builder->num_commands - 1;
    current_arg = current->args + current->num_args - 1;

    if (!desc)
        return current;
    strncpy(current_arg->description, desc, OPTIMAL_MAX_DESCRIPTION);
    current_arg->description[OPTIMAL_MAX_DESCRIPTION] = '\0';

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

void *param_get(struct optimal_param_table *table,
                char short_name, char *long_name)
{
    char short_buf[] = {short_name, '\0'};
    uint64_t h = hash(short_buf, long_name) % OPTIMAL_PARAM_TABLE_SIZE;
    struct optimal_bucket *base = table->buckets;
    struct optimal_bucket *curr_bucket = base + h;

    while (curr_bucket->long_name != NULL || curr_bucket->short_name != NULL)
    {
        if (short_name && curr_bucket->short_name)
            if (curr_bucket->short_name[0] == short_name)
                return curr_bucket->val;

        if (long_name && curr_bucket->long_name)
            if (strcmp(curr_bucket->long_name, long_name) == 0)
                return curr_bucket->val;
        h = (h + 1) % OPTIMAL_PARAM_TABLE_SIZE;
        curr_bucket = base + h;
    }

    return NULL;
}