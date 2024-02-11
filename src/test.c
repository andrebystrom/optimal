#include <stdio.h>
#include <stdlib.h>

#include "optimal.h"

int handle_add(struct optimal_param_table *params, int restc, char **restv)
{
    char *s = (char *)param_get(params, 'n', "name");
    printf("from add: %s\n", s);
    for (int i = 0; i < restc; i++)
    {
        printf("rest %d: %s\n", i + 1, restv[i]);
    }
    return 0;
}

int handle_del(struct optimal_param_table *params, int restc, char **restv)
{
    char *s = (char *)param_get(params, 'n', "name");
    printf("from del: %s\n", s);
    printf("float: %f int: %d\n", *(float *)param_get(params, 'f', "float"),
           *(int *)param_get(params, 'i', "int"));
    for (int i = 0; i < restc; i++)
    {
        printf("rest %d: %s\n", i + 1, restv[i]);
    }
    return 0;
}

int handle_default(struct optimal_param_table *params, int restc, char **restv)
{
    (void)restc;
    (void)restv;
    printf("default! %s\n", (char *)param_get(params, 'h', "help"));
    return 0;
}

int main(int argc, char **argv)
{
    char *app_name = "test";
    char *desc = "add and delete things";
    struct optimal_builder *app = optimal_builder(app_name, desc);

    app->add_command(NULL)
        ->add_command_description("")
        ->add_arg('h', "help", OPTIMAL_OPTIONAL, OPTIMAL_STRING)
        ->add_handler(handle_default);

    app->add_command("add")
        ->add_command_description("add something")
        ->add_arg('n', "name", OPTIMAL_REQUIRED, OPTIMAL_STRING)
        ->add_arg_description("the name")
        ->add_flag('w', "why")
        ->add_arg_description("cool")
        ->add_handler(handle_add);

    app->add_command("del")
        ->add_arg('n', "name", OPTIMAL_REQUIRED, OPTIMAL_STRING)
        ->add_arg('w', "why", OPTIMAL_OPTIONAL, OPTIMAL_FLAG)
        ->add_arg('f', "float", OPTIMAL_REQUIRED, OPTIMAL_FLOAT)
        ->add_arg('i', "int", OPTIMAL_REQUIRED, OPTIMAL_INT)
        ->add_handler(handle_del);

    return app->build(argc, argv);
}