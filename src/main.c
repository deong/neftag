/*
 * main.c
 * main program for dealing with d90 raw files
 */

#include <stdio.h>
#include <stdlib.h>
#include "tiff.h"
#include "util.h"
#include "nikond90.h"

int main(int argc, char** argv)
{
    unsigned int offset;
    unsigned int i;
    FILE* fp;
    ifd_t zeroth_ifd;
    ifd_t exif_ifd;
    ifd_t gps_info_ifd;
    ifd_t* sub_ifds;
    unsigned int gps_offset;
    
    if(argc != 2)
    {
        printf("usage: nefread <rawfile>\n");
        return EXIT_FAILURE;
    }
    
    if((fp = fopen(argv[1], "rb")) == NULL)
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

    printf("loading 0thIFD\n");
    ifd_load(fp, &zeroth_ifd);

    for(i=0; i<zeroth_ifd.count; ++i)
    {
        if(zeroth_ifd.dirs[i].tag == ExifIFDPointer) /* Exif IFD pointer */
        {
            printf("loading ExifIFD\n");
            offset = zeroth_ifd.dirs[i].uint_values[0];
            fseek(fp, offset, SEEK_SET);
            ifd_load(fp, &exif_ifd);
        }
        else if(zeroth_ifd.dirs[i].tag == GPSInfoIFDPointer) /* GPS Info IFD pointer */
        {
            printf("loading GPSInfoIFD\n");
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
                printf("loading SubIFD%d\n", j);
                offset = zeroth_ifd.dirs[i].uint_values[j];
                fseek(fp, offset, SEEK_SET);
                ifd_load(fp, &sub_ifds[j]);
            }
        }
    }

    ifd_free(&zeroth_ifd);
    ifd_free(&exif_ifd);
    ifd_free(&gps_info_ifd);
    
    fclose(fp);
    return 0;
}
