#ifndef _HTPL_H
#define _HTPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* Error buffer. I think it is enough 1024 characters for error message. */
/* Enlarge it if I was wrong. */
#define HTPLERRORSIZE 1024
HUSKYEXT char htplError[HTPLERRORSIZE];

/* types of variables */
typedef enum {
    T_STRING = 0,
    T_INT
} e_vartype;

typedef struct s_line {
    char *text;
    int lineNo;
    struct s_line *next;
} sectionLine;

typedef struct {
    short state, inelse;
} ifstack;

typedef struct s_section {
    char *name;
    char *file;
    sectionLine *firstLine;
    struct s_section *next;
    short condition;
    int iflevel;
    int maxif;
    ifstack *ifstack;
} section;

typedef struct s_variable {
    char *label;
    void **value;
    e_vartype type;
    struct s_variable *next;
} variable;

typedef struct s_template {
    section *firstSection;
    section *currentSection;
    variable *firstVariable;
} template;


/* makes a new template */
HUSKYEXT template *newTemplate();

/* reads template and makea a structure of sections */
/* returns 1 on success, and 0 if any error occured, */
/* parseSection() should not be called in this case or it */
/* will fail with the same error */
HUSKYEXT int parseTemplate(template *tpl, char *file);

/* parses section's lines and prints output into output buffer */
/* returns 1 on success, and 0 if any error occured */
HUSKYEXT int parseSection(template *tpl, char *name, char **output);

/* add variable to list */
/* returns 1 on success, and 0 if any error occured */
HUSKYEXT int registerVariable(template *tpl, char *label, void **value, e_vartype type);

/* removes variable from list */
/* returns 1 on success, and 0 if any error occured */
HUSKYEXT int unregisterVariable(template *tpl, char *name);

/* removes all variables from list */
/* returns 1 on success, and 0 if any error occured */
HUSKYEXT int unregisterAllVariables(template *tpl);

/* removes the template */
/* returns 1 on success, and 0 if any error occured */
HUSKYEXT int deleteTemplate(template *tpl);

#ifdef __cplusplus
}
#endif

#endif
