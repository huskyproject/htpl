/*
 *  This program is used to check templates for syntax errors
 *
 */
#include <stdio.h>

#define DLLIMPORT
#include <huskylib/huskyext.h>

#include "htpl.h"

int main(int argc, char **argv)
{
    int i;

    if (argc == 1)
    {
        printf("Check templates for syntax errors.\n");
        printf("Usage: <template> [<template>...]\n");
        return 1;
    }

    for (i = 1; i < argc; i++)
    {
        template *tpl = newTemplate();

        if (parseTemplate(tpl, argv[i]))
            printf("Template %s is OK\n", argv[i]);
        else
            printf("%s\n", htplError);

        deleteTemplate(tpl);
    }

    return 0;
}
