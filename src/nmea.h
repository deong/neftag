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
    unsigned int fix_time;
    double latitude;
    char lat_ref;
    double longitude;
    char lon_ref;
    int fix_quality;
    int num_sats;
    double h_dilution;
    double altitude;
    char alt_units;
    double geoid_height;
    char geoid_units;
} gga_t;

typedef struct
{
    unsigned int fix_time;
    char status; /* A (active), V (void) */
    double latitude;
    char lat_ref;
    double longitude;
    char lon_ref;
    double speed; /* in knots */
    double heading; /* in degrees */
    unsigned int fix_date;
} rmc_t;

typedef struct
{
    unsigned int record_type;
    union
    {
        gga_t* gga;
        rmc_t* rmc;
    };
} waypoint_t;

void parse_nmea_file(FILE* fp, waypoint_t** rows, int* num_rows);
void init_gga_rec(gga_t* rec, char** toks);
void init_rmc_rec(rmc_t* rec, char** toks);

#endif
