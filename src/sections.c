#include "sections.h"
#include "mem.h"
#include "htpl.h"

typedef struct
{
    section *s;
    void *up;
} s_stack;

s_stack *section_stack=NULL;
section *firstSection=NULL;
section *currentSection=NULL;


sectionLine *newLine()
{
    sectionLine *l;
    l = (sectionLine *) scalloc(sizeof(sectionLine), 1);
    return l;
}

void deleteLine(sectionLine *l)
{
    nfree(l->text);
    nfree(l->file);
    nfree(l);
}

void pushSection(section *s)
{
    s_stack *ss;

    currentSection = s;
    ss = (s_stack *) scalloc(sizeof(s_stack), 1);
    ss->s = currentSection;
    ss->up = section_stack;
    section_stack = ss;
}

void popSection()
{
    s_stack *ss;

    if (!section_stack) return;
    ss = section_stack;
    section_stack = section_stack->up;
    currentSection = section_stack->s;
    nfree(ss);
}


section *newSection()
{
    section *s;
    s = (section *) scalloc(sizeof(section), 1);
    return s;
}

void deleteSection(section *s)
{
    sectionLine *linetmp;
    nfree(s->name);
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

section *findSection(char *name)
{
    section *s;
    if (!firstSection) return NULL;
    s = firstSection;
    while((s) && (s->name) && (strcmp(s->name, name)))
        s = s->next;
    if ((s == NULL) || (s->name == NULL))
        return NULL;
    else
        return s;
}

void addLine(section *s, char *text, char *file, int lineNo)
{
    sectionLine *l = newLine();
    l->text = (char *) sstrdup(text);
    l->file = (char *) sstrdup(file);
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

int addSection(char *name)
{
    section *s, *stmp;
    if ((s=findSection(name))!=NULL) {
        sprintf(htplError, "section \"%s\" has already been defined", name);
        return 0;
    } else
        s = newSection();
    s->name = sstrdup(name);
    if (!firstSection)
        firstSection = s;
    else
    {
        stmp = firstSection;
        while(stmp->next) stmp = stmp->next;
        stmp->next = s;
        s->prev = stmp;
    }
    pushSection(s);
}

void deleteSections()
{
    section *s, *stmp;
    if (!firstSection) return;
    s = firstSection;
    while(s->next) {
        stmp = (section *)s->next;
        deleteSection(s);
        s = (section *)stmp;
    }
    deleteSection(s);
}
