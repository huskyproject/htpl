#include <malloc.h>
#include <stdio.h>

#include "mem.h"

void *smalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "out of memory");
        abort();
    }
    return ptr;
}

void *srealloc(void *ptr, size_t size)
{
    void *newptr = realloc(ptr, size);
    if (newptr == NULL) {
        fprintf(stderr, "out of memory");
        abort();
    }
    return newptr;
}
void *scalloc(size_t nmemb, size_t size)
{
    void *ptr = smalloc(size*nmemb);
    memset(ptr,'\0',size*nmemb);
    return ptr;
}

char *sstrdup(const char *src)
{
    char *ptr;

    if (src == NULL) return NULL;
    ptr = (char *) strdup (src);
    if (ptr == NULL) {
        fprintf(stderr, "out of memory");
        abort();
    }
    return ptr;
}
