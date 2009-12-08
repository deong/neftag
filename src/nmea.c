/*
 * nmea.c
 * functions for parsing NMEA files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nmea.h"
#include "util.h"
#include "csv.h"

void parse_nmea_file(FILE* fp, waypoint_t** rows, int* num_recs)
{
    char** toks;
    char*  line;
    int    max_recs = 1024;
    
    /* init some memory to use to parse the nmea file */
    toks = (char**)malloc(NUM_TOKENS * sizeof(char*));

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

waypoint_t* find_location_at(waypoint_t* rows, unsigned int nrows, time_t ts, int epsilon)
{
    int low = 0;
    int mid;
    int high = nrows - 1;
    
    while(low < high)
    {
        mid = (low + high) / 2;
        if(rows[mid].rmc->when < ts)
            low = mid;
        else if(rows[mid].rmc->when > ts)
            high = mid;
        else
            return &rows[mid];
    }

    /* low has passed high, so check which is closer to desired time */
    if(fabs(rows[low].rmc->when - ts) < fabs(rows[high].rmc->when - ts) &&
       fabs(rows[low].rmc->when - ts) < epsilon)
        return &rows[low];
    else if(fabs(rows[high].rmc->when - ts) < fabs(rows[low].rmc->when - ts) &&
            fabs(rows[high].rmc->when - ts) < epsilon)
        return &rows[high];

    /* no record found within required time limit */
    return NULL;
}
