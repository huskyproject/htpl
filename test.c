#include <stdio.h>
#include <malloc.h>
#include "htpl/htpl.h"

void printSection(template *tpl, char *name, char **report)
{
    if (!parseSection(tpl, name, report)) {
        printf("Warning: %s\n", htplError);
    }
}

int main()
{
    char *report=NULL;
    char *timestr=NULL;
    int a=0;
    template *tpl;

    /* create new template */
    tpl = newTemplate();

    /* register our variables for report */
    registerVariable(tpl, "time", (void **) &timestr, T_STRING);
    registerVariable(tpl, "a", (void **) &a, T_INT);

    /* parse template file */
    if (!parseTemplate(tpl, "test.tpl")) {
        printf("%s\n", htplError);
    } else
    {
        /* print parsed section "header" into buffer "report" */
        printSection(tpl, "header", &report);

        for (a=0; a < 10; a++)
            printSection(tpl, "a_line", &report);

        printSection(tpl, "head1", &report);

        printSection(tpl, "footer", &report);

        printf("%s", report);
    }

    /* delete template (free memory) */
    deleteTemplate(tpl);

    free(report);
    return 0;
}
