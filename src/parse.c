#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "htpl.h"
#include "token.h"
#include "sections.h"
#include "varlist.h"
#include "format.h"

char htplError[HTPLERRORSIZE];

int isError(char *var, char *msg,...)
{
    va_list args;
    char *params;

    params=(char *)scalloc(HTPLERRORSIZE, 1);
    va_start(args, msg);
    vsprintf(params, msg, args);
    va_end(args);

    if (var == NULL) {
        sprintf(htplError, "%s", params);
        nfree(params);
        return 1;
    }
    nfree(params);
    return 0;
}

void makeErrorHeader(char *msg,...)
{
    char *s;
    va_list args;
    char *params;

    params=(char *)scalloc(HTPLERRORSIZE, 1);
    va_start(args, msg);
    vsprintf(params, msg, args);
    va_end(args);
    s = (char *) sstrdup(htplError);
    sprintf(htplError, "%s%s", params, s);
}

template *newTemplate()
{
    template *tpl = NULL;
    tpl = (template *) scalloc(sizeof(template), 1);
    return tpl;
}

int getValue(template *tpl, char *name, void **result)
{
    variable *v;
    nfree(*result);
/*
    if (!strcmp(name, "test")) {
        xstrcat((char **)result, name);
    } else if (!strcmp(name, "abc")) {
        xstrcat((char **)result, "abc123");
    } else if (!strcmp(name, "time")) {
        time_t tp=time(NULL);
        xstrcat((char **)result, (char *) htpl_trimLine(stripn(ctime(&tp))));
    } else
        return 0;

    return 1;
*/
    if ((v = findVariable(tpl, name)) == NULL) return 0;
    switch (v->type) {
        case T_STRING: xstrcat((char **)result, *((char **)v->value) ? *((char **)v->value) : ""); break;
        case T_INT: xscatprintf((char **)result, "%d", *((int *)v->value)); break;
    }
    return 1;
}

int expandMacro(template *tpl, char **macro)
{
    char *buf=NULL;
    char *label=NULL;
    char *fmt_ptr = NULL;
    char *format = NULL;
    char *lbl = NULL; // label of variable
    format_t f;

    if ((fmt_ptr = (char *) strchr(*macro, '%')) != NULL)
    {
        format = (char *) sstrdup(fmt_ptr+1);
        if (!parseVarFormat(&f, format, strlen(*macro)+2)) { // +2 because of 2 '@'
            sprintf(htplError, "\"%s\" : invalid format", format);
            nfree(*macro);
            *macro = sstrdup("");
            return 0;
        }
        lbl = (char *) scalloc(fmt_ptr - (*macro) + 1, 1);
        strncpy(lbl, *macro, (fmt_ptr - (*macro)));
    } else
        lbl = (char *) sstrdup(*macro);

    if (!getValue(tpl, lbl, (void **)&buf)) {
        sprintf(htplError, "\"%s\" is an unknown variable", lbl);
        nfree(lbl);
        nfree(*macro);
        *macro = sstrdup("");
        return 0;
    }
    nfree(lbl);
    if (fmt_ptr)
        varFormat(macro, &f, buf);
    else
        xstrcpy(macro, buf);
    return 1;
}

int htpl_parseLine(template *tpl, char *line, char **output)
{
    int i, escape=0;
    char *ptr;
    char *macStart=NULL;
    char *fmtStart=NULL;
    char *macro=NULL;
    char *strformat=NULL;
    format_t fmt;
    int format_flag=0;

    if ((i=strlen(line))==0) return -1;

    nfree(*output);
    *output = (char *)scalloc(i+1, 1);
    ptr = *output;

    while(*line) {
        if (*line == '\\') {
            escape = 1;
            line++;
        } else
        {
            if (*line == '@' &! escape &! fmtStart)
            {
                if (!macStart)
                    macStart = line;
                else
                {
                    macro = (char *) scalloc(line-macStart+1, 1);
                    strncpy(macro, macStart+1, line-macStart - 1);
                    if (!expandMacro(tpl, &macro)) {
                        nfree(macro);
                        return 0;
                    }
                    xstrcat(output, macro);
                    nfree(macro);
                    xstralloc(output, strlen(line)+1);
                    ptr = *output + strlen(*output);
                    macStart = 0;
                }
            } else
            if (*line == '%' &! escape &! macStart)
            {
                if (!fmtStart) fmtStart = line;
                else {
                    strformat = (char *) scalloc(line-fmtStart+1, 1);
                    strncpy(strformat, fmtStart+1, line-fmtStart - 1);
                    if (!parseStrFormat(&fmt, strformat)) {
                        nfree(strformat);
                        return 0;
                    }
                    nfree(strformat);
                    format_flag = 1;
                    ptr = *output + strlen(*output);
                    fmtStart = 0;
                }
            } else
            if (!macStart &! fmtStart){
                (unsigned char) ptr[0] = (unsigned char) line[0];
                ptr[1] = '\0';
                ptr++;
            }
            line++;
            escape = 0;
        }
    }
    *ptr = '\0';
    if (macStart) {
        sprintf(htplError, "Unbalanced @ operator");
        return 0;
    }
    if (fmtStart) {
        sprintf(htplError, "Unbalanced % operator");
        return 0;
    }

    if (format_flag) {
        strformat = (char *) sstrdup(*output);
        strFormat(output, &fmt, strformat);
        nfree(strformat);
    }
    if (!*output)
        *output = (char *) sstrdup("");

    return 1;
}

int htpl_readLine(template *tpl, char *file, char *line, int lineNo)
{
    if (*line == '#')
        return parseDirective(tpl, file, line+1);
    else if (tpl->currentSection)
        addLine(tpl->currentSection, line, lineNo);
    /* or quietly skip the line if out of section */

    return 1;
}

int parseTemplate(template *tpl, char *file)
{
    FILE *f;
    char *val=NULL;
    char *line=NULL;
    int lineNo=0;

    line = (char *) scalloc(MAXPATHLEN, 1);
    f = fopen(file, "rt");
    if (isError((void *)f, "Can't open file %s: %d", file, errno))
        return 0;

    while(fgets(line, MAXPATHLEN, f))
    {
        lineNo++;
        if (!htpl_readLine(tpl, file, line, lineNo))
        {
            makeErrorHeader("Error at %s:%d - ", file, lineNo);
            nfree(line);
            fclose(f);
            return 0;
        }
    }

    nfree(line);
    fclose(f);

    if (tpl->currentSection)  /* section was not closed by #endsection */
    {
        sprintf(htplError, "Section \"%s\" was not closed by #endsection directive till end of file %s",
            tpl->currentSection->name, file);
        makeErrorHeader("Error at %s:%d - ", file, lineNo);
        deleteSection(tpl->currentSection);
        return 0;
    }

    return 1;
}

int parseSection(template *tpl, char *name, char **output)
{
    char *buff=NULL;
    section *s;

    s = findSection(tpl, name);
    if(isError((void *)s, "section %s not found", name))
        return 0;
    if (!s->firstLine) return 0;

    s->line = s->firstLine;
    while(s->line->next) {
        if (htpl_parseLine(tpl, s->line->text, &buff))
            xstrcat(output, buff);
        else {
            makeErrorHeader("Error at %s:%d - ", s->file, s->line->lineNo);
            return 0;
        }
        s->line = s->line->next;
    }
    if (htpl_parseLine(tpl, s->line->text, &buff))
        xstrcat(output, buff);
    else
        return 0;

    return 1;
}
