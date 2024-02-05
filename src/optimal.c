#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "optimal.h"

int optimal_run(struct optimal_command *commands, int argc, char **argv)
{
    optimal_command_handler handler;
    struct optimal_command command;
    
    return handler(&command, argc, argv);
}