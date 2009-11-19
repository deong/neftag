/*
 * tiff.c
 * basic functions for dealing with tiff files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiff.h"
#include "util.h"

/* size in bytes of each of the TIFF data types */
unsigned int type_bytes[13] = {-1, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

/* little or big endian */
unsigned int byte_order;


void ifd_load(FILE* f, ifd_t* ifd)
{
    /* must be called when the file pointer is at the beginning of the IFD */
    int i, j;
    memset(ifd, 0, sizeof(ifd_t));
    
    /* get the number of directory entries */
    ifd->count = read_ushort(f);
    ifd->dirs = (direntry_t*)malloc(ifd->count * sizeof(direntry_t));

    printf("ifd->count = %d\n\n", ifd->count);
    
    /* read each directory entry */
    for(i=0; i<ifd->count; ++i)
    {
        ifd->dirs[i].tag = read_ushort(f);
        ifd->dirs[i].type = read_ushort(f);
        ifd->dirs[i].count = read_uint(f);
        printf("\tifd->dirs[%d].tag = %x\n", i, ifd->dirs[i].tag);
        printf("\tifd->dirs[%d].type = %d\n", i, ifd->dirs[i].type);
        printf("\tifd->dirs[%d].count = %u\n", i, ifd->dirs[i].count);
        
        switch(ifd->dirs[i].type)
        {
        case BYTE:
        case ASCII:
        case UNDEFINED:
            ifd->dirs[i].byte_values = (unsigned char*)malloc(ifd->dirs[i].count * sizeof(char));
            break;
        case SBYTE:
            ifd->dirs[i].sbyte_values = (char*)malloc(ifd->dirs[i].count * sizeof(char));
            break;
        case SHORT:
            ifd->dirs[i].ushort_values = (unsigned short*)malloc(ifd->dirs[i].count * sizeof(short));
            break;
        case SSHORT:
            ifd->dirs[i].short_values = (short*)malloc(ifd->dirs[i].count * sizeof(short));
            break;
        case LONG:
            ifd->dirs[i].uint_values = (unsigned int*)malloc(ifd->dirs[i].count * sizeof(int));
            break;
        case SLONG:
            ifd->dirs[i].int_values = (int*)malloc(ifd->dirs[i].count * sizeof(int));
            break;
        case FLOAT:
            ifd->dirs[i].float_values = (float*)malloc(ifd->dirs[i].count * sizeof(float));
            break;
        case DOUBLE:
            ifd->dirs[i].double_values = (double*)malloc(ifd->dirs[i].count * sizeof(double));
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
                    ifd->dirs[i].ushort_values[j] = read_ushort(f);
                    break;
                case SSHORT:
                    ifd->dirs[i].short_values[j] = read_short(f);
                    break;
                case LONG:
                    ifd->dirs[i].uint_values[j] = read_uint(f);
                    break;
                case SLONG:
                    ifd->dirs[i].int_values[j] = read_int(f);
                    break;
                case FLOAT:
                    ifd->dirs[i].float_values[j] = read_float(f);
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
            unsigned int dataloc = read_uint(f);
            printf("\toffset = %x\n", dataloc);
            
            /* save the current offset */
            unsigned int cpos = ftell(f);

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
                    ifd->dirs[i].ushort_values[p] = read_ushort(f);
                    break;
                case SSHORT:
                    ifd->dirs[i].short_values[p] = read_short(f);
                    break;
                case LONG:
                    ifd->dirs[i].uint_values[p] = read_uint(f);
                    break;
                case SLONG:
                    ifd->dirs[i].int_values[p] = read_int(f);
                    break;
                case FLOAT:
                    ifd->dirs[i].float_values[p] = read_float(f);
                    break;
                case DOUBLE:
                    ifd->dirs[i].double_values[p] = read_double(f);
                    break;
                case RATIONAL:
                    ifd->dirs[i].rational_values[p].numerator = read_uint(f);
                    ifd->dirs[i].rational_values[p].denominator = read_uint(f);
                    break;
                }
            }

            /* and jump back */
            fseek(f, cpos, SEEK_SET);
        }

        printf("\tifd->dirs[i].values = ");
        for(j=0; j<ifd->dirs[i].count; ++j)
        {
            switch(ifd->dirs[i].type)
            {
            case BYTE:
            case ASCII:
            case UNDEFINED:
                printf("%u ", ifd->dirs[i].byte_values[j]);
                break;
            case SBYTE:
                printf("%d ", ifd->dirs[i].sbyte_values[j]);
                break;
            case SHORT:
                printf("%u ", ifd->dirs[i].ushort_values[j]);
                break;
            case SSHORT:
                printf("%d ", ifd->dirs[i].short_values[j]);
                break;
            case LONG:
                printf("%u ", ifd->dirs[i].uint_values[j]);
                break;
            case SLONG:
                printf("%d ", ifd->dirs[i].int_values[j]);
                break;
            case FLOAT:
                printf("%f ", ifd->dirs[i].float_values[j]);
                break;
            case DOUBLE:
                printf("%lf ", ifd->dirs[i].double_values[j]);
                break;
            case RATIONAL:
                printf("%u/%u ", ifd->dirs[i].rational_values[j].numerator,
                       ifd->dirs[i].rational_values[j].denominator);
                break;
            }
        }
        printf("\n\n");
    }
    
    /* read the next ifd offset */
    ifd->next_offset = read_uint(f);
}

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
            free(ifd->dirs[i].ushort_values);
            break;
        case SSHORT:
            free(ifd->dirs[i].short_values);
            break;
        case LONG:
            free(ifd->dirs[i].uint_values);
            break;
        case SLONG:
            free(ifd->dirs[i].int_values);
            break;
        case FLOAT:
            free(ifd->dirs[i].float_values);
            break;
        case DOUBLE:
            free(ifd->dirs[i].double_values);
            break;
        case RATIONAL:
            free(ifd->dirs[i].rational_values);
            break;
        }
    }
    free(ifd->dirs);
}

