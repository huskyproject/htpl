#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "strutil.h"
#include "htpl.h"
#include "token.h"
#include "sections.h"

extern char htplError[HTPLERRORSIZE];

static token tokens[] = {
    { "include", ID_INCLUDE },
    { "section", ID_SECTION },
    { "endsection", ID_ENDSECTION },
    { "if", ID_IF },
    { "elseif", ID_ELSEIF },
    { "else", ID_ELSE },
    { "endif", ID_ENDIF },
    { "ifdef", ID_IFDEF },
    { "ifndef", ID_IFNDEF },
    { NULL, -1 }
};


char *detectToken(char *line, char *token_label)
{
    while(isalnum(*line) && (strlen(line) > 0) && (strlen(token_label) < 256))
    {
        token_label[strlen(token_label)]=*line;
        line++;
    }
    return line;
}

int findTokenID(char *token_label)
{
    int i=0;

    if (token_label == NULL || strlen(token_label) == 0)
        return -1; // internal error

    while ((tokens[i].label != NULL) && (strcasecmp(tokens[i].label, token_label)))
        i++;

    if (tokens[i].label == NULL)
        return -2; // token not found

    return tokens[i].id;
}
