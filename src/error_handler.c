#include "error_handler.h"
#include <stdio.h>
#include <stdlib.h>

enum error_codes{
    MALLOC_ERR = 4
};

void _check_malloc(void *pointer, uint line, const char *file)
{
    if (pointer == NULL)
    {
        fprintf(stderr, "Error at file %s:%u\nMalloc failed to allocate the needed memory.\n", file, line);
        exit(MALLOC_ERR);
    }   
}