int valid_tiff_file(FILE* f)
{
    /* assume that file pointer is at offset 0 */
    unsigned short magic_number = 0;

    fread(&byte_order, 1, 2, f);
    magic_number = read_ushort(f);
    
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

void ifd_write(FILE* f, ifd_t* ifd)
{
    /* ifd is the GPSInfoIFD structure; file pointer of f must be positioned
     * to the location of the gps_info pointer
     */
    unsigned short i, j;
    unsigned int ifd_block_size;
    unsigned int ifd_value_offset;
    unsigned int value_bytes_written = 0;
    
    /* total block is a two byte header determining count of directory entries,
     * 12*n bytes for all n directories, and a 4 byte pointer to the next block */
    ifd_block_size = 2 + 12*ifd->count + 4;
    ifd_value_offset = ftell(f)+ifd_block_size;
    
    /* write the number of directory entries in the gps info section */
    write_ushort(f, ifd->count);
    
    /* write each directory */
    for(i=0; i<ifd->count; ++i)
    {
        write_ushort(f, ifd->dirs[i].tag);
        write_ushort(f, ifd->dirs[i].type);
        write_uint(f, ifd->dirs[i].count);

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
                    write_ushort(f, ifd->dirs[i].ushort_values[j]);
                    break;
                case SSHORT:
                    write_short(f, ifd->dirs[i].short_values[j]);
                    break;
                case LONG:
                    write_uint(f, ifd->dirs[i].uint_values[j]);
                    break;
                case SLONG:
                    write_int(f, ifd->dirs[i].int_values[j]);
                    break;
                case FLOAT:
                    write_float(f, ifd->dirs[i].float_values[j]);
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
                unsigned char c = 0;
                write_byte(f, c);
                ++bytes_written;
            }
            /* make sure we fill out the value offset field completely */
            for(; bytes_written<4; ++bytes_written)
            {
                unsigned char c = 0;
                write_byte(f, c);
            }
        }
        else
        {
            /* must write a pointer to where the data will be written */
            unsigned int cpos = ftell(f);
            unsigned int pos = ifd_value_offset + value_bytes_written;
            write_uint(f, pos);

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
                    write_ushort(f, ifd->dirs[i].ushort_values[j]);
                    break;
                case SSHORT:
                    write_short(f, ifd->dirs[i].short_values[j]);
                    break;
                case LONG:
                    write_uint(f, ifd->dirs[i].uint_values[j]);
                    break;
                case SLONG:
                    write_int(f, ifd->dirs[i].int_values[j]);
                    break;
                case FLOAT:
                    write_float(f, ifd->dirs[i].float_values[j]);
                    break;
                case DOUBLE:
                    write_double(f, ifd->dirs[i].double_values[j]);
                    break;
                case RATIONAL:
                    write_uint(f, ifd->dirs[i].rational_values[j].numerator);
                    write_uint(f, ifd->dirs[i].rational_values[j].denominator);
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
                unsigned char c = 0;
                write_byte(f, c);
                ++value_bytes_written;
            }

            /* now jump back to where we were in the file */
            fseek(f, cpos, SEEK_SET);
        }
    }

    /* and finally, write the offset to the next ifd */
    write_uint(f, ifd->next_offset);
}

        
