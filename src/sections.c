#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "htpl.h"
#include "sections.h"


sectionLine *newLine()
{
    sectionLine *l;
    l = (sectionLine *) scalloc(sizeof(sectionLine), 1);
    return l;
}

void deleteLine(sectionLine *l)
{
    nfree(l->text);
    nfree(l);
}

section *newSection(char *file, char *name)
{
    section *s;
    s = (section *) scalloc(sizeof(section), 1);
    s->file = sstrdup(file);
    s->name = sstrdup(name);
    return s;
}

void deleteSection(section *s)
{
    sectionLine *linetmp;
    nfree(s->name);
    nfree(s->file);
    if (s->firstLine)
        s->line = s->firstLine;
    else return;
    while(s->line->next) {
        linetmp = s->line->next;
        deleteLine(s->line);
        s->line = linetmp;
    }
    deleteLine(s->line);
    nfree(s);
}

section *findSection(template *tpl, char *name)
{
    section *s;
    if (!tpl->firstSection) return NULL;
    s = tpl->firstSection;
    while((s) && (s->name) && (strcmp(s->name, name)))
        s = s->next;
    if ((s == NULL) || (s->name == NULL))
        return NULL;
    else
        return s;
}

void addLine(section *s, char *text, int lineNo)
{
    sectionLine *l = newLine();
    l->text = sstrdup(text);
    l->lineNo = lineNo;
    if (!s->firstLine)
        s->firstLine = l;
    else
    {
        s->line = s->firstLine;
        while(s->line->next) s->line = s->line->next;
        s->line->next = l;
    }
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

void deleteSections(template *tpl)
{
    section *s, *stmp;
    if (!tpl->firstSection) return;
    s = tpl->firstSection;
    while(s->next) {
        stmp = (section *)s->next;
        deleteSection(s);
        s = (section *)stmp;
    }
    deleteSection(s);
}

void deleteTemplate(template *tpl)
{
    if (tpl->firstSection) deleteSections(tpl);
    if (tpl->firstVariable) unregisterVariables(tpl);
    nfree(tpl->htplError);
    nfree(tpl);
}
