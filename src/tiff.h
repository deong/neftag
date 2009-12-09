/*
 * tiff.h
 * basic constants for dealing with tiff files
 */

#include <time.h>
#include "nmea.h"

#ifndef _TIFF_H_
#define _TIFF_H_

/* TIFF header constants */
#define TIFF_BIG_ENDIAN 0x4d4d
#define TIFF_LITTLE_ENDIAN 0x4949
#define TIFF_MAGIC 42

/* TIFF data types */
#define BYTE 1
#define ASCII 2
#define SHORT 3
#define LONG 4
#define RATIONAL 5
#define SBYTE 6
#define UNDEFINED 7
#define SSHORT 8
#define SLONG 9
#define SRATIONAL 10
#define FLOAT 11
#define DOUBLE 12

extern unsigned int byte_order;
extern unsigned int type_bytes[13];

typedef struct
{
    unsigned int numerator;
    unsigned int denominator;
} rational_t;

typedef struct
{
    unsigned short tag;
    unsigned short type;
    unsigned int count;
    union
    {
        unsigned char* byte_values;
        char* sbyte_values;
        unsigned short* ushort_values;
        short* short_values;
        unsigned int* uint_values;
        int* int_values;
        float* float_values;
        double* double_values;
        rational_t* rational_values;
    };
} direntry_t;

typedef struct
{
    unsigned short count;
    direntry_t* dirs;
    unsigned int next_offset;
} ifd_t;

void ifd_load(FILE* f, ifd_t* ifd);
void ifd_free(ifd_t* ifd);
int valid_tiff_file(FILE* f);
void ifd_write(FILE* f, ifd_t* ifd);
void print_values(direntry_t* dir);
void parse_datetime(const char* dt, struct tm* t);
void populate_gps_info_ifd(ifd_t* ifd, location_t* match);

#endif
