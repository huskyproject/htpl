#include <stdio.h>
#include <malloc.h>
#include "htpl.h"

void printSection(char *name, char **report)
{
    if (!parseSection(name, report)) {
        printf("Warning: %s\n", htplError);
    }
}

int main()
{
    char *report=NULL;
    char *timestr=NULL;
    int a=0;

    /* registering our variables for report */
    registerVariable("time", (void **) &timestr, T_STRING);
    registerVariable("a", (void **) &a, T_INT);

    /* parse template file */
    if (!parseTemplate("test.tpl")) {
        printf("%s\n", htplError);
    } else
    {
        /* print parsed section "header" into buffer "report" */
        printSection("header", &report);

        for (a=0; a < 10; a++)
            printSection("a_line", &report);

        printSection("head1", &report);

        printSection("footer", &report);

        printf("%s", report);
    }
    deleteSections();
    unregisterAllVars();
    free(report);
    return 0;
}