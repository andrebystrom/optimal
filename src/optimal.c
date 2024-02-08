#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "optimal.h"

static struct optimal_builder int_optimal_builder;
static struct optimal_builder *const p_builder = &int_optimal_builder;

struct optimal_builder *optimal_builder(void)
{
    int_optimal_builder.build = build;
    int_optimal_builder.add_command = add_command;
    int_optimal_builder.num_commands = 0;
    return &int_optimal_builder;
}

int build(void)
{
}

struct optimal_command_builder *add_command(char *name)
{
    struct optimal_command_builder *current;
    current = p_builder->commands + p_builder->num_commands++;
    strncpy(current->command_name, name, OPTIMAL_MAX_COMMAND_NAME);
    current->command_name[OPTIMAL_MAX_COMMAND_NAME] = '\0';
    current->num_args = 0;
    return current;
}

struct optimal_command_builder *add_arg(char short_name, char *long_name,
                                        enum optimal_value take_value,
                                        enum optimal_type value_type)
{
    struct optimal_command_builder *current;
    struct optimal_arg *current_arg;

    current = p_builder->commands + p_builder->num_commands - 1;
    current_arg = current->args + current->num_args++;

    current_arg->short_name[0] = short_name;

    current_arg->short_name[1] = '\0';
    strncpy(current_arg->long_name, long_name, OPTIMAL_MAX_ARG);
    current_arg->long_name[OPTIMAL_MAX_ARG] = '\0';
    
    current_arg->take_value = take_value;
    current_arg->type = value_type;

    return current;
}