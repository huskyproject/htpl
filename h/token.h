#ifndef _TOKEN_H
#define _TOKEN_H

typedef struct
{
    const char *label;
    int   id;
} token;

enum {
    ID_INCLUDE = 0,  // required token - used to include other configs.
    ID_SECTION,
    ID_ENDSECTION
};

extern token tokens[];

#endif
