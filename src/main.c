/*
 * main.c
 * main program for dealing with d90 raw files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tiff.h"
#include "util.h"
#include "csv.h"
#include "nmea.h"
#include "nikond90.h"
#include "date.h"

#define TIME_ZONE_OFFSET 6

int main(int argc, char** argv)
{
    unsigned int offset;
    unsigned int i;
    unsigned int fnum;
    FILE* fp;
    FILE* gpsf;
    ifd_t zeroth_ifd;
    ifd_t gps_info_ifd;
    unsigned int gps_offset = 0;
    int num_rows = 0;
    waypoint_t* rows = (waypoint_t*)malloc(1024 * sizeof(waypoint_t));
    
    if(argc < 3)
    {
        printf("usage: d90tag <gpslog> <rawfile>+\n");
        return EXIT_FAILURE;
    }

    /* parse the gps log file */
    if((gpsf = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, "could not open gps log file: '%s'\n", argv[2]);
        return EXIT_FAILURE;
    }
    parse_nmea_file(gpsf, &rows, &num_rows);
    

    /* open and parse the image files */
    for(fnum=2; fnum<argc; ++fnum)
    {
        if((fp = fopen(argv[fnum], "rb+")) == NULL)
        {
            fprintf(stderr, "could not open raw file '%s'...skipping\n", argv[fnum]);
            continue;
        }

        if(!valid_tiff_file(fp))
        {
            fprintf(stderr, "error reading raw file '%s'; invalid tiff header...skipping\n",
                    argv[fnum]);
            continue;
        }

        /* find the offset of the first ifd */
        offset = read_uint(fp);
        fseek(fp, offset, SEEK_SET);
        
        ifd_load(fp, &zeroth_ifd);
        for(i=0; i<zeroth_ifd.count; ++i)
        {
            if(zeroth_ifd.dirs[i].tag == GPSInfoIFDPointer) /* GPS Info IFD pointer */
            {
                gps_offset = zeroth_ifd.dirs[i].uint_values[0];
                fseek(fp, gps_offset, SEEK_SET);
                ifd_load(fp, &gps_info_ifd);
            }
        }
        
        /* print out the time the image was captured */
        for(i=0; i<zeroth_ifd.count; ++i)
        {
            if(zeroth_ifd.dirs[i].tag == DateTimeOriginal)
            {
                struct tm t;
                unsigned int utc_time;
                waypoint_t* match;
                ifd_t gd;
                
                parse_datetime((const char*)zeroth_ifd.dirs[i].byte_values, &t);
                add_offset(&t, TIME_ZONE_OFFSET);
                utc_time = timegm(&t);

                match = find_location_at(rows, num_rows, utc_time, 3600);
                if(!match)
                {
                    printf("no match found within 1 hour of photo '%s'...skipping\n", argv[fnum]);
                    break;
                }
                
                /* now write the gps info ifd structure */
                gd.count = 7;
                gd.dirs = (direntry_t*)malloc(gd.count * sizeof(direntry_t));
                gd.next_offset = gps_info_ifd.next_offset;
                
                gd.dirs[0].tag = GPSVersionID;
                gd.dirs[0].type = BYTE;
                gd.dirs[0].count = 4;
                gd.dirs[0].byte_values = (unsigned char*)malloc(gd.dirs[0].count * sizeof(char));
                gd.dirs[0].byte_values[0] = 2;
                gd.dirs[0].byte_values[1] = 2;
                gd.dirs[0].byte_values[2] = 0;
                gd.dirs[0].byte_values[3] = 0;
                
                gd.dirs[1].tag = GPSLatitudeRef;
                gd.dirs[1].type = ASCII;
                gd.dirs[1].count = 2;
                gd.dirs[1].byte_values = (unsigned char*)malloc(gd.dirs[1].count);
                snprintf((char*)gd.dirs[1].byte_values, gd.dirs[1].count, "%c", match->rmc->lat_ref);
            
                gd.dirs[2].tag = GPSLatitude;
                gd.dirs[2].type = RATIONAL;
                gd.dirs[2].count = 3;
                gd.dirs[2].rational_values = (rational_t*)malloc(gd.dirs[2].count * sizeof(rational_t));
                gd.dirs[2].rational_values[0].numerator = (int)match->rmc->latitude / 100;
                gd.dirs[2].rational_values[0].denominator = 1;
                gd.dirs[2].rational_values[1].numerator = (int)match->rmc->latitude % 100;
                gd.dirs[2].rational_values[1].denominator = 1;
                gd.dirs[2].rational_values[2].numerator = (int)(match->rmc->latitude * 10000) % 10000;
                gd.dirs[2].rational_values[2].denominator = 100;
            
                gd.dirs[3].tag = GPSLongitudeRef;
                gd.dirs[3].type = ASCII;
                gd.dirs[3].count = 2;
                gd.dirs[3].byte_values = (unsigned char*)malloc(gd.dirs[3].count);
                snprintf((char*)gd.dirs[3].byte_values, gd.dirs[3].count, "%c", match->rmc->lon_ref);
            
                gd.dirs[4].tag = GPSLongitude;
                gd.dirs[4].type = RATIONAL;
                gd.dirs[4].count = 3;
                gd.dirs[4].rational_values = (rational_t*)malloc(gd.dirs[4].count * sizeof(rational_t));
                gd.dirs[4].rational_values[0].numerator = (int)match->rmc->longitude / 100;
                gd.dirs[4].rational_values[0].denominator = 1;
                gd.dirs[4].rational_values[1].numerator = (int)match->rmc->longitude % 100;
                gd.dirs[4].rational_values[1].denominator = 1;
                gd.dirs[4].rational_values[2].numerator = (int)(match->rmc->longitude * 10000) % 10000;
                gd.dirs[4].rational_values[2].denominator = 100;

                /* convert time from gps device back to struct tm format */
                gmtime_r(&(match->rmc->when), &t);
            
                gd.dirs[5].tag = GPSTimeStamp;
                gd.dirs[5].type = RATIONAL;
                gd.dirs[5].count = 3;
                gd.dirs[5].rational_values = (rational_t*)malloc(gd.dirs[5].count * sizeof(rational_t));
                gd.dirs[5].rational_values[0].numerator = t.tm_hour;
                gd.dirs[5].rational_values[0].denominator = 1;
                gd.dirs[5].rational_values[1].numerator = t.tm_min;
                gd.dirs[5].rational_values[1].denominator = 1;
                gd.dirs[5].rational_values[2].numerator = t.tm_sec;
                gd.dirs[5].rational_values[2].denominator = 1;
            
                gd.dirs[6].tag = GPSDateStamp;
                gd.dirs[6].type = ASCII;
                gd.dirs[6].count = strlen("yyyy:mm:dd")+1;
                gd.dirs[6].byte_values = (unsigned char*)malloc((gd.dirs[6].count+1) * sizeof(char));
                snprintf((char*)gd.dirs[6].byte_values, gd.dirs[6].count,
                         "%04d:%02d:%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);
                /* write the new gps information */
                assert(gps_offset > 0);

                fseek(fp, gps_offset, SEEK_SET);
                ifd_write(fp, &gd);

                printf("file '%s' geotagged successfully...\n", argv[fnum]);
                ifd_free(&gd);
                break;
            }
        }
        ifd_free(&zeroth_ifd);
        ifd_free(&gps_info_ifd);
        fclose(fp);
    }

    for(i=0; i<num_rows; ++i)
    {
        if(rows[i].record_type == GPRMC)
            free(rows[i].rmc);
    }
    free(rows);

    fclose(gpsf);
    return EXIT_SUCCESS;
}
