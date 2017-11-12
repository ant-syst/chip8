#include "disassemble.h"
#include "tools.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
    unsigned int value;
    uint16_t it;
    char buffer[DISASS_BUFFER_LEN];

    if(argc != 2)
    {
        printf("Usage: %s <instruction>\n", argv[0]);
        goto error;
    }

    if(sscanf(argv[1], "%x", &value) != 1)
        THROW(error, 0, "Wrong instruction");

    it = value;

    disassemble(it, buffer);

    puts(buffer);

    return EXIT_SUCCESS;

    error:

    return EXIT_FAILURE;
}
