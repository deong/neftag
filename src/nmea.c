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

    /* parse each GGA record */
    *num_recs = 0;
    line = (char*)malloc(82 * sizeof(char));
    while((line = fgets(line, 82, fp)) != NULL)
    {
        parse_line(line, toks);
        if(strncmp(toks[0], "$GPGGA", MAX_TOKEN_LEN) == 0)
        {
            (*rows)[*num_recs].record_type = GPGGA;
            (*rows)[*num_recs].gga = (gga_t*)malloc(sizeof(gga_t));
            init_gga_rec((*rows)[(*num_recs)++].gga, toks);
        }
        else if(strncmp(toks[0], "$GPRMC", MAX_TOKEN_LEN) == 0)
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

void init_gga_rec(gga_t* rec, char** toks)
{
    rec->fix_time = 0; /* fixme */
    rec->latitude = 0; /* fixme */
    rec->lat_ref = toks[3][0];
    rec->longitude = 0; /* fixme */
    rec->lon_ref = toks[5][0];
    rec->fix_quality = atoi(toks[6]);
    rec->num_sats = atoi(toks[7]);
    rec->h_dilution = atof(toks[8]);
    rec->altitude = atof(toks[9]);
    rec->alt_units = toks[10][0];
    rec->geoid_height = atof(toks[11]);
    rec->geoid_units = toks[12][0];
}

void init_rmc_rec(rmc_t* rec, char** toks)
{
    rec->fix_time = 0; /* fixme */
    rec->status = toks[1][0];
    rec->latitude = 0; /* fixme */
    rec->lat_ref = toks[3][0];
    rec->longitude = 0; /* fixme */
    rec->lon_ref = toks[5][0];
    rec->speed = atof(toks[6]);
    rec->heading = atof(toks[7]);
    rec->fix_date = 0; /* fixme */
}
