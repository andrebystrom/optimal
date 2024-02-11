# About
Allocation free and Fluent API for parsing and handling command line 
options/commands in the C programming language.

# Example

```C
#include <stdio.h>
#include <stdlib.h>

#include "optimal.h"
int handler(struct optimal_param_table *params, int restc, char **restv){
    printf("Hello %s\n", (char *)param_get(params, 'n', "name"));
    printf("flag was %s\n",
        *(bool *)param_get(params, 'f', "flag") ? "true" : "false");
    for (int i = 0; i < restc; i++) {
        printf("rest %d: %s\n", i + 1, restv[i]);
    }
}

int main(int argc, char **argv) {
    char *app_name = "demo";
    char *description = "print names";

    struct optimal_builder *app = optimal_builder(app_name, description);
    app->add_command(NULL)
        ->add_arg('n', "name", OPTIMAL_REQUIRED, OPTIMAL_STRING)
        ->add_arg_description("the name")
        ->add_flag('f', "flag")
        ->add_arg_description("the flag")
        ->add_handler(handler);
    
    app->add_command("some_command")
        ->add_command_description("some_command description")
        ->add_arg('n', "name", OPTIMAL_REQUIRED, OPTIMAL_STRING)
        ->add_arg_description("the name")
        ->add_flag('f', "flag")
        ->add_arg_description("the flag")
        ->add_handler(handler);

    return app->build(argc, argv);
}
```

```
> ./demo -njoe
Hello joe
flag was false
> ./demo --name -f joe aaa bbb
Hello joe
flag was true
rest 1: aaa
rest 2: bbb
> ./demo --help
demo: print names
usage: demo [<command>] <args>
  -n | --name:  the name
  -f | --flag:  the flag
commands:
some_command: some_command description
  -n | --name:  the name
  -f | --flag:  the flag
```

# API
```C
// get the app builder with the specified app name and description.
app = optimal_builder(char *app_name, char *description);

// Add a command to the app. If name is NULL or "", it is a "root" command,
// meaning that its options are parsed immediately after the application name.
// returns the optimal_command_builder.
command = app->add_command(char *name);

// add an option to the command.
// short_name is mandatory, long_name can be null.
// the qualifier can be either OPTIMAL_REQUIRED or OPTIMAL_OPTIONAL. If
// OPTIMAL_REQUIRED, optimal guarantees that param_get for the option won't
// return NULL in the handler.
// type can be OPTIMAL_*, where * can be INT, FLOAT and STR. If int,
// param_get for the option will return an int * if the option exists. Likewise
// for the other types.
command->add_arg(char short_name, char *long_name, qualifier, type);

// add a flag to the command. A flag will always be present in the param table.
// if the flag is specified in the options, it will be true, otherwise false.
command->add_flag(char short_name, char *long_name);

// Adds the handler as the function to run for the command. Will be invoked
// by optimal when encountering this command. The handler must return an int,
// and take a struct optimal_param_table *, int and char ** as arguments.
// The param table is where the arguments can be retrieved from. Int is the
// number of unparsed arguments, and char ** is the unparsed arguments.
command->add_handler(handler);
```
# Errors
Right now, the help menu is printed whenever an error is encountered, even
for internal errors, like failed allocations from the static buffer, or
overflows of the number of commands/arguments. On error, the build function will
return -1.