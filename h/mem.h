#ifndef _MEM_H
#define _MEM_H

#include <stdlib.h>

#define nfree(a) { if (a != NULL) { free(a); a = NULL; } }

void *smalloc(size_t size);
void *srealloc(void *ptr, size_t size);
void *scalloc(size_t nmemb, size_t size);
char *sstrdup(const char *src);

#endif
