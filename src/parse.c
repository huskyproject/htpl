#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "htpl.h"
#include "token.h"
#include "sections.h"
#include "parse.h"
#include "varlist.h"
#include "format.h"
#include "strutil.h"

char htplError[HTPLERRORSIZE];

void makeErrorHeader(char *msg,...)
{
    char *s;
    va_list args;
    char *params;

    params=(char *)scalloc(HTPLERRORSIZE, 1);
    va_start(args, msg);
    vsprintf(params, msg, args);
    va_end(args);
    s = sstrdup(htplError);
    sprintf(htplError, "%s%s", params, s);
}

template *newTemplate()
{
    template *tpl = NULL;
    tpl = (template *) scalloc(sizeof(template), 1);
    tpl->currentSection = NULL;
    tpl->firstSection = NULL;
    tpl->firstVariable = NULL;
    return tpl;
}

void getValue(template *tpl, char *name, char **result)
{
    variable *v;
    char *buff;
    nfree(*result);

    if ((v = findVariable(tpl, name)) == NULL) {
        if ((buff = getenv(name)) != NULL)
            xstrcat(result, buff);
        else
            *result = sstrdup("");
    } else
        switch (v->type) {
            case T_STRING: xstrcat(result, *((char **)v->value) ? *((char **)v->value) : ""); break;
            case T_INT: xscatprintf(result, "%d", *((int *)v->value)); break;
        }
}

short boolExpression(char *str)
{
  char *p, *p1, *p2;
  short ret, inquote, relax;

  ret=1;
  inquote=0;
  for (p=str; isspace(*p); p++);
  for (p1=p; *p1; p1++)
  {
    if (p1[0]=='\\' && (p1[1]=='\\' || p1[1]=='\"'))
    {
      p1++;
      continue;
    }
    if (*p1=='\"')
    {
      inquote = !inquote;
      continue;
    }
    if (!inquote)
      if ((p1[0] == '=' || p1[0] == '!') && (p1[1] == '=' || p1[1] == '~'))
        break;
  }
  if (*p1==0)
  {
    sprintf(htplError, "Error in expression %s: no command found", str);
    return -1;
  }
  if (p1[0]=='!') ret=!ret;
  relax=(p1[1]=='~');
  *p1=0;
  for (p2=p1-1; isspace(*p2); *p2--=0);
  for (p1+=2; isspace(*p1); p1++);
  for (p2=p1+sstrlen(p1)-1; isspace(*p2); *p2--=0);
  if (relax ? patimat(p, p1) : sstricmp(p, p1))
    ret=!ret;
  return ret;
}

int expandMacro(template *tpl, char **macro)
{
    char *buf=NULL;
    char *fmt_ptr = NULL;
    char *format = NULL;
    char *lbl = NULL; // label of variable
    format_t f;

    if ((fmt_ptr = strchr(*macro, '%')) != NULL)
    {
        format = sstrdup(fmt_ptr+1);
        if (!parseVarFormat(&f, format, strlen(*macro)+2)) { // +2 because of 2 '@'
            sprintf(htplError, "\"%s\" : invalid format", format);
            nfree(*macro);
            *macro = sstrdup("");
            return 0;
        }

        lbl = (char *) scalloc(fmt_ptr - (*macro) + 1, 1);
        strncpy(lbl, *macro, (fmt_ptr - (*macro)));
    }
    else
        lbl = sstrdup(*macro);

    getValue(tpl, lbl, &buf);
    nfree(lbl);

    if (fmt_ptr)
        varFormat(macro, &f, buf);
    else
        xstrcpy(macro, buf);

    return 1;
}

