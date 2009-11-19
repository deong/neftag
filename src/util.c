/*
 * util.c
 * utility functions for dealing with tiff files
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "tiff.h"

unsigned char read_byte(FILE* f)
{
    unsigned char b;
    fread(&b, sizeof(char), 1, f);
    return b;
}

void write_byte(FILE* f, unsigned char b)
{
    fwrite(&b, sizeof(char), 1, f);
}

char read_sbyte(FILE* f)
{
    char b;
    fread(&b, sizeof(char), 1, f);
    return b;
}

void write_sbyte(FILE* f, char b)
{
    fwrite(&b, sizeof(char), 1, f);
}

unsigned int read_uint(FILE* f)
{
    unsigned int tmp;
    fread(&tmp, sizeof(int), 1, f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4(&tmp);
    }
    return tmp;
}

void write_uint(FILE* f, unsigned int n)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4(&n);
    }
    fwrite(&n, sizeof(unsigned int), 1, f);
}

unsigned short read_ushort(FILE* f)
{
    unsigned short tmp;
    fread(&tmp, sizeof(short), 1, f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2(&tmp);
    }
    return tmp;
}

void write_ushort(FILE* f, unsigned short k)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2(&k);
    }
    fwrite(&k, sizeof(unsigned short), 1, f);
}

int read_int(FILE* f)
{
    int tmp;
    fread(&tmp, sizeof(int), 1, f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int*)&tmp);
    }
    return tmp;
}

void write_int(FILE* f, int n)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int*)&n);
    }
    fwrite(&n, sizeof(int), 1, f);
}

short read_short(FILE* f)
{
    short tmp;
    fread(&tmp, 1, sizeof(short), f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2((unsigned short*)&tmp);
    }
    return tmp;
}

void write_short(FILE* f, short k)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian2((unsigned short*)&k);
    }
    fwrite(&k, sizeof(short), 1, f);
}

float read_float(FILE* f)
{
    float tmp;
    fread(&tmp, 1, sizeof(float), f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int*)&tmp);
    }
    return tmp;
}

void write_float(FILE* f, float x)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian4((unsigned int*)&x);
    }
    fwrite(&x, sizeof(float), 1, f);
}

double read_double(FILE* f)
{
    float tmp;
    fread(&tmp, 1, sizeof(float), f);
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian8((unsigned long*)&tmp);
    }
    return tmp;
}

void write_double(FILE* f, double x)
{
    if(byte_order == TIFF_BIG_ENDIAN)
    {
        swap_endian8((unsigned long*)&x);
    }
    fwrite(&x, sizeof(double), 1, f);
}

void swap_endian2(unsigned short* x)
{
    *x = (*x>>8) | (*x<<8);
}

void swap_endian4(unsigned int* x)
{
    *x = (*x>>24) | 
        ((*x<<8) & 0x00FF0000) |
        ((*x>>8) & 0x0000FF00) |
        (*x<<24);
}

void swap_endian8(unsigned long* x)
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

