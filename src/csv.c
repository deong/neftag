/*
 * csv.c
 * parse csv file into arrays of tokens
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

/*
 * parse a line of comma separated text into a given
 * array of tokens
 */
void parse_line(char* line, char* sep, char** toks)
{
    char*  curr;
    char*  p;
    int i = 0;

    i=0;
    while(line != NULL && i < NUM_TOKENS)
    {
        curr = strsep(&line, sep);
        
        /* NMEA files from my device are DOS formatted; kill the \r\n stuff */
        if((p = strchr(curr, '\r')) != NULL)
            *p = '\0';
        else if((p = strchr(curr, '\n')) != NULL)
            *p = '\0';
        
        /* note we're reusing the space in the line here */
        toks[i++] = curr;
    }
}
