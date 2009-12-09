/*
 * nmea.h
 * functions for parsing NMEA data files
 */

#ifndef _NMEA_H_
#define _NMEA_H_

#include <stdio.h>
#include <time.h>

typedef enum
{
    GPRMC=0
} sentence_id;

typedef struct
{
    time_t when;
    char status; /* A (active), V (void) */
    double latitude;
    char lat_ref;
    double longitude;
    char lon_ref;
    double speed; /* in knots */
    double heading; /* in degrees */
    double altitude; /* in meters */
    double geoid_ht; /* in meters */
} rmc_t;

typedef struct
{
    sentence_id record_type;
    union
    {
        rmc_t* rmc;
    };
} sentence_t;

void parse_nmea_file(FILE* fp, sentence_t** rows, int* num_rows);
void init_rmc_rec(rmc_t* rec, char** toks);
void process_gga_rec(rmc_t* rec, char** toks);
sentence_t* find_location_at(sentence_t* rows, unsigned int nrows, time_t timestamp,
    int epsilon);

#endif
