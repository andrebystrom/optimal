#ifndef OPTIMAL_H
#define OPTIMAL_H

#include <stdbool.h>
#include <stdint.h>

#define OPTIMAL_MAX_OPTS 20
#define OPTIMAL_MAX_LONG_NAME 20
#define OPTIMAL_MAX_ARG 20
#define OPTIMAL_MAX_COMMAND_NAME 20
#define OPTIMAL_MAX_COMMANDS 20
#define OPTIMAL_MAX_STATIC_DATA (OPTIMAL_MAX_OPTS * 40)

struct optimal_bucket
{
    char *long_name;
    char *short_name;
    void *val;
};

struct optimal_param_table
{
    struct optimal_bucket buckets[OPTIMAL_MAX_OPTS * OPTIMAL_MAX_ARG];
};

enum optimal_qualifier
{
    OPTIMAL_OPTIONAL,
    OPTIMAL_REQUIRED
};

enum optimal_type
{
    OPTIMAL_INT,
    OPTIMAL_FLOAT,
    OPTIMAL_STRING,
    OPTIMAL_FLAG
};

struct optimal_arg
{
    char short_name[2];
    char long_name[OPTIMAL_MAX_LONG_NAME + 1];
    enum optimal_qualifier qualifier;
    enum optimal_type type;
};

struct optimal_command_builder
{
    int num_args;
    struct optimal_arg args[OPTIMAL_MAX_ARG];
    char command_name[OPTIMAL_MAX_COMMAND_NAME + 1];
    int (*handler)(struct optimal_param_table *, int, char **);

    struct optimal_command_builder *(*add_arg)(char, char *,
                                               enum optimal_qualifier,
                                               enum optimal_type);
    struct optimal_command_builder *(*add_flag)(char, char *,
                                                enum optimal_qualifier);
    struct optimal_command_builder *(*add_handler)(
        int (*handler)(struct optimal_param_table *, int, char **));
};

struct optimal_builder
{
    int num_commands;
    struct optimal_command_builder commands[OPTIMAL_MAX_COMMANDS];
    struct optimal_param_table param_table;

    struct optimal_command_builder *(*add_command)(char *command);
    int (*build)(int, char **);
};

struct optimal_builder *optimal_builder(void);

/* b->add_command("hej")
    ->add_arg('n', "name", OPTIMAL_MANDATORY, OPTIMAL_VAL_STR)
    ->add_arg('b', NULL, OPTIMAL_OPTIONAL, OPTIMAL_NO_VAL)
    ->add_handler(my_func);
b->build(argc, argv);
b->cleanup();

bool b = *(bool *)args->get("b"); */

#endif