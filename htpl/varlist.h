#ifndef _VARLIST_H
#define _VARLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

typedef struct {
    char *label;
    void **value;
    int type;
    void *next;
} variable;

#ifdef __cplusplus
}
#endif

#endif
