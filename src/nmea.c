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

/*
 * parse a file of NMEA sentences into an array of location_t records
 */
void parse_nmea_file(FILE* fp, location_t** rows, int* num_recs, int max_size)
{
    char** toks;
    char*  line;
    
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
            init_rmc_rec(&(*rows)[(*num_recs)++], toks);
        }
        else if(strncmp(toks[0], "$GPGGA", MAX_TOKEN_LEN) == 0)
        {
            /* if first record is a GPGGA record, ignore it */
            if(*num_recs > 0)
                process_gga_rec(&(*rows)[(*num_recs)-1], toks);
        }
        else
        {
            /* skip other sentence types */
            continue;
        }

        /* if we've run out of space, realloc twice the space and keep going */
        if(*num_recs >= max_size)
        {
            max_size = max_size * 2;
            *rows = (location_t*)realloc(*rows, max_size * sizeof(location_t));
            if(!*rows)
            {
                fprintf(stderr, "error enlarging nmea sentences array\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    free(toks);
    free(line);
}

/*
 * builds up a location_t record based on a parsed GPRMC sentence
 */
void init_rmc_rec(location_t* rec, char** toks)
{
    struct tm fix_time;
    char tmp[3];
    
    /* time */
    strncpy(tmp, toks[1], 2);
    fix_time.tm_hour = atoi(tmp);
    strncpy(tmp, toks[1]+2, 2);
    fix_time.tm_min = atoi(tmp);
    strncpy(tmp, toks[1]+4, 2);
    fix_time.tm_sec = atoi(tmp);

    /* date */
    strncpy(tmp, toks[9], 2);
    fix_time.tm_mday = atoi(tmp);
    strncpy(tmp, toks[9]+2, 2);
    fix_time.tm_mon = atoi(tmp) - 1;
    strncpy(tmp, toks[9]+4, 2);
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

    /* these fields aren't part of GPRMS sentence; we'll add them later */
    rec->altitude = 0;
    rec->geoid_ht = 0;
    rec->quality = 0;
    rec->num_sat = 0;
}

/*
 * updates a given location_t record with information from a parsed GPGGA sentence
 */
void process_gga_rec(location_t* rec, char** toks)
{
    /* if we've already seen a more accurate GPGGA record and used it,
       then bail.  this is because there may be multiple GPGGA records
       per GPRMC record, and we want to use the first one succeeding a
       GPRMC record, as it should have the closest timestamp. */
    if(rec->altitude > 1e-3)
        return;

    /* make sure we have at least some kind of fix */
    if(atoi(toks[6]) == 0)
        return;

    /* update the previous location_t record with the fix and altitude data
       from the currently parsed GPGGA sentence tokens */
    rec->quality = atoi(toks[6]);
    rec->num_sat = atoi(toks[7]);
    rec->altitude = atof(toks[9]);
    rec->geoid_ht = atof(toks[11]);
}

/*
 * custom binary search that looks for the record in the array whose timestamp
 * is nearest to the given timestamp.  if no records are stamped within epsilon
 * seconds, returns NULL.
 */
location_t* find_location_at(location_t* rows, unsigned int nrows, time_t ts, int epsilon)
{
    int low = 0;
    int mid;
    int high = nrows - 1;

    while(low <= high)
    {
        mid = (low + high) / 2;
        if(rows[mid].when < ts)
            low = mid + 1;
        else if(rows[mid].when > ts)
            high = mid - 1;
        else
            return &rows[mid];
    }

    /* low has passed high, so check which is closer to desired time */
    if(fabs(rows[high].when - ts) <= fabs(rows[low].when - ts) &&
       fabs(rows[high].when - ts) <= epsilon)
        return &rows[high];
    else if(fabs(rows[low].when - ts) < fabs(rows[high].when - ts) &&
       fabs(rows[low].when - ts) <= epsilon)
        return &rows[low];

    /* no record found within required time limit */
    return NULL;
}

/* convert nmea Dm.H format to degrees, minutes, seconds */
void dec2dms(double dec, int* deg, int* min, double* sec)
{
    /* nmea format is weird.  12 deg, 34' 56.78" is stored as 1234.5678 */
    double dec_min;
    *deg = (int)floor(dec / 100);  /* 1234.5678 => 12 degrees */
    dec_min = dec - (*deg * 100);  /* 1234.5678 - (12 * 100) => 34.5678 */
    *min = (int)floor(dec_min);    /* 34 minutes */
    *sec = (dec_min - *min) * 60;  /* (34.5678 - 34) * 60 => 34.068 sec */
}

    
    
