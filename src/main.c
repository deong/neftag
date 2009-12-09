/*
 * main.c
 * main program for dealing with d90 raw files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "tiff.h"
#include "util.h"
#include "csv.h"
#include "nmea.h"
#include "nikond90.h"
#include "date.h"

static void print_usage();

void print_usage()
{
    printf("usage: d90tag [-o utc_offset] <gpslog> <rawfile>+\n\n");
    printf("\tnote that utc_offset is specified as X where GMT=local+X,\n");
    printf("\te.g., CST is GMT-6, so to tag images taken in CST, specify\n");
    printf("\t-o6, not -o-6\n\n");
}

int main(int argc, char** argv)
{
    unsigned int offset;
    unsigned int i;
    FILE* fp;
    FILE* gpsf;
    ifd_t zeroth_ifd;
    ifd_t gps_info_ifd;
    unsigned int gps_offset = 0;
    int num_rows = 0;
    sentence_t* rows = (sentence_t*)malloc(1024 * sizeof(sentence_t));
    char ch;
    int tzoffset = 0;
    
    if(argc < 3)
    {
        print_usage();
        return EXIT_FAILURE;
    }

    while((ch = getopt(argc, argv, "ho:")) != -1)
    {
        switch(ch)
        {
        case 'o': // time zone offset from UTC
            tzoffset = atoi(optarg);
            break;
        case 'h':
            print_usage();
            return EXIT_SUCCESS;
        default:
            printf("unrecognized option: %c\n", ch);
            return EXIT_SUCCESS;
        }
    }

    /* parse the gps log file */
    if((gpsf = fopen(argv[optind], "r")) == NULL)
    {
        fprintf(stderr, "could not open gps log file: '%s'\n", argv[optind]);
        return EXIT_FAILURE;
    }
    parse_nmea_file(gpsf, &rows, &num_rows);
    

    /* open and parse the image files */
    for(++optind; optind<argc; ++optind)
    {
        if((fp = fopen(argv[optind], "rb+")) == NULL)
        {
            fprintf(stderr, "could not open raw file '%s'...skipping\n", argv[optind]);
            continue;
        }

        if(!valid_tiff_file(fp))
        {
            fprintf(stderr, "error reading raw file '%s'; invalid tiff header...skipping\n",
                    argv[optind]);
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
                sentence_t* match;
                ifd_t gd;
                int index = 0;
                
                parse_datetime((const char*)zeroth_ifd.dirs[i].byte_values, &t);
                add_offset(&t, tzoffset);
                utc_time = timegm(&t);

                match = find_location_at(rows, num_rows, utc_time, 3600);
                if(!match)
                {
                    printf("no match found within 1 hour of photo '%s'...skipping\n", argv[optind]);
                    break;
                }
                
                /* now write the gps info ifd structure */
                if(fabs(match->rmc->geoid_ht) > 1e-3)
                    gd.count = 10;
                else
                    gd.count = 7;
                gd.dirs = (direntry_t*)malloc(gd.count * sizeof(direntry_t));
                gd.next_offset = gps_info_ifd.next_offset;
                
                gd.dirs[index].tag = GPSVersionID;
                gd.dirs[index].type = BYTE;
                gd.dirs[index].count = 4;
                gd.dirs[index].byte_values = (unsigned char*)malloc(gd.dirs[index].count * sizeof(char));
                gd.dirs[index].byte_values[0] = 2;
                gd.dirs[index].byte_values[1] = 2;
                gd.dirs[index].byte_values[2] = 0;
                gd.dirs[index].byte_values[3] = 0;
                ++index;
                
                gd.dirs[index].tag = GPSLatitudeRef;
                gd.dirs[index].type = ASCII;
                gd.dirs[index].count = 2;
                gd.dirs[index].byte_values = (unsigned char*)malloc(gd.dirs[index].count);
                snprintf((char*)gd.dirs[index].byte_values, gd.dirs[index].count, "%c", match->rmc->lat_ref);
                ++index;
                
                gd.dirs[index].tag = GPSLatitude;
                gd.dirs[index].type = RATIONAL;
                gd.dirs[index].count = 3;
                gd.dirs[index].rational_values = (rational_t*)malloc(gd.dirs[index].count * sizeof(rational_t));
                gd.dirs[index].rational_values[0].numerator = (int)match->rmc->latitude / 100;
                gd.dirs[index].rational_values[0].denominator = 1;
                gd.dirs[index].rational_values[1].numerator = (int)match->rmc->latitude % 100;
                gd.dirs[index].rational_values[1].denominator = 1;
                gd.dirs[index].rational_values[2].numerator = (int)(match->rmc->latitude * 10000) % 10000;
                gd.dirs[index].rational_values[2].denominator = 100;
                ++index;
                
                gd.dirs[index].tag = GPSLongitudeRef;
                gd.dirs[index].type = ASCII;
                gd.dirs[index].count = 2;
                gd.dirs[index].byte_values = (unsigned char*)malloc(gd.dirs[index].count);
                snprintf((char*)gd.dirs[index].byte_values, gd.dirs[index].count, "%c", match->rmc->lon_ref);
                ++index;
                
                gd.dirs[index].tag = GPSLongitude;
                gd.dirs[index].type = RATIONAL;
                gd.dirs[index].count = 3;
                gd.dirs[index].rational_values = (rational_t*)malloc(gd.dirs[index].count * sizeof(rational_t));
                gd.dirs[index].rational_values[0].numerator = (int)match->rmc->longitude / 100;
                gd.dirs[index].rational_values[0].denominator = 1;
                gd.dirs[index].rational_values[1].numerator = (int)match->rmc->longitude % 100;
                gd.dirs[index].rational_values[1].denominator = 1;
                gd.dirs[index].rational_values[2].numerator = (int)(match->rmc->longitude * 10000) % 10000;
                gd.dirs[index].rational_values[2].denominator = 100;
                ++index;
                
                /* only report altitude if we have a geoid height */
                if(fabs(match->rmc->geoid_ht) > 1e-3)
                {
                    gd.dirs[index].tag = GPSAltitudeRef;
                    gd.dirs[index].type = BYTE;
                    gd.dirs[index].count = 1;
                    gd.dirs[index].byte_values = (unsigned char*)malloc(gd.dirs[index].count * sizeof(char));
                    gd.dirs[index].byte_values[0] = (match->rmc->altitude < 0) ? 1 : 0;
                    ++index;
                    
                    gd.dirs[index].tag = GPSAltitude;
                    gd.dirs[index].type = RATIONAL;
                    gd.dirs[index].count = 1;
                    gd.dirs[index].rational_values = (rational_t*)malloc(gd.dirs[index].count * sizeof(rational_t));
                    gd.dirs[index].rational_values[0].numerator = (int)(match->rmc->altitude * 10);
                    gd.dirs[index].rational_values[0].denominator = 10;
                    ++index;

                    gd.dirs[index].tag = GPSMapDatum;
                    gd.dirs[index].type = ASCII;
                    gd.dirs[index].count = strlen("WGS-84")+1;
                    gd.dirs[index].byte_values = (unsigned char*)malloc(gd.dirs[index].count * sizeof(char));
                    strncpy((char*)gd.dirs[index].byte_values, "WGS-84", gd.dirs[index].count);
                    ++index;
                }
                
                /* convert time from gps device back to struct tm format */
                gmtime_r(&(match->rmc->when), &t);
            
                gd.dirs[index].tag = GPSTimeStamp;
                gd.dirs[index].type = RATIONAL;
                gd.dirs[index].count = 3;
                gd.dirs[index].rational_values = (rational_t*)malloc(gd.dirs[index].count * sizeof(rational_t));
                gd.dirs[index].rational_values[0].numerator = t.tm_hour;
                gd.dirs[index].rational_values[0].denominator = 1;
                gd.dirs[index].rational_values[1].numerator = t.tm_min;
                gd.dirs[index].rational_values[1].denominator = 1;
                gd.dirs[index].rational_values[2].numerator = t.tm_sec;
                gd.dirs[index].rational_values[2].denominator = 1;
                ++index;
                
                gd.dirs[index].tag = GPSDateStamp;
                gd.dirs[index].type = ASCII;
                gd.dirs[index].count = strlen("yyyy:mm:dd")+1;
                gd.dirs[index].byte_values = (unsigned char*)malloc((gd.dirs[index].count+1) * sizeof(char));
                snprintf((char*)gd.dirs[index].byte_values, gd.dirs[index].count,
                         "%04d:%02d:%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);
                ++index;
                
                /* write the new gps information */
                assert(gps_offset > 0);
                fseek(fp, gps_offset, SEEK_SET);
                ifd_write(fp, &gd);

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
