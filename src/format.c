#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "format.h"

int parseVarFormat(format_t *format, char *formatstr, int maxstrsize)
{
    char intstr[6];
    char fmtlabel;
    int i=0, i_off=0;

    if ((formatstr == NULL) || (strlen(formatstr)==0)) return 0;

    memset(format, 0, sizeof(format_t));
    memset(intstr, 0, 6);

    // looking for alignment definition
    if (*formatstr == '<') format->align = A_LEFT;
    else
        if (*formatstr == '>') format->align = A_RIGHT;
        else
            if (*formatstr == '|') format->align = A_CENTER;

    if ((*formatstr == '<') || (*formatstr == '>') || (*formatstr == '|'))
    {
        fmtlabel = *formatstr;
        formatstr++;
        i++;
    }

    // increasing maxlen by multiple alignment definition
    while (*formatstr == fmtlabel)
    {
        i++;
        formatstr++;
    }

    if (strlen(formatstr) == 0) {
        format->maxlen = maxstrsize;
        return 1;
    }

    //looking for maxlen definition
    while((strlen(formatstr) > 0) && (isdigit(*formatstr)) && i_off < 5)
    {
        intstr[i_off] = *formatstr;
        i_off++;
        i++;
        formatstr++;
    }

    if (i)
        format->maxlen = atoi(intstr);
    else
        return 0;

    return 1;
}

char *varFormat(char **output, format_t *format, char *str)
{
    char *printf_param=NULL;

    nfree(*output);

    switch (format->align) {
    case A_LEFT: {
        xscatprintf(&printf_param, "%%-%us", format->maxlen);
        xscatprintf(output, printf_param, str);
        break;
    }
    case A_RIGHT: {
        xscatprintf(&printf_param, "%%%us", format->maxlen);
        xscatprintf(output, printf_param, str);
        break;
    }
    case A_CENTER: {
        if ((strlen(str)+1) < format->maxlen) {
            xscatprintf(&printf_param, "%%%uc",
                        ((format->maxlen - strlen(str))/2));
            xscatprintf(output, printf_param, ' ');
            format->maxlen -= ((format->maxlen - strlen(str))/2);
            nfree(printf_param);
        }
        xscatprintf(&printf_param, "%%-%us", format->maxlen);
        xscatprintf(output, printf_param, str);
        break;
    }
    }

    nfree(printf_param);

    return *output;
}

int parseStrFormat(format_t *format, char *formatstr)
{
    char intstr[6];
    int i=0, i_off=0;

    if ((formatstr == NULL) || (strlen(formatstr)==0)) return 0;

    memset(format, 0, sizeof(format_t));
    memset(intstr, 0, 6);

    //looking for maxlen definition
    while((strlen(formatstr) > 0) && (isdigit(*formatstr)) && i_off < 5)
    {
        intstr[i_off] = *formatstr;
        i_off++;
        i++;
        formatstr++;
    }

    if (i)
        format->maxlen = atoi(intstr);
    else
        return 0;

    if ((strlen(formatstr) > 2) && (*formatstr == '{')) {
        formatstr++;
        i_off=0;
        memset(intstr, 0, 6);
        while((strlen(formatstr) > 0) && (*formatstr != '}') && (i_off < 5)) {
            intstr[i_off] = *formatstr;
            formatstr++;
            i_off++;
        }
        format->pattern = sstrdup(intstr);
    } else
        format->pattern = sstrdup(" ");

    return 1;
}

char *strFormat(char **output, format_t *format, char *str)
{
    char *printf_param=NULL;
    char *patternpos = NULL;
    char *chunk[3];
    int chunkcount=0, chunkslen=0;
    int patlen=0, intpatlen=0;
    int i, k;

    if ((format->pattern == NULL) || (str == NULL) || (strlen(str)==0))
        return NULL;

    nfree(*output);

    // looking for patterns and fill up chunks.
    while((strlen(str) > 0) && (chunkcount <= 3) &&
          (patternpos=strstr(str, "{}")) != NULL)
    {
        chunkcount++;
        if (patternpos!=str)
        {
            chunk[chunkcount-1] = (char *) scalloc(patternpos - str, 1);
            strncpy(chunk[chunkcount-1], str, patternpos - str);
            chunk[chunkcount-1][patternpos - str] = '\0';
        } else
            chunk[chunkcount-1] = sstrdup("");
        str = patternpos + strlen("{}");
        chunkslen += strlen(chunk[chunkcount-1]);
    }
    chunkcount++;
    chunk[chunkcount-1] = sstrdup(str);
    chunkslen += strlen(chunk[chunkcount-1]);

    if (chunkcount > 1)
        patlen = (format->maxlen - chunkslen) / (chunkcount-1);
    else
        patlen = 0;

    for (i=0;i<chunkcount;i++)
    {
        nfree(printf_param);
        xscatprintf(&printf_param, "%%-%us", strlen(chunk[i]));
        xscatprintf(output, printf_param, chunk[i]); // drawing chunk
        if (i!=(chunkcount -1))
        {   // drawing pattern
            intpatlen = patlen / strlen(format->pattern);
            for (k=0; k<intpatlen; k++)
                xscatprintf(output, "%s", format->pattern);
            if (intpatlen < patlen)
                for (k=0;k<=(patlen-(intpatlen*strlen(format->pattern)));k++)
                    xscatprintf(output, "%c", format->pattern[k]);

        }
    }

    nfree(printf_param);

    return *output;
}
