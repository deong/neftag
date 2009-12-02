/*
 * nmea.c
 * functions for parsing NMEA files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nmea.h"
#include "util.h"
#include "csv.h"

void parse_nmea_file(FILE* fp, waypoint_t** rows, int* num_recs)
{
    char** toks;
    char*  line;
    int    i;
    int    max_recs = 1024;
    
    /* init some memory to use to parse the nmea file */
    toks = (char**)malloc(NUM_TOKENS * sizeof(char*));
    for(i=0; i<NUM_TOKENS; ++i)
    {
        toks[i] = (char*)malloc(MAX_TOKEN_LEN * sizeof(char));
        *toks[i] = '\0';
    }

    /* parse each GPRMC record */
    *num_recs = 0;
    line = (char*)malloc(82 * sizeof(char));
    while((line = fgets(line, 82, fp)) != NULL)
    {
        parse_line(line, ",", toks);
        if(strncmp(toks[0], "$GPRMC", MAX_TOKEN_LEN) == 0)
        {
            (*rows)[*num_recs].record_type = GPRMC;
            (*rows)[*num_recs].rmc = (rmc_t*)malloc(sizeof(rmc_t));
            init_rmc_rec((*rows)[(*num_recs)++].rmc, toks);
        }
        else
        {
            /* skip other sentence types */
            continue;
        }
        
        if(*num_recs >= max_recs)
        {
            max_recs += 1024;
            *rows = (waypoint_t*)realloc(*rows, max_recs * sizeof(waypoint_t));
            if(!*rows)
            {
                fprintf(stderr, "error enlarging gga rows array\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    /* free the memory used by the nmea parsing */
    for(i=0; i<NUM_TOKENS; ++i)
        free(toks[i]);
    free(toks);
    free(line);
}

void init_rmc_rec(rmc_t* rec, char** toks)
{
    char raw_time[11];
    char raw_date[7];
    char tmp[3];
    struct tm fix_time;

    /* time */
    strncpy(raw_time, toks[1], sizeof(raw_time));
    strncpy(tmp, raw_time, 2);
    fix_time.tm_hour = atoi(tmp);
    strncpy(tmp, raw_time+2, 2);
    fix_time.tm_min = atoi(tmp);
    strncpy(tmp, raw_time+4, 2);
    fix_time.tm_sec = atoi(tmp);

    /* date */
    strncpy(raw_date, toks[9], sizeof(raw_date));
    strncpy(tmp, raw_date, 2);
    fix_time.tm_mday = atoi(tmp);
    strncpy(tmp, raw_date+2, 2);
    fix_time.tm_mon = atoi(tmp) - 1;
    strncpy(tmp, raw_date+4, 2);
    fix_time.tm_year = 2000 + atoi(tmp) - 1900;

    /* set the offset from UTC to 0, as GPS reports times in UTC anyway */
    fix_time.tm_gmtoff = 0;
    
    rec->when = timegm(&fix_time);
    rec->status = toks[2][0];
    rec->latitude = atof(toks[3]);
    rec->lat_ref = toks[4][0];
    rec->longitude = atof(toks[5]); 
    rec->lon_ref = toks[6][0];
    rec->speed = atof(toks[7]);
    rec->heading = atof(toks[8]);
}
