#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "htpl.h"
#include "sections.h"

extern char htplError[HTPLERRORSIZE];

sectionLine *newLine()
{
    sectionLine *l;
    l = (sectionLine *) scalloc(sizeof(sectionLine), 1);
    return l;
}

void addLine(section *s, char *text, int lineNo)
{
    sectionLine *tmp;
    sectionLine *l = newLine();

    l->text = sstrdup(text);
    l->lineNo = lineNo;
    if (!s->firstLine)
        s->firstLine = l;
    else
    {
        tmp = s->firstLine;
        while (tmp->next) tmp = tmp->next;
        tmp->next = l;
    }
}

void deleteLine(sectionLine *l)
{
    if (!l) return;
    nfree(l->text);
    nfree(l);
}

section *newSection(char *file, char *name)
{
    section *s;
    s = (section *) scalloc(sizeof(section), 1);
    s->file = sstrdup(file);
    s->name = sstrdup(name);
    s->iflevel = -1;
    s->condition = 1;
    return s;
}

section *findSection(template *tpl, char *name)
{
    section *s;

    if (!tpl->firstSection) return NULL;
    s = tpl->firstSection;
    while ((s) && (s->name)) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next;
    }
    return NULL;
}

int addSection(template *tpl)
{
    section *s;

    if (!tpl->firstSection)
        tpl->firstSection = tpl->currentSection;
    else
    {
        s = tpl->firstSection;
        while(s->next) s = s->next;
        s->next = tpl->currentSection;
    }
    tpl->currentSection = NULL;

    return 1;
}

void deleteSection(section *s)
{
    sectionLine *tmp, *line;

    if (!s) return;
    nfree(s->name);
    nfree(s->file);
    nfree(s->ifstack);
    line = s->firstLine;
    while (line) {
        tmp = line->next;
        deleteLine(line);
        line = tmp;
    }
    nfree(s);
}

void deleteAllSections(template *tpl)
{
    section *s, *stmp;

    if (!tpl->firstSection) return;
    s = tpl->firstSection;
    while (s) {
        stmp = s->next;
        deleteSection(s);
        s = stmp;
    }
}

int deleteTemplate(template *tpl)
{
    if (!tpl) {
        sprintf(htplError, "Error: template structure should be defined!");
        return 0;
    }

    if (tpl->firstSection) deleteAllSections(tpl);
    if (tpl->firstVariable) unregisterAllVariables(tpl);
    nfree(tpl);
    return 1;
}
