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
#include "types.h"

static void print_usage();

void print_usage()
{
    printf("usage: d90tag [-o utc_offset] [-w window_size] <gpslog> <rawfile>+\n\n"
           "\tutc_offset is specified as X where GMT=local+X,\n"
           "\te.g., CST is GMT-6, so to tag images taken in CST, specify\n"
           "\t-o6, not -o-6. (default: 0)\n\n"
           "\twindow_size sets maximum number of seconds that the GPS timestamp\n"
           "\tmay differ from the camera's timestamp and still be considered to\n"
           "\tmatch. (default 3600, e.g., one hour)\n\n");
}

int main(int argc, char** argv)
{
    unsigned int32 offset;
    unsigned int i;
    FILE* fp;
    FILE* gpsf;
    ifd_t ifd0;
    ifd_t gps_info_ifd;
    unsigned int32 gps_offset = 0;
    int num_rows = 0;
    int init_size = 1024;
    location_t* rows = (location_t*)malloc(init_size * sizeof(location_t));
    char ch;
    int tzoffset = 0;
    int window_size = 3600;

    /* sanity check the platform */
    assert(sizeof(byte) == 1);
    assert(sizeof(int16) == 2);
    assert(sizeof(int32) == 4);
    assert(sizeof(int64) == 8);
    assert(sizeof(float32) == 4);
    assert(sizeof(float64) == 8);
    
    if(argc < 3)
    {
        print_usage();
        return EXIT_FAILURE;
    }

    while((ch = getopt(argc, argv, "ho:w:")) != -1)
    {
        switch(ch)
        {
        case 'o': // time zone offset from UTC
            tzoffset = atoi(optarg);
            break;
        case 'w':
            window_size = atoi(optarg);
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
    parse_nmea_file(gpsf, &rows, &num_rows, init_size);
    fclose(gpsf);

    /* for each image file, open and parse the tiff headers */
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
        offset = read_uint32(fp);
        fseek(fp, offset, SEEK_SET);
        
        ifd_load(fp, &ifd0);
        for(i=0; i<ifd0.count; ++i)
        {
            if(ifd0.dirs[i].tag == GPSInfoIFDPointer) /* GPS Info IFD pointer */
            {
                gps_offset = ifd0.dirs[i].uint32_values[0];
                fseek(fp, gps_offset, SEEK_SET);
                ifd_load(fp, &gps_info_ifd);
            }
        }
        
        /* find the DateTimeOriginal header, and use the data to match a GPS location record */
        for(i=0; i<ifd0.count; ++i)
        {
            if(ifd0.dirs[i].tag == DateTimeOriginal)
            {
                /*
                 * basic algorithm is to pull the date/time from the image (in whatever time
                 * zone the camera is set to), convert it to utc, find the nearest GPS location
                 * record, populate a new GPSInfoIFD structure, and write it to the image
                 */
                struct tm t;
                unsigned int utc_time;
                location_t* match;
                ifd_t gd;
                
                parse_datetime((const char*)ifd0.dirs[i].byte_values, &t);
                add_offset(&t, tzoffset);
                utc_time = timegm(&t);

                match = find_location_at(rows, num_rows, utc_time, window_size);
                if(!match)
                {
                    printf("no match found within 1 hour of photo '%s'...skipping\n", argv[optind]);
                    break;
                }

                /* now fill the gps info ifd structure */
                populate_gps_info_ifd(&gd, match);
                gd.next_offset = gps_info_ifd.next_offset;
                
                /* write the new gps information */
                assert(gps_offset > 0);
                fseek(fp, gps_offset, SEEK_SET);
                ifd_write(fp, &gd);

                ifd_free(&gd);
                break;
            }
        }
        ifd_free(&ifd0);
        ifd_free(&gps_info_ifd);
        fclose(fp);
    }

    free(rows);
    return EXIT_SUCCESS;
}
