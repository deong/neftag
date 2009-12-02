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
    unsigned int j;
    FILE* fp;
    FILE* gpsf;
    ifd_t zeroth_ifd;
    ifd_t exif_ifd;
    ifd_t gps_info_ifd;
    ifd_t* sub_ifds = 0;
    unsigned int gps_offset = 0;
    int num_rows = 0;
    waypoint_t* rows = (waypoint_t*)malloc(1024 * sizeof(waypoint_t));
    
    if(argc != 3)
    {
        printf("usage: nefread <rawfile> <gpslog>\n");
        return EXIT_FAILURE;
    }

    /* parse the gps log file */
    if((gpsf = fopen(argv[2], "r")) == NULL)
    {
        fprintf(stderr, "could not open gps log file: '%s'\n", argv[2]);
        return EXIT_FAILURE;
    }
    parse_nmea_file(gpsf, &rows, &num_rows);
    

    /* open and parse the image files */
    if((fp = fopen(argv[1], "rb+")) == NULL)
    {
        fprintf(stderr, "could not open raw file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    if(!valid_tiff_file(fp))
    {
        fprintf(stderr, "error reading raw file '%s'; invalid tiff header\n",
                argv[1]);
        return EXIT_FAILURE;
    }

    /* find the offset of the first ifd */
    offset = read_uint(fp);
    fseek(fp, offset, SEEK_SET);

    ifd_load(fp, &zeroth_ifd);
    for(i=0; i<zeroth_ifd.count; ++i)
    {
        if(zeroth_ifd.dirs[i].tag == ExifIFDPointer) /* Exif IFD pointer */
        {
            offset = zeroth_ifd.dirs[i].uint_values[0];
            fseek(fp, offset, SEEK_SET);
            ifd_load(fp, &exif_ifd);
        }
        else if(zeroth_ifd.dirs[i].tag == GPSInfoIFDPointer) /* GPS Info IFD pointer */
        {
            gps_offset = zeroth_ifd.dirs[i].uint_values[0];
            fseek(fp, gps_offset, SEEK_SET);
            ifd_load(fp, &gps_info_ifd);
        }
        else if(zeroth_ifd.dirs[i].tag == SubIFDs)
        {
            int j;
            sub_ifds = (ifd_t*)malloc(zeroth_ifd.dirs[i].count * sizeof(ifd_t));
            for(j=0; j<zeroth_ifd.dirs[i].count; ++j)
            {
                offset = zeroth_ifd.dirs[i].uint_values[j];
                fseek(fp, offset, SEEK_SET);
                ifd_load(fp, &sub_ifds[j]);
            }
        }
    }

    /* print out the time the image was captured */
    for(i=0; i<zeroth_ifd.count; ++i)
    {
        if(zeroth_ifd.dirs[i].tag == DateTimeOriginal)
        {
            struct tm t;
            unsigned int utc_time;
            print_values(&zeroth_ifd.dirs[i]);
            parse_datetime((const char*)zeroth_ifd.dirs[i].byte_values, &t);
            printf("local time: %lu\n", timegm(&t));
            add_offset(&t, TIME_ZONE_OFFSET);
            utc_time = timegm(&t);
            printf("utc time:   %u\n", utc_time);
            for(j=0; j<num_rows; ++j)
            {
                if(rows[j].record_type == GPRMC &&
                   rows[j].rmc->when == utc_time)
                {
                    printf("match found. at (%ld), location was (%lf, %lf)\n",
                           rows[j].rmc->when, rows[j].rmc->latitude, rows[j].rmc->longitude);
                    break;
                }
            }

            /* now write the gps info ifd structure */
            gps_info_ifd.count = 7;
            gps_info_ifd.dirs = (direntry_t*)realloc(gps_info_ifd.dirs, gps_info_ifd.count * sizeof(direntry_t));

            gps_info_ifd.dirs[1].tag = GPSLatitudeRef;
            gps_info_ifd.dirs[1].type = ASCII;
            gps_info_ifd.dirs[1].count = 2;
            gps_info_ifd.dirs[1].byte_values = (unsigned char*)malloc(gps_info_ifd.dirs[1].count);
            snprintf((char*)gps_info_ifd.dirs[1].byte_values, gps_info_ifd.dirs[1].count, "%c", rows[j].rmc->lat_ref);
            
            gps_info_ifd.dirs[2].tag = GPSLatitude;
            gps_info_ifd.dirs[2].type = RATIONAL;
            gps_info_ifd.dirs[2].count = 3;
            gps_info_ifd.dirs[2].rational_values = (rational_t*)malloc(gps_info_ifd.dirs[2].count * sizeof(rational_t));
            gps_info_ifd.dirs[2].rational_values[0].numerator = (int)rows[j].rmc->latitude / 100;
            gps_info_ifd.dirs[2].rational_values[0].denominator = 1;
            gps_info_ifd.dirs[2].rational_values[1].numerator = (int)rows[j].rmc->latitude % 100;
            gps_info_ifd.dirs[2].rational_values[1].denominator = 1;
            gps_info_ifd.dirs[2].rational_values[2].numerator = (int)(rows[j].rmc->latitude * 10000) % 10000;
            gps_info_ifd.dirs[2].rational_values[2].denominator = 100;
            
            gps_info_ifd.dirs[3].tag = GPSLongitudeRef;
            gps_info_ifd.dirs[3].type = ASCII;
            gps_info_ifd.dirs[3].count = 2;
            gps_info_ifd.dirs[3].byte_values = (unsigned char*)malloc(gps_info_ifd.dirs[3].count);
            snprintf((char*)gps_info_ifd.dirs[3].byte_values, gps_info_ifd.dirs[3].count, "%c", rows[j].rmc->lon_ref);
            
            gps_info_ifd.dirs[4].tag = GPSLongitude;
            gps_info_ifd.dirs[4].type = RATIONAL;
            gps_info_ifd.dirs[4].count = 3;
            gps_info_ifd.dirs[4].rational_values = (rational_t*)malloc(gps_info_ifd.dirs[4].count * sizeof(rational_t));
            gps_info_ifd.dirs[4].rational_values[0].numerator = (int)rows[j].rmc->longitude / 100;
            gps_info_ifd.dirs[4].rational_values[0].denominator = 1;
            gps_info_ifd.dirs[4].rational_values[1].numerator = (int)rows[j].rmc->longitude % 100;
            gps_info_ifd.dirs[4].rational_values[1].denominator = 1;
            gps_info_ifd.dirs[4].rational_values[2].numerator = (int)(rows[j].rmc->longitude * 10000) % 10000;
            gps_info_ifd.dirs[4].rational_values[2].denominator = 100;

            /* convert time from gps device back to struct tm format */
            gmtime_r(&(rows[j].rmc->when), &t);
            
            gps_info_ifd.dirs[5].tag = GPSTimeStamp;
            gps_info_ifd.dirs[5].type = RATIONAL;
            gps_info_ifd.dirs[5].count = 3;
            gps_info_ifd.dirs[5].rational_values = (rational_t*)malloc(gps_info_ifd.dirs[5].count * sizeof(rational_t));
            gps_info_ifd.dirs[5].rational_values[0].numerator = t.tm_hour;
            gps_info_ifd.dirs[5].rational_values[0].denominator = 1;
            gps_info_ifd.dirs[5].rational_values[1].numerator = t.tm_min;
            gps_info_ifd.dirs[5].rational_values[1].denominator = 1;
            gps_info_ifd.dirs[5].rational_values[2].numerator = t.tm_sec;
            gps_info_ifd.dirs[5].rational_values[2].denominator = 1;
            
            gps_info_ifd.dirs[6].tag = GPSDateStamp;
            gps_info_ifd.dirs[6].type = ASCII;
            gps_info_ifd.dirs[6].count = strlen("yyyy:mm:dd")+1;
            gps_info_ifd.dirs[6].byte_values = (unsigned char*)malloc((gps_info_ifd.dirs[6].count+1) * sizeof(char));
            snprintf((char*)gps_info_ifd.dirs[6].byte_values, gps_info_ifd.dirs[6].count,
                     "%04d:%02d:%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);
            /* write the new gps information */
            assert(gps_offset > 0);

            fseek(fp, gps_offset, SEEK_SET);
            ifd_write(fp, &gps_info_ifd);
        }
    }
    

    if(sub_ifds)
        free(sub_ifds);
    for(i=0; i<num_rows; ++i)
    {
        if(rows[i].record_type == GPRMC)
            free(rows[i].rmc);
    }
    free(rows);
    
    fclose(fp);
    fclose(gpsf);
    
    return EXIT_SUCCESS;
}
