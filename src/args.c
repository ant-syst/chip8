#include "args.h"

#include <string.h>

int parse_name(int argc, char ** argv, char const *name)
{
    int i;

    for(i=1; i<argc; i++)
        if(strcmp(name, argv[i]) == 0)
            return 0;

    return 1;
}

char const * parse_str(int argc, char ** argv, char const *name, char const *def)
{
    int i;

    for(i=1; i<argc-1; i++)
        if(strcmp(name, argv[i]) == 0)
            return argv[i + 1];

    return def;
}
