#include "varlist.h"
#include "mem.h"
#include "htpl.h"

variable *firstVariable=NULL;

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

variable *findVariable(char *label)
{
    variable *v;
    if ((v = firstVariable) == NULL) return NULL;
    while((v) && (v->label) && (strcmp(v->label, label)))
        v = v->next;
    if ((v == NULL) || (v->label == NULL))
        return NULL;
    else
        return v;
}

int registerVariable(char *label, void **value, int type)
{
    variable *v, *vtmp;
    if ((v=findVariable(label))!=NULL) {
        sprintf(htplError, "variable \"%s\" has already been registered", label);
        return 0;
    } else
        v = newVariable();
    v->label = sstrdup(label);
    v->type = type;
    v->value = value;
    if (!firstVariable)
        firstVariable = v;
    else
    {
        vtmp = firstVariable;
        while(vtmp->next) vtmp = vtmp->next;
        vtmp->next = v;
    }
}

void unregisterVariable(char *label)
{
    deleteVariable(findVariable(label));
}

void unregisterAllVars()
{
    variable *v, *vtmp;
    v = firstVariable;
    if ((v = firstVariable) == NULL) return;
    while(v->next) {
        vtmp = v->next;
        deleteVariable(v);
        v = vtmp;
    }
    deleteVariable(v);
}
