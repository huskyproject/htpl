#include <string.h>
#include <stdio.h>

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "strutil.h"
#include "token.h"
#include "htpl.h"
#include "sections.h"

token tokens[] = {
    { "include", ID_INCLUDE },
    { "section", ID_SECTION },
    { "endsection", ID_ENDSECTION },
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

void assignStringValue(char **token, char *value) // assings value to config's string parameter
{

    if (*token == NULL)
        *token = (char *) sstrdup(value);
    else
    {
        nfree(*token);
        *token = (char *) sstrdup(value);
    }
}

int parseDirective(template *tpl, char *file, char *orig_line)  // extracts token and its values from line
{
    char *line;
    char token_label[256];
    int token_id;
    section *s;

    if (orig_line==NULL)
        return 1;

    line=(char *) sstrdup(stripn(htpl_trimLine(orig_line)));

    if (strlen(line)==0)
        return 1;

    memset(token_label, 0, 256);
    line=detectToken(line, token_label);
    // now line is without token - just token's value up to the end of line
    token_id=findTokenID(token_label);

    line=htpl_trimLine(line);

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
