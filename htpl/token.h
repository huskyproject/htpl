#ifndef _TOKEN_H
#define _TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

typedef struct
{
    const char *label;
    int   id;
} token;

enum {
    ID_INCLUDE = 0,  // required token - used to include other configs.
    ID_SECTION,
    ID_ENDSECTION,
    ID_IF,
    ID_ELSEIF,
    ID_ELSE,
    ID_ENDIF,
    ID_IFDEF,
    ID_IFNDEF
};

extern token tokens[];

#ifdef __cplusplus
}
#endif

#endif
