#ifndef _SECTIONS_H
#define _SECTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

sectionLine *newLine();
void addLine(section *s, char *text, int lineNo);
void deleteLine(sectionLine *l);
section *newSection(char *file, char *name);
section *findSection(template *tpl, char *name);
int addSection(template *tpl);
void deleteSection(section *s);
void deleteAllSections(template *tpl);

#ifdef __cplusplus
}
#endif

#endif
