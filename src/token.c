#include <string.h>
#include <stdio.h>
#include "mem.h"
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

int parseDirective(char *orig_line)  // extracts token and its values from line
{
    char *line;
    char token_label[256];
    int token_id;

    if (orig_line==NULL)
        return 1;

    line=(char *) sstrdup(stripn(trimLine(orig_line)));

    if (strlen(line)==0)
        return 1;

    memset(token_label, 0, 256);
    line=detectToken(line, token_label);
    // now line is without token - just token's value up to the end of line
    token_id=findTokenID(token_label);

    line=trimLine(line);

    switch (token_id) {
    case ID_INCLUDE:
        return parseTemplate(stripn(trimLine(line)));
        break;
    case ID_SECTION:
        return addSection(line);
        break;
    case ID_ENDSECTION:
        if (currentSection && currentSection->prev)
            popSection();
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
