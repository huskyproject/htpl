#ifndef _SECTIONS_H
#define _SECTIONS_H

typedef struct {
    char *text;
    char *file;
    int lineNo;
    void *next;
} sectionLine;

typedef struct {
    char *name;
    sectionLine *firstLine;
    sectionLine *line;
    void *next;
    void *prev;
} section;

extern section *currentSection;

section *findSection(char *name);
void addLine(section *s, char *text, char *file, int lineNo);
int addSection(char *name);
void deleteSections();

#endif
