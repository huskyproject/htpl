#ifndef _VARLIST_H
#define _VARLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

variable *newVariable();
variable *findVariable(template *tpl, char *label);
void deleteVariable(variable *v);

#ifdef __cplusplus
}
#endif

#endif
