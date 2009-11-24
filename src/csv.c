/*
 * csv.c
 * parse csv file into arrays of tokens
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

void parse_line(char* line, char** toks)
{
    char*  curr;
    char*  p;
    int i = 0;

    i=0;
    while(line != NULL && i < NUM_TOKENS)
    {
        curr = strsep(&line, ",");
        if((p = strchr(curr, '\r')) != NULL)
            *p = '\0';
        else if((p = strchr(curr, '\n')) != NULL)
            *p = '\0';
        strncpy(toks[i++], curr, MAX_TOKEN_LEN);
    }
}
