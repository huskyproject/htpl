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


/* reads template and make a structure of sections */
HUSKYEXT int parseTemplate(char *filename);

/* Callback function. Returns value by name. Each program wich uses libhtpl
   *must* have this function. */
HUSKYEXT int getValue(char *name, void **result);

/* Error handler (like assert() but it prints error into buffer htplError) */
HUSKYEXT int isError(char *var, char *msg,...);

/* parses section's lines and prints output into buffer */
HUSKYEXT int parseSection(char *name, char **output);

/* add variable to list */
HUSKYEXT int registerVariable(char *label, void **value, int type);

/* removes variable from list */
HUSKYEXT void unregisterVariable(char *name);

/* removes all variables from list */
HUSKYEXT void unregisterAllVars();

HUSKYEXT void deleteSections();

#ifdef __cplusplus
}
#endif

#endif