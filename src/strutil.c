#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "strutil.h"

char *htpl_trimLine(char *line)
{
    char *end_of_line;

    if (line==NULL)
        return NULL;

    while((*line == ' ' || *line == '\t') && (strlen(line) > 0)) // skip spaces/tabs at the beginning
        line++;

    end_of_line=line+strlen(line);
    while((*line == ' ' || *line == '\t') && (end_of_line > line)) // skip spaces/tabs at the ending
        end_of_line--;

    return line;
}
