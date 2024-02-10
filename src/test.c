#include <stdio.h>
#include <stdlib.h>

#include "optimal.h"

int handle_add(struct optimal_param_table *params, int restc, char **restargc)
{
    char *s = (char *)param_get(params, 'n', "name");
    printf("from add: %s\n", s);
    return 0;
}

int handle_del(struct optimal_param_table *params, int restc, char **restargc)
{
    return 0;
}

int main(int argc, char **argv)
{
    struct optimal_builder *app = optimal_builder();

    app->add_command("add")
        ->add_arg('n', "name", OPTIMAL_REQUIRED, OPTIMAL_STRING)
        ->add_flag('w', "why", OPTIMAL_OPTIONAL)
        ->add_handler(handle_add);

    app->add_command("del")
        ->add_arg('n', "name", OPTIMAL_REQUIRED, OPTIMAL_STRING)
        ->add_arg('w', "why", OPTIMAL_OPTIONAL, OPTIMAL_FLAG)
        ->add_handler(handle_del);
    return app->build(argc, argv);
}