#ifndef _SECTIONS_H
#define _SECTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

section *findSection(template *tpl, char *name);
void addLine(section *s, char *text, int lineNo);
int addSection(template *tpl);
section *newSection(char *file, char *name);
void deleteAllSections(template *tpl);

#ifdef __cplusplus
}
#endif

#endif
