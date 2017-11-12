#include "args.h"

#include <string.h>

static int lookup_name_index(int argc, char ** argv, char const *name)
{
    int i;

    for(i=1; i<argc; i++)
        if(strcmp(name, argv[i]) == 0)
            return i;
    return -1;
}

int parse_name(int argc, char ** argv, char const *name)
{
    return lookup_name_index(argc, argv, name) != -1;
}

char const * parse_str(int argc, char ** argv, char const *name, char const *def)
{
    int i;

    for(i=1; i<argc-1; i++)
        if(strcmp(name, argv[i]) == 0)
            return argv[i + 1];

    return def;
}

char const * parse_choices(int argc, char ** argv, char const *name, char const ** choices, char const *def)
{
    int index = lookup_name_index(argc, argv, name);
    if(index == -1 || index == argc-1)
        return def;

    for(; *choices != NULL; choices++)
        if(strcmp(argv[index + 1], *choices) == 0)
            return *choices;

    return def;
}
