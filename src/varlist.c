#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* export functions from DLL */
#define DLLEXPORT
#include <huskylib/huskyext.h>

#include "htpl.h"
#include "varlist.h"

extern char htplError[HTPLERRORSIZE];

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

int registerVariable(template *tpl, char *label, void **value, e_vartype type)
{
    variable *v, *vtmp;

    if (!tpl) {
        sprintf(htplError, "Error: template structure should be defined!");
        return 0;
    }
    if (!label || strlen(label) == 0) {
        sprintf(htplError, "Error: variable label should be defined!");
        return 0;
    }
    if (!value) {
        sprintf(htplError, "Error: pointer to variable should be defined!");
        return 0;
    }

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

int unregisterVariable(template *tpl, char *label)
{
    variable *v, *vtmp;

    if (!tpl) {
        sprintf(htplError, "Error: template structure should be defined!");
        return 0;
    }
    if (!label || strlen(label) == 0) {
        sprintf(htplError, "Error: variable label should be defined!");
        return 0;
    }

    if ((v = tpl->firstVariable) == NULL) return 1;
    if ((v->label) && (!strcmp(v->label, label))) {
        tpl->firstVariable = v->next;
        deleteVariable(v);
        return 1;
    }
    while (v->next) {
        if ((v->next->label) && (!strcmp(v->next->label, label))) {
            vtmp = v->next;
            v->next = vtmp->next;
            deleteVariable(vtmp);
            return 1;
        } else
            v = v->next;
    }
    return 1;
}

int unregisterAllVariables(template *tpl)
{
    variable *v, *vtmp;

    if (!tpl) {
        sprintf(htplError, "Error: template structure should be defined!");
        return 0;
    }

    v = tpl->firstVariable;
    while (v) {
        vtmp = v->next;
        deleteVariable(v);
        v = vtmp;
    }
    return 1;
}
