/*
 * nmea.h
 * functions for parsing NMEA data files
 */

#ifndef _NMEA_H_
#define _NMEA_H_

#include <stdio.h>
#include <time.h>

#define GPGGA 0
#define GPRMC 1

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
} rmc_t;

typedef struct
{
    unsigned int record_type;
    union
    {
        rmc_t* rmc;
    };
} waypoint_t;

void parse_nmea_file(FILE* fp, waypoint_t** rows, int* num_rows);
void init_rmc_rec(rmc_t* rec, char** toks);
waypoint_t* find_location_at(waypoint_t* rows, unsigned int nrows, time_t timestamp,
    int epsilon);

#endif
