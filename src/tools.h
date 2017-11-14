#ifndef H_TOOLS
#define H_TOOLS

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define XSTR(a) STR(a)
#define STR(a) #a

#define THROW(LABEL, USE_PERROR, MSG, ...) do {                     \
    if(USE_PERROR)                                                  \
        fprintf(stderr, __FILE__ ":" XSTR(__LINE__) " %s " MSG "\n",\
                strerror(errno), ##__VA_ARGS__);                    \
    else                                                            \
        fprintf(stderr, __FILE__ ":" XSTR(__LINE__)" " MSG "\n",    \
                ##__VA_ARGS__);                                     \
    goto LABEL;                                                     \
} while(0)

#define UNUSED(a) a=a

#endif
