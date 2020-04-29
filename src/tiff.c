/*
 * tiff.c
 * basic functions for dealing with tiff files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "tiff.h"
#include "util.h"
#include "csv.h"
#include "date.h"
#include "nmea.h"
#include "nikond90.h"

/* size in bytes of each of the TIFF data types */
unsigned int type_bytes[13] = {-1, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

/* little or big endian */
unsigned int byte_order;

/*
 * load an ifd_t from a given tiff file
 * reads all the direntry_t blocks and sets up the next_offset pointer
 * for the block
 */
void ifd_load(FILE* f, ifd_t* ifd)
{
    /* must be called when the file pointer is at the beginning of the IFD */
    int i, j;
    memset(ifd, 0, sizeof(ifd_t));
    
    /* get the number of directory entries */
    ifd->count = read_uint16(f);
    ifd->dirs = (direntry_t*)malloc(ifd->count * sizeof(direntry_t));
    
    /* read each directory entry */
    for(i=0; i<ifd->count; ++i)
    {
        ifd->dirs[i].tag = read_uint16(f);
        ifd->dirs[i].type = read_uint16(f);
        ifd->dirs[i].count = read_uint32(f);
        switch(ifd->dirs[i].type)
        {
        case BYTE:
        case ASCII:
        case UNDEFINED:
            ifd->dirs[i].byte_values = (unsigned byte*)malloc(ifd->dirs[i].count * sizeof(byte));
            break;
        case SBYTE:
            ifd->dirs[i].sbyte_values = (byte*)malloc(ifd->dirs[i].count * sizeof(byte));
            break;
        case SHORT:
            ifd->dirs[i].uint16_values = (unsigned int16*)malloc(ifd->dirs[i].count * sizeof(int16));
            break;
        case SSHORT:
            ifd->dirs[i].int16_values = (int16*)malloc(ifd->dirs[i].count * sizeof(int16));
            break;
        case LONG:
            ifd->dirs[i].uint32_values = (unsigned int32*)malloc(ifd->dirs[i].count * sizeof(int32));
            break;
        case SLONG:
            ifd->dirs[i].int32_values = (int32*)malloc(ifd->dirs[i].count * sizeof(int32));
            break;
        case FLOAT:
            ifd->dirs[i].float32_values = (float32*)malloc(ifd->dirs[i].count * sizeof(float32));
            break;
        case DOUBLE:
            ifd->dirs[i].float64_values = (float64*)malloc(ifd->dirs[i].count * sizeof(float64));
            break;
        case RATIONAL:
            ifd->dirs[i].rational_values = (rational_t*)malloc(ifd->dirs[i].count * sizeof(rational_t));
            break;
        }

        if(ifd->dirs[i].count * type_bytes[ifd->dirs[i].type] <= 4)
        {
            /* value fits entirely in the 4 byte value offset field */
            unsigned int bytes_read = 0;
            for(j=0; j<ifd->dirs[i].count; ++j)
            {
                bytes_read+=type_bytes[ifd->dirs[i].type];
                switch(ifd->dirs[i].type)
                {
                case BYTE:
                case ASCII:
                case UNDEFINED:
                    ifd->dirs[i].byte_values[j] = read_byte(f);
                    break;
                case SBYTE:
                    ifd->dirs[i].sbyte_values[j] = read_byte(f);
                    break;
                case SHORT:
                    ifd->dirs[i].uint16_values[j] = read_uint16(f);
                    break;
                case SSHORT:
                    ifd->dirs[i].int16_values[j] = read_int16(f);
                    break;
                case LONG:
                    ifd->dirs[i].uint32_values[j] = read_uint32(f);
                    break;
                case SLONG:
                    ifd->dirs[i].int32_values[j] = read_int32(f);
                    break;
                case FLOAT:
                    ifd->dirs[i].float32_values[j] = read_float32(f);
                    break;
                default:
                    fprintf(stderr, "can't fit specified type '%d' into value offset field\n",
                            ifd->dirs[i].type);
                    break;
                }
            }
            for(; bytes_read<4; ++bytes_read)
            {
                read_byte(f);
            }
        }
        else
        {
            int p;
            
            /* read the offset where the data is stored */
            unsigned int32 dataloc = read_uint32(f);
            
            /* save the current offset */
            unsigned int32 cpos = ftell(f);

            /* jump to the data */
            fseek(f, dataloc, SEEK_SET);

            /* read it */
            for(p=0; p<ifd->dirs[i].count; ++p)
            {
                switch(ifd->dirs[i].type)
                {
                case BYTE:
                case ASCII:
                case UNDEFINED:
                    ifd->dirs[i].byte_values[p] = read_byte(f);
                    break;
                case SBYTE:
                    ifd->dirs[i].sbyte_values[p] = read_byte(f);
                    break;
                case SHORT:
                    ifd->dirs[i].uint16_values[p] = read_uint16(f);
                    break;
                case SSHORT:
                    ifd->dirs[i].int16_values[p] = read_int16(f);
                    break;
                case LONG:
                    ifd->dirs[i].uint32_values[p] = read_uint32(f);
                    break;
                case SLONG:
                    ifd->dirs[i].int32_values[p] = read_int32(f);
                    break;
                case FLOAT:
                    ifd->dirs[i].float32_values[p] = read_float32(f);
                    break;
                case DOUBLE:
                    ifd->dirs[i].float64_values[p] = read_float64(f);
                    break;
                case RATIONAL:
                    ifd->dirs[i].rational_values[p].numerator = read_uint32(f);
                    ifd->dirs[i].rational_values[p].denominator = read_uint32(f);
                    break;
                }
            }

            /* and jump back */
            fseek(f, cpos, SEEK_SET);
        }
    }
    
    /* read the next ifd offset */
    ifd->next_offset = read_uint32(f);
}

/*
 * free any allocated memory inside an ifd_t
 */
void ifd_free(ifd_t* ifd)
{
    int i;
    for(i=0; i<ifd->count; ++i)
    {
        switch(ifd->dirs[i].type)
        {
        case BYTE:
        case ASCII:
        case UNDEFINED:
            free(ifd->dirs[i].byte_values);
            break;
        case SBYTE:
            free(ifd->dirs[i].sbyte_values);
            break;
        case SHORT:
            free(ifd->dirs[i].uint16_values);
            break;
        case SSHORT:
            free(ifd->dirs[i].int16_values);
            break;
        case LONG:
            free(ifd->dirs[i].uint32_values);
            break;
        case SLONG:
            free(ifd->dirs[i].int32_values);
            break;
        case FLOAT:
            free(ifd->dirs[i].float32_values);
            break;
        case DOUBLE:
            free(ifd->dirs[i].float64_values);
            break;
        case RATIONAL:
            free(ifd->dirs[i].rational_values);
            break;
        }
    }
    free(ifd->dirs);
}

/*
 * check the magic bytes at the beginning of the file to make sure it's
 * really a tiff file, and set the byte ordering in use in the file
 */
int valid_tiff_file(FILE* f)
{
    /* assume that file pointer is at offset 0 */
    unsigned int16 magic_number = 0;

    fread(&byte_order, 1, 2, f);
    magic_number = read_uint16(f);
    
    if(byte_order != TIFF_LITTLE_ENDIAN && byte_order != TIFF_BIG_ENDIAN)
    {
        fprintf(stderr, "invalid byte ordering %x\n", byte_order);
        return 0;
    }
    if(magic_number != TIFF_MAGIC)
    {
        fprintf(stderr, "magic number %d (%x) not valid for tiff file\n",
                magic_number, magic_number);
        return 0;
    }

    return 1;
}

/*
 * write a new ifd_t block into the given tiff file
 */
void ifd_write(FILE* f, ifd_t* ifd)
{
    /* ifd is the GPSInfoIFD structure; file pointer of f must be positioned
     * to the location of the gps_info pointer
     */
    unsigned int16 i, j;
    unsigned int32 ifd_block_size;
    unsigned int32 ifd_value_offset;
    unsigned int value_bytes_written = 0;
    
    /* total block is a two byte header determining count of directory entries,
     * 12*n bytes for all n directories, and a 4 byte pointer to the next block */
    ifd_block_size = 2 + 12*ifd->count + 4;
    ifd_value_offset = ftell(f)+ifd_block_size;
    
    /* write the number of directory entries in the gps info section */
    write_uint16(f, ifd->count);
    
    /* write each directory */
    for(i=0; i<ifd->count; ++i)
    {
        write_uint16(f, ifd->dirs[i].tag);
        write_uint16(f, ifd->dirs[i].type);
        write_uint32(f, ifd->dirs[i].count);

        if(ifd->dirs[i].count * type_bytes[ifd->dirs[i].type] <= 4)
        {
            /* can write the data directly into the value offset field */
            unsigned int bytes_written = 0;
            for(j=0; j<ifd->dirs[i].count; ++j)
            {
                bytes_written+=type_bytes[ifd->dirs[i].type];
                switch(ifd->dirs[i].type)
                {
                case BYTE:
                case ASCII:
                case UNDEFINED:
                    write_byte(f, ifd->dirs[i].byte_values[j]);
                    break;
                case SBYTE:
                    write_sbyte(f, ifd->dirs[i].sbyte_values[j]);
                    break;
                case SHORT:
                    write_uint16(f, ifd->dirs[i].uint16_values[j]);
                    break;
                case SSHORT:
                    write_int16(f, ifd->dirs[i].int16_values[j]);
                    break;
                case LONG:
                    write_uint32(f, ifd->dirs[i].uint32_values[j]);
                    break;
                case SLONG:
                    write_int32(f, ifd->dirs[i].int32_values[j]);
                    break;
                case FLOAT:
                    write_float32(f, ifd->dirs[i].float32_values[j]);
                    break;
                default:
                    fprintf(stderr, "attempt to write impossible type '%d' into "
                            "value offset field\n", ifd->dirs[i].type);
                    break;
                }
            }
            /* make sure we have written an even number of bytes */
            if(bytes_written % 2 == 1)
            {
                unsigned byte c = 0;
                write_byte(f, c);
                ++bytes_written;
            }
            /* make sure we fill out the value offset field completely */
            for(; bytes_written<4; ++bytes_written)
            {
                unsigned byte c = 0;
                write_byte(f, c);
            }
        }
        else
        {
            /* must write a pointer to where the data will be written */
            unsigned int32 cpos;
            unsigned int32 pos = ifd_value_offset + value_bytes_written;
            write_uint32(f, pos);
            cpos = ftell(f);

            /* now go to that location and write the data */
            fseek(f, pos, SEEK_SET);
            for(j=0; j<ifd->dirs[i].count; ++j)
            {
                value_bytes_written+=type_bytes[ifd->dirs[i].type];
                switch(ifd->dirs[i].type)
                {
                case BYTE:
                case ASCII:
                case UNDEFINED:
                    write_byte(f, ifd->dirs[i].byte_values[j]);
                    break;
                case SBYTE:
                    write_sbyte(f, ifd->dirs[i].sbyte_values[j]);
                    break;
                case SHORT:
                    write_uint16(f, ifd->dirs[i].uint16_values[j]);
                    break;
                case SSHORT:
                    write_int16(f, ifd->dirs[i].int16_values[j]);
                    break;
                case LONG:
                    write_uint32(f, ifd->dirs[i].uint32_values[j]);
                    break;
                case SLONG:
                    write_int32(f, ifd->dirs[i].int32_values[j]);
                    break;
                case FLOAT:
                    write_float32(f, ifd->dirs[i].float32_values[j]);
                    break;
                case DOUBLE:
                    write_float64(f, ifd->dirs[i].float64_values[j]);
                    break;
                case RATIONAL:
                    write_uint32(f, ifd->dirs[i].rational_values[j].numerator);
                    write_uint32(f, ifd->dirs[i].rational_values[j].denominator);
                    break;
                default:
                    fprintf(stderr, "attempt to write impossible type '%d' into "
                            "value offset field\n", ifd->dirs[i].type);
                    break;
                }
            }
            /* make sure we have written an even number of bytes */
            if(value_bytes_written % 2 == 1)
            {
                unsigned byte c = 0;
                write_byte(f, c);
                ++value_bytes_written;
            }

            /* now jump back to where we were in the file */
            fseek(f, cpos, SEEK_SET);
        }
    }

    /* and finally, write the offset to the next ifd */
    write_uint32(f, ifd->next_offset);
}

/*
 * debugging aid to print out the information in a particular direntry_t
 */
void print_values(direntry_t* dir)
{
    int i;
    
    printf("tag:    %x\n", dir->tag);
    printf("type:   %d\n", dir->type);
    printf("count:  %d\n", dir->count);
    printf("values:");
    for(i=0; i<dir->count; ++i)
    {
        switch(dir->type)
        {
        case BYTE:
        case ASCII:
        case SBYTE:
        case UNDEFINED:
            if(i==0)
                printf(" ");
            printf("%c", dir->byte_values[i]);
            break;
        case SHORT:
            printf(" %hu", dir->uint16_values[i]);
            break;
        case SSHORT:
            printf(" %hd", dir->int16_values[i]);
            break;
        case LONG:
            printf(" %u", dir->uint32_values[i]);
            break;
        case SLONG:
            printf(" %d", dir->int32_values[i]);
            break;
        case FLOAT:
            printf(" %f", dir->float32_values[i]);
            break;
        case DOUBLE:
            printf(" %lf", dir->float64_values[i]);
            break;
        case RATIONAL:
            printf(" %u/%u", dir->rational_values[i].numerator,
                dir->rational_values[i].denominator);
            break;
        default:
            fprintf(stderr, "attempt to print invalid type '%d'\n", dir->type);
            break;
        }
    }
    printf("\n");
}

/*
 * parse the weird date-time format from tiff file to normal
 * time structure
 */
void parse_datetime(const char* dt, struct tm* t)
{
    char* tmp = (char*)malloc(strlen(dt));
    char** toks;
    const int NUM_DT_TOKENS = 6;
    const int MAX_DT_TOKEN_LEN = 4;
    int i;
    
    /* make a backup copy of the string */
    strncpy(tmp, dt, strlen(dt));

    /* init some memory to use to parse the date string */
    toks = (char**)malloc(NUM_DT_TOKENS * sizeof(char*));
    for(i=0; i<NUM_DT_TOKENS; ++i)
    {
        toks[i] = (char*)malloc((MAX_DT_TOKEN_LEN+1) * sizeof(char));
        //*toks[i] = '\0';
    }

    /* parse the line */
    parse_line(tmp, ": ", toks, NUM_DT_TOKENS);

    t->tm_year = atoi(toks[0]) - 1900;
    t->tm_mon = atoi(toks[1]) - 1;
    t->tm_mday = atoi(toks[2]);
    t->tm_hour = atoi(toks[3]);
    t->tm_min = atoi(toks[4]);
    t->tm_sec = atoi(toks[5]);
}

/*
 * given an ifd_t structure for the gps information and a location_t
 * structure recorded from the gps logger, populate the useful fields
 * to write the gps info into the tiff headers
 */
void populate_gps_info_ifd(ifd_t* ifd, location_t* match)
{
    int index = 0;
    struct tm t;
    int deg;
    int min;
    double sec;
    
    /* now fill the gps info ifd structure */
    if(fabs(match->geoid_ht) > 1e-3)
        ifd->count = 10;
    else
        ifd->count = 7;
    ifd->dirs = (direntry_t*)malloc(ifd->count * sizeof(direntry_t));

    /* hard coded version id */
    ifd->dirs[index].tag = GPSVersionID;
    ifd->dirs[index].type = BYTE;
    ifd->dirs[index].count = 4;
    ifd->dirs[index].byte_values = (unsigned byte*)malloc(ifd->dirs[index].count * sizeof(byte));
    ifd->dirs[index].byte_values[0] = 2;
    ifd->dirs[index].byte_values[1] = 2;
    ifd->dirs[index].byte_values[2] = 0;
    ifd->dirs[index].byte_values[3] = 0;
    ++index;

    /* latitude ref is "N" or "S" */
    ifd->dirs[index].tag = GPSLatitudeRef;
    ifd->dirs[index].type = ASCII;
    ifd->dirs[index].count = 2;
    ifd->dirs[index].byte_values = (unsigned byte*)malloc(ifd->dirs[index].count);
    snprintf((char*)ifd->dirs[index].byte_values, ifd->dirs[index].count, "%c", match->lat_ref);
    ++index;

    /* latitudes recorded in degrees, minutes, seconds as a triple of rational numbers */
    dec2dms(match->latitude, &deg, &min, &sec);
    ifd->dirs[index].tag = GPSLatitude;
    ifd->dirs[index].type = RATIONAL;
    ifd->dirs[index].count = 3;
    ifd->dirs[index].rational_values = (rational_t*)malloc(ifd->dirs[index].count * sizeof(rational_t));
    ifd->dirs[index].rational_values[0].numerator = deg;
    ifd->dirs[index].rational_values[0].denominator = 1;
    ifd->dirs[index].rational_values[1].numerator = min;
    ifd->dirs[index].rational_values[1].denominator = 1;
    ifd->dirs[index].rational_values[2].numerator = (int32)floor(sec * 100);
    ifd->dirs[index].rational_values[2].denominator = 100;
    ++index;

    /* longitude ref is "E" or "W" */
    ifd->dirs[index].tag = GPSLongitudeRef;
    ifd->dirs[index].type = ASCII;
    ifd->dirs[index].count = 2;
    ifd->dirs[index].byte_values = (unsigned byte*)malloc(ifd->dirs[index].count);
    snprintf((char*)ifd->dirs[index].byte_values, ifd->dirs[index].count, "%c", match->lon_ref);
    ++index;

    /* longitude recorded in degrees, minutes, seconds as a triple of rational numbers */
    dec2dms(match->longitude, &deg, &min, &sec);
    ifd->dirs[index].tag = GPSLongitude;
    ifd->dirs[index].type = RATIONAL;
    ifd->dirs[index].count = 3;
    ifd->dirs[index].rational_values = (rational_t*)malloc(ifd->dirs[index].count * sizeof(rational_t));
    ifd->dirs[index].rational_values[0].numerator = deg;
    ifd->dirs[index].rational_values[0].denominator = 1;
    ifd->dirs[index].rational_values[1].numerator = min;
    ifd->dirs[index].rational_values[1].denominator = 1;
    ifd->dirs[index].rational_values[2].numerator = (int32)floor(sec * 100);
    ifd->dirs[index].rational_values[2].denominator = 100;
    ++index;
                
    /* only report altitude if we have a geoid height
     * 
     * geoid height is the correction required to get accurate MSL (mean sea level)
     * altitude information. For my SiRF device at least, it appears that the altitude
     * field is already updated with the correction, so as long as the GPS receiver
     * had a good enough fix to record a geoid height, then the raw altitude readout
     * should be accurate. On some devices, you may have to record the altitude as
     * "altitude - geoid_height".
     */
    if(fabs(match->geoid_ht) > 1e-3)
    {
        /* altitude ref is different than other ref fields.  it's a single byte that's
           0 for "above sea level" and 1 for "below sea level" */
        ifd->dirs[index].tag = GPSAltitudeRef;
        ifd->dirs[index].type = BYTE;
        ifd->dirs[index].count = 1;
        ifd->dirs[index].byte_values = (unsigned byte*)malloc(ifd->dirs[index].count * sizeof(byte));
        ifd->dirs[index].byte_values[0] = (match->altitude < 0) ? 1 : 0;
        ++index;

        /* altitude recorded as a rational in meters */
        ifd->dirs[index].tag = GPSAltitude;
        ifd->dirs[index].type = RATIONAL;
        ifd->dirs[index].count = 1;
        ifd->dirs[index].rational_values = (rational_t*)malloc(ifd->dirs[index].count * sizeof(rational_t));
        ifd->dirs[index].rational_values[0].numerator = (int32)(match->altitude * 10);
        ifd->dirs[index].rational_values[0].denominator = 10;
        ++index;

        /* geoidesic specification is WGS-84 */
        ifd->dirs[index].tag = GPSMapDatum;
        ifd->dirs[index].type = ASCII;
        ifd->dirs[index].count = strlen("WGS-84")+1;
        ifd->dirs[index].byte_values = (unsigned byte*)malloc(ifd->dirs[index].count * sizeof(byte));
        strncpy((char*)ifd->dirs[index].byte_values, "WGS-84", ifd->dirs[index].count);
        ++index;
    }
                
    /* convert time from gps device back to struct tm format
       so we can write accurate GPS timestamp information */
    gmtime_r(&(match->when), &t);

    /* again, time stored as three rationals for h/m/s */
    ifd->dirs[index].tag = GPSTimeStamp;
    ifd->dirs[index].type = RATIONAL;
    ifd->dirs[index].count = 3;
    ifd->dirs[index].rational_values = (rational_t*)malloc(ifd->dirs[index].count * sizeof(rational_t));
    ifd->dirs[index].rational_values[0].numerator = t.tm_hour;
    ifd->dirs[index].rational_values[0].denominator = 1;
    ifd->dirs[index].rational_values[1].numerator = t.tm_min;
    ifd->dirs[index].rational_values[1].denominator = 1;
    ifd->dirs[index].rational_values[2].numerator = t.tm_sec;
    ifd->dirs[index].rational_values[2].denominator = 1;
    ++index;

    /* date, on the other hand, is stored as an ascii string */
    ifd->dirs[index].tag = GPSDateStamp;
    ifd->dirs[index].type = ASCII;
    ifd->dirs[index].count = strlen("yyyy:mm:dd")+1;
    ifd->dirs[index].byte_values = (unsigned byte*)malloc((ifd->dirs[index].count+1) * sizeof(byte));
    snprintf((char*)ifd->dirs[index].byte_values, ifd->dirs[index].count,
             "%04d:%02d:%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);
    ++index;
}
