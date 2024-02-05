#ifndef OPTIMAL_H
#define OPTIMAL_H

#include <stdbool.h>
#include <stdint.h>

#define OPTIMAL_MAX_OPTS 20

#define OPTIMAL_MANDATORY 1
#define OPTIMAL_HAS_VAL 2

struct optimal_option;
struct optimal_command;

typedef int (*optimal_command_handler)(struct optimal_command *command,
                                       int restc, char **restargv);

struct optimal_option
{
    char *long_name;   // Nullable
    char short_name;   // 0 to not use
    char *description; // Nullable
    int flags;
    char *value; // set by lib
};

struct optimal_command
{
    char *command_name;                              // Nullable.
    struct optimal_option options[OPTIMAL_MAX_OPTS]; // Null terminated
    optimal_command_handler handler;
};

int optimal_run(struct optimal_command *commands, int argc, char **argv);

#endif