#ifndef _VARLIST_H
#define _VARLIST_H

typedef struct {
    char *label;
    void **value;
    int type;
    void *next;
} variable;

#endif
