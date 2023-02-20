#ifndef _ERROR_HANDLER_H__
#define _ERROR_HANDLER_H__

#include "misc.h" 

void _check_malloc(void *pointer, uint line, const char *file);
void _check_file(void *pointer, uint line, const char *file);

#endif // _ERROR_HANDLER_H__