int expandLine(template *tpl, char *line, char **output)
{
    int i, escape=0;
    char *ptr;
    char *macStart=NULL;
    char *fmtStart=NULL;
    char *macro=NULL;
    char *strformat=NULL;
    int format_flag=0;
    format_t fmt;

    i = strlen(line);

    nfree(*output);
    *output = (char *)scalloc(i+1, 1);
    ptr = *output;

    while(*line) {
        if (*line == '\\') {
            escape = 1;
            line++;
        } else
        {
            if ((*line == '@') &! (escape) &! (fmtStart))
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
            if ((*line == '%') &! (escape) &! (macStart))
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
        sprintf(htplError, "Unbalanced %% operator");
        return 0;
    }

    if (format_flag) {
        strformat = sstrdup(*output);
        strFormat(output, &fmt, strformat);
        nfree(strformat);
    }
    if (!*output)
        *output = sstrdup("");

    return 1;
}

int parseLine(template *tpl, char *file, char *line, int lineNo)
{
    char *out = NULL;
    int rc = 1;

    if (*line == '#') {
        rc = parseDirective(tpl, file, line);
        if (rc == 2) {
            addLine(tpl->currentSection, line, lineNo);
            if (!expandLine(tpl, line, &out)) {
                nfree(out);
                return 0;
            } else nfree(out);
        }
    } else if (tpl->currentSection) {
        addLine(tpl->currentSection, line, lineNo);
        if (!expandLine(tpl, line, &out)) {
            nfree(out);
            return 0;
        } else nfree(out);
    }
    /* or quietly skip the line if out of section */

    return rc;
}

int parseTemplate(template *tpl, char *file)
{
    FILE *f;
    char *line=NULL;
    int lineNo=0, length=0, i=0, eof=0, eol=0, ch;

    if (!tpl) {
        sprintf(htplError, "Error: template structure should be defined!");
        return 0;
    }
    if (!file || strlen(file) == 0) {
        sprintf(htplError, "Error: template filename should be defined!");
        return 0;
    }

    f = fopen(file, "rt");
    if (!f) {
        sprintf(htplError, "Can't open file %s: %s", file, strerror(errno));
        return 0;
    }


    while (!eof) {

        /* read line */
        do {
            /* not fgets() 'cause it concatenates lines without \r on Watcom C / WinDos */
            ch = getc(f);

            if (i >= length) {
                length += 128;
                line = srealloc(line, length);
            }

            if (ch < 0) {  /* EOF */
                line[i] = '\0';
                eol++;
                eof++;
            } else {
                if (ch == '\n') {  /* EOL */
                    line[i] = '\0';
                    eol++;
                } else if (ch != '\r') {  /* CR (must be before LF), ignore */
                    line[i] = ch;
                    i++;
                }
            }
        } while (!eol);

        /* parse line */
        lineNo++;
        if (!parseLine(tpl, file, line, lineNo))
        {
            makeErrorHeader("Error at %s:%d - ", file, lineNo);
            nfree(line);
            fclose(f);
            return 0;
        }
        eol = i = 0;
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
    char *buff = NULL;
    section *s;
    sectionLine *line;

    if (!tpl) {
        sprintf(htplError, "Error: template structure should be defined!");
        return 0;
    }
    if (!name || strlen(name) == 0) {
        sprintf(htplError, "Error: section name should be defined!");
        return 0;
    }
    if (!output) {
        sprintf(htplError, "Error: output buffer should be defined!");
        return 0;
    }

    /* these are not errors! */
    if ((s = findSection(tpl, name)) == NULL) return 1; /* section not found */
    if (!s->firstLine) return 1; /* section contains no lines */

    /* for parsing #if,#else,... directives  */
    tpl->currentSection = s;
    s->iflevel = -1;
    s->condition = 1;

    line = s->firstLine;
    while (line) {
        if (*(line->text) == '#') {
            if (!parseDirective(tpl, s->file, line->text)) {
                makeErrorHeader("Error at %s:%d - ", s->file, line->lineNo);
                return 0;
            }
        } else if (s->condition) {
            if (!expandLine(tpl, line->text, &buff)) {
                makeErrorHeader("Error at %s:%d - ", s->file, line->lineNo);
                nfree(buff);
                return 0;
            }
            xstrscat(output, buff, "\r", NULL);
        }
        line = line->next;
    }
    return 1;
}

int parseDirective(template *tpl, char *file, char *orig_line)  // extracts token and its values from line
{
    char *line, *buff;
    char token_label[256];
    int token_id, ret;
    section *s;

    if (orig_line==NULL || strlen(orig_line)<=1)
        return 1;

    line = htpl_trimLine(orig_line+1);

    memset(token_label, 0, 256);
    line=detectToken(line, token_label);
    // now line is without token - just token's value up to the end of line
    token_id=findTokenID(token_label);

    line = htpl_trimLine(line);

    switch (token_id) {
    case ID_INCLUDE:
        if (tpl->currentSection) {
            sprintf(htplError, "Found #include directive while section \"%s\" in %s has not been closed by #endsection yet",
                tpl->currentSection->name, tpl->currentSection->file);
            return 0;
        }
        return parseTemplate(tpl, htpl_trimLine(line));
        break;
    case ID_SECTION:
        if (tpl->currentSection) {
            sprintf(htplError, "Found new #section directive while section \"%s\" in %s has not been closed by #endsection yet",
                tpl->currentSection->name, tpl->currentSection->file);
            return 0;
        } else if ((s = findSection(tpl, line)) != NULL) {
            sprintf(htplError, "Section \"%s\" has already been defined in %s",
                line, s->file);
            return 0;
        } else {
            s = newSection(file, line);
            tpl->currentSection = s;
        }
        break;
    case ID_ENDSECTION:
        if (tpl->currentSection)
            addSection(tpl);
        else {
            sprintf(htplError, "Found #endsection without #section");
            return 0;
        }
        break;
    case ID_IF:
        if (!tpl->currentSection) {
            sprintf(htplError, "Found #if directive out of section");
            return 0;
        } else if (line == NULL || strlen(line) == 0) {
            sprintf(htplError, "No expression after #if directive");
            return 0;
        }
        s = tpl->currentSection;
        expandLine(tpl, line, &buff);
        s->iflevel++;
        if (s->iflevel == s->maxif)
          s->ifstack = srealloc(s->ifstack, (s->maxif+=10)*sizeof(ifstack));
        s->ifstack[s->iflevel].inelse = 0;
        if ((ret = boolExpression(buff)) == -1) return 0;  /* error */
        s->ifstack[s->iflevel].state = ret;
        s->condition = s->ifstack[s->iflevel].state;
        nfree(buff);
        return 2;
    case ID_ELSEIF:
        if (!tpl->currentSection) {
            sprintf(htplError, "Found #elseif directive out of section");
            return 0;
        } else if ((tpl->currentSection->iflevel==-1) || tpl->currentSection->ifstack[tpl->currentSection->iflevel].inelse) {
            sprintf(htplError, "Misplaced #elseif");
            return 0;
        } else if (line == NULL || strlen(line) == 0) {
            sprintf(htplError, "No expression after #else directive");
            return 0;
        }
        s = tpl->currentSection;
        expandLine(tpl, line, &buff);
        if (s->ifstack[s->iflevel].state)
          s->ifstack[s->iflevel].state = 0;
        else {
          if ((ret = boolExpression(buff)) == -1) return 0;  /* error */
          s->ifstack[s->iflevel].state = ret;
        }
        s->condition = s->ifstack[s->iflevel].state;
        nfree(buff);
        return 2;
    case ID_ELSE:
        if (!tpl->currentSection) {
            sprintf(htplError, "Found #else directive out of section");
            return 0;
        } else if ((tpl->currentSection->iflevel==-1) || tpl->currentSection->ifstack[tpl->currentSection->iflevel].inelse) {
            sprintf(htplError, "Misplaced #else");
            return 0;
        }
        s = tpl->currentSection;
        s->ifstack[s->iflevel].inelse = 1;
        s->ifstack[s->iflevel].state = !s->ifstack[s->iflevel].state;
        s->condition = s->ifstack[s->iflevel].state;
        return 2;
    case ID_ENDIF:
        if (!tpl->currentSection) {
            sprintf(htplError, "Found #endif directive out of section");
            return 0;
        } else if (tpl->currentSection->iflevel==-1) {
            sprintf(htplError, "Misplaced #endif");
            return 0;
        }
        s = tpl->currentSection;
        s->iflevel--;
        if (s->iflevel == -1)
            s->condition = 1;
        else
            s->condition = s->ifstack[s->iflevel].state;
        return 2;
    case ID_IFDEF:
    case ID_IFNDEF:
        if (!tpl->currentSection) {
            sprintf(htplError, "Found #ifdef or #ifndef directive out of section");
            return 0;
        } else if (line == NULL || strlen(line) == 0) {
            sprintf(htplError, "No expression after #ifdef or #ifndef directive");
            return 0;
        }
        s = tpl->currentSection;
        expandLine(tpl, line, &buff);
        s->iflevel++;
        if (s->iflevel == s->maxif)
            s->ifstack = srealloc(s->ifstack, (s->maxif+=10)*sizeof(ifstack));
        s->ifstack[s->iflevel].inelse = 0;
        if (buff == NULL || strlen(buff) == 0)
            s->ifstack[s->iflevel].state = 0;
        else
            s->ifstack[s->iflevel].state = 1;
        if (token_id == ID_IFNDEF)
            s->ifstack[s->iflevel].state = !s->ifstack[s->iflevel].state;
        s->condition = s->ifstack[s->iflevel].state;
        nfree(buff);
        return 2;
    case -1:
        sprintf(htplError, "Internal error while searching for token id");
        return 0;
    case -2:
        sprintf(htplError, "unknown token: %s", token_label);
        return 0;
    default:
        sprintf(htplError, "Internal error while processing token id");
        return 0;
    }
    return 1; // all ok
}
