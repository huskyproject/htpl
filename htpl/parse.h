#ifndef _PARSE_H
#define _PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

void getValue(template *tpl, char *name, char **result);
short boolExpression(char *str);
int expandMacro(template *tpl, char **macro);
int expandLine(template *tpl, char *line, char **output);
int parseLine(template *tpl, char *file, char *line, int lineNo);
int parseDirective(template *tpl, char *file, char *orig_line);

#ifdef __cplusplus
}
#endif

#endif
