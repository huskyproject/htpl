#ifndef _HTPL_H
#define _HTPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <huskylib/compiler.h>
#include <huskylib/huskylib.h>

/* Error buffer for checkNull(). I think it is enough 1024 characters for
 error message. Enlarge it if I was wrong. */
#define HTPLERRORSIZE 1024
HUSKYEXT char htplError[HTPLERRORSIZE];

/* types of variables */
enum {
    T_STRING = 0,
    T_INT
};

typedef struct s_line {
    char *text;
    int lineNo;
    struct s_line *next;
} sectionLine;

typedef struct s_section {
    char *name;
    char *file;
    sectionLine *firstLine;
    sectionLine *line;
    struct s_section *next;
} section;

typedef struct s_variable {
    char *label;
    void **value;
    int type;
    struct s_variable *next;
} variable;

typedef struct s_template {
    section *firstSection;
    section *currentSection;
    variable *firstVariable;
    char *htplError;
} template;


/* makes a new template */
HUSKYEXT template *newTemplate();

/* reads template and make a structure of sections */
HUSKYEXT int parseTemplate(template *tpl, char *file);

/* parses section's lines and prints output into buffer */
HUSKYEXT void parseSection(template *tpl, char *name, char **output);

/* add variable to list */
HUSKYEXT int registerVariable(template *tpl, char *label, void **value, int type);

/* removes variable from list */
HUSKYEXT void unregisterVariable(template *tpl, char *name);

/* removes all variables from list */
HUSKYEXT void unregisterVariables(template *tpl);

/* removes the template */
HUSKYEXT void deleteTemplate(template *tpl);

#ifdef __cplusplus
}
#endif

#endif
