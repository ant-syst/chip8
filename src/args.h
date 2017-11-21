#ifndef H_ARGS
#define H_ARGS

int parse_name(int argc, char ** argv, char const *name);

char const * parse_str(int argc, char ** argv, char const *name, char const *def);

int parse_choices(int argc, char ** argv, char const *name, char const ** choices, char const *def,
                  char const ** res);

#endif
