#include <stdio.h>
#include <stdlib.h>

#include "optimal.h"

int handle_add(struct optimal_command *command, int restc, char **restargc)
{
    return 0;
}

int handle_del(struct optimal_command *command, int restc, char **restargc)
{
    return 0;
}

int main(int argc, char **argv)
{
    struct optimal_command commands[] = {
        {
            .command_name = "add",
            .handler = handle_add,
            .options = {
                {
                    .long_name = "name",
                    .short_name = 'n',
                    .flags = OPTIMAL_MANDATORY,
                },
                NULL,
            },
        },
        {
            .command_name = "delete",
            .handler = handle_del,
            .options = {
                {
                    .long_name = "name",
                    .short_name = 'n',
                    .flags = OPTIMAL_MANDATORY,
                },
                NULL,
            },
        },
        NULL,
    };
}