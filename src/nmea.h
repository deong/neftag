/*
 * nmea.h
 * functions for parsing NMEA data files
 */

#ifndef _NMEA_H_
#define _NMEA_H_

#include <stdio.h>
#include <time.h>

#define NUM_TOKENS 20
#define MAX_TOKEN_LEN 80

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
    int num_sat;
    int quality;
} location_t;

void parse_nmea_file(FILE* fp, location_t** rows, int* num_rows, int max_size);
void init_rmc_rec(location_t* rec, char** toks);
void process_gga_rec(location_t* rec, char** toks);
location_t* find_location_at(location_t* rows, unsigned int nrows, time_t timestamp,
    int epsilon);
void dec2dms(double dec, int* deg, int* min, double* sec);

#endif
