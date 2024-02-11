#ifndef OPTIMAL_H
#define OPTIMAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>

#define OPTIMAL_MAX_OPTS 20
#define OPTIMAL_MAX_LONG_NAME 20
#define OPTIMAL_MAX_ARG 20
#define OPTIMAL_MAX_COMMAND_NAME 20
#define OPTIMAL_MAX_COMMANDS 20
#define OPTIMAL_MAX_DESCRIPTION 50
#define OPTIMAL_MAX_APP_NAME 20
#define OPTIMAL_MAX_STATIC_DATA (OPTIMAL_MAX_OPTS * 40)
#define OPTIMAL_PARAM_TABLE_SIZE (OPTIMAL_MAX_OPTS * OPTIMAL_MAX_ARG)

struct optimal_bucket
{
    char *long_name;
    char *short_name;
    void *val;
};

struct optimal_param_table
{
    struct optimal_bucket buckets[OPTIMAL_PARAM_TABLE_SIZE];
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
    char description[OPTIMAL_MAX_DESCRIPTION + 1];
    enum optimal_qualifier qualifier;
    enum optimal_type type;
};

struct optimal_command_builder
{
    int num_args;
    struct optimal_arg args[OPTIMAL_MAX_ARG];
    char command_name[OPTIMAL_MAX_COMMAND_NAME + 1];
    char description[OPTIMAL_MAX_DESCRIPTION + 1];
    int (*handler)(struct optimal_param_table *, int, char **);

    struct optimal_command_builder *(*add_command_description)(char *);
    struct optimal_command_builder *(*add_arg)(char, char *,
                                               enum optimal_qualifier,
                                               enum optimal_type);
    struct optimal_command_builder *(*add_arg_description)(char *s);
    struct optimal_command_builder *(*add_flag)(char, char *);
    struct optimal_command_builder *(*add_handler)(
        int (*handler)(struct optimal_param_table *, int, char **));
};

struct optimal_builder
{
    int num_commands;
    struct optimal_command_builder commands[OPTIMAL_MAX_COMMANDS];
    struct optimal_param_table param_table;
    char description[OPTIMAL_MAX_DESCRIPTION + 1];
    char app_name[OPTIMAL_MAX_APP_NAME + 1];

    struct optimal_command_builder *(*add_command)(char *command);
    int (*build)(int, char **);
};

/// @brief Returns the optimal_builder.
/// @param app_name The name of the application.
/// @param description The description of the application.
/// @return the optimal_builder.
struct optimal_builder *optimal_builder(char *app_name, char *description);

/// @brief Get an argument from the param table.
/// @param table the param table.
/// @param short_name the short name of the option.
/// @param long_name the long name of the option
/// @return the argument, or NULL if there is none. Note that a flag will never
/// be null. An option that is required will also never be null.
void *param_get(struct optimal_param_table *table,
                char short_name, char *long_name);

/// @brief Print the help text. Must be called after build.
/// @return -1.
int print_help(void);

#endif