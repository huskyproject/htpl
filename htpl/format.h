#ifndef _FORMAT_H
#define _FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

enum { A_LEFT, A_RIGHT, A_CENTER };

typedef struct {
    int align;
    unsigned int maxlen;
    char *pattern;
} format_t;

int parseVarFormat(format_t *format, char *formatstr, int maxstrsize);
char *varFormat(char **output, format_t *format, char *str);

int parseStrFormat(format_t *format, char *formatstr);
char *strFormat(char **output, format_t *format, char *str);

#ifdef __cplusplus
}
#endif

#endif
