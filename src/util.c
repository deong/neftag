/*
 * util.c
 * utility functions for dealing with tiff files
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "tiff.h"
#include "types.h"

unsigned byte read_byte(FILE* f)
{
    unsigned byte b;
    fread(&b, sizeof(byte), 1, f);
    return b;
}

void write_byte(FILE* f, unsigned byte b)
{
    fwrite(&b, sizeof(byte), 1, f);
}

byte read_sbyte(FILE* f)
{
    byte b;
    fread(&b, sizeof(byte), 1, f);
    return b;
}

void write_sbyte(FILE* f, byte b)
{
    fwrite(&b, sizeof(byte), 1, f);
}

int16 read_int16(FILE* f)
{
    int16 tmp;
    fread(&tmp, sizeof(int16), 1, f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2((unsigned int16*)&tmp);
    }
    return tmp;
}

void write_int16(FILE* f, int16 k)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2((unsigned int16*)&k);
    }
    fwrite(&k, sizeof(int16), 1, f);
}

unsigned int16 read_uint16(FILE* f)
{
    unsigned int16 tmp;
    fread(&tmp, sizeof(int16), 1, f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2(&tmp);
    }
    return tmp;
}

void write_uint16(FILE* f, unsigned int16 k)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2(&k);
    }
    fwrite(&k, sizeof(unsigned int16), 1, f);
}

unsigned int32 read_uint32(FILE* f)
{
    unsigned int32 tmp;
    fread(&tmp, sizeof(int32), 1, f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4(&tmp);
    }
    return tmp;
}

void write_uint32(FILE* f, unsigned int32 n)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4(&n);
    }
    fwrite(&n, sizeof(unsigned int32), 1, f);
}

int32 read_int32(FILE* f)
{
    int32 tmp;
    fread(&tmp, sizeof(int32), 1, f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int32*)&tmp);
    }
    return tmp;
}

void write_int32(FILE* f, int32 n)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int32*)&n);
    }
    fwrite(&n, sizeof(int32), 1, f);
}

float32 read_float32(FILE* f)
{
    float32 tmp;
    fread(&tmp, 1, sizeof(float32), f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int32*)&tmp);
    }
    return tmp;
}

void write_float32(FILE* f, float32 x)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int32*)&x);
    }
    fwrite(&x, sizeof(float32), 1, f);
}

float64 read_float64(FILE* f)
{
    float64 tmp;
    fread(&tmp, 1, sizeof(float64), f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian8((unsigned int64*)&tmp);
    }
    return tmp;
}

void write_float64(FILE* f, float64 x)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian8((unsigned int64*)&x);
    }
    fwrite(&x, sizeof(float64), 1, f);
}

void swap_endian2(unsigned int16* x)
{
    *x = (*x>>8) | (*x<<8);
}

void swap_endian4(unsigned int32* x)
{
    *x = (*x>>24) | 
        ((*x<<8) & 0x00FF0000) |
        ((*x>>8) & 0x0000FF00) |
        (*x<<24);
}

void swap_endian8(unsigned int64* x)
{
    *x = (*x>>56) | 
        ((*x<<40) & 0x00FF000000000000) |
        ((*x<<24) & 0x0000FF0000000000) |
        ((*x<<8)  & 0x000000FF00000000) |
        ((*x>>8)  & 0x00000000FF000000) |
        ((*x>>24) & 0x0000000000FF0000) |
        ((*x>>40) & 0x000000000000FF00) |
        (*x<<56);
}
