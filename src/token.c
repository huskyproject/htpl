#include <string.h>
#include <stdio.h>

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

int parseDirective(template *tpl, char *file, char *orig_line)  // extracts token and its values from line
{
    char *line, *buff;
    char token_label[256];
    int token_id, i;
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
        return parseTemplate(tpl, stripn(htpl_trimLine(line)));
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
        s->ifstack[s->iflevel].state = boolExpression(buff);
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
        else
          s->ifstack[s->iflevel].state = boolExpression(buff);
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
