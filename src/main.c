/*
 * main.c
 * main program for dealing with d90 raw files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tiff.h"
#include "util.h"
#include "csv.h"
#include "nmea.h"
#include "nikond90.h"

int main(int argc, char** argv)
{
    unsigned int offset;
    unsigned int i;
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

    assert(gps_offset > 0);

    if(sub_ifds)
        free(sub_ifds);
    for(i=0; i<num_rows; ++i)
    {
        if(rows[i].record_type == GPGGA)
            free(rows[i].gga);
        else if(rows[i].record_type == GPRMC)
            free(rows[i].rmc);
    }
    free(rows);
    
    fclose(fp);
    fclose(gpsf);
    
    return EXIT_SUCCESS;
}
