#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "htpl.h"
#include "varlist.h"


variable *newVariable()
{
    variable *v;
    v = (variable *) scalloc(sizeof(variable), 1);
    return v;
}

void deleteVariable(variable *v)
{
    if (!v) return;
    nfree(v->label);
    nfree(v);
}

variable *findVariable(template *tpl, char *label)
{
    variable *v;

    if ((v = tpl->firstVariable) == NULL) return NULL;
    while((v) && (v->label) && (strcmp(v->label, label)))
        v = v->next;
    if ((v == NULL) || (v->label == NULL))
        return NULL;
    else
        return v;
}

int registerVariable(template *tpl, char *label, void **value, int type)
{
    variable *v, *vtmp;

    if ((v = findVariable(tpl, label)) != NULL) {
        sprintf(htplError, "variable \"%s\" has already been registered", label);
        return 0;
    } else
        v = newVariable();
    v->label = sstrdup(label);
    v->type = type;
    v->value = value;
    if (!tpl->firstVariable)
        tpl->firstVariable = v;
    else
    {
        vtmp = tpl->firstVariable;
        while(vtmp->next) vtmp = vtmp->next;
        vtmp->next = v;
    }
    return 1;
}

void unregisterVariable(template *tpl, char *label)
{
    variable *v, *vtmp;

    if ((v = tpl->firstVariable) == NULL) return;
    if ((v->label) && (!strcmp(v->label, label))) {
        tpl->firstVariable = v->next;
        deleteVariable(v);
        return;
    }
    while(v->next)
        if ((v->next->label) && (!strcmp(v->next->label, label))) {
            vtmp = v->next;
            v->next = vtmp->next;
            deleteVariable(vtmp);
            return;
        } else
            v = v->next;
}

void unregisterVariables(template *tpl)
{
    variable *v, *vtmp;

    if ((v = tpl->firstVariable) == NULL) return;
    while(v->next) {
        vtmp = v->next;
        deleteVariable(v);
        v = vtmp;
    }
    deleteVariable(v);
}
