#include "error_handler.h"
#include <stdio.h>
#include <stdlib.h>

enum error_codes{
    MALLOC_ERR = 4,
    FILE_ERR
};

void _check_malloc(void *pointer, uint line, const char *file)
{
    if (pointer == NULL)
    {
        fprintf(stderr, "Error at file %s:%u\nMalloc failed to allocate the needed memory.\n", file, line);
        exit(MALLOC_ERR);
    }   
}

void _check_file(void *pointer, uint line, const char *file)
{
    if (pointer == NULL)
    {
        fprintf(stderr, "Error at file %s:%u\nFile opening failed.\n", file, line);
        exit(FILE_ERR);
    }   
}
