/*
 * util.h
 * utility functions for reading and writing binary data
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>

unsigned int read_uint(FILE* f);
int read_int(FILE* f);
char read_sbyte(FILE* f);
unsigned char read_byte(FILE* f);
float read_float(FILE* f);
double read_double(FILE* f);
unsigned short read_ushort(FILE* f);
short read_short(FILE* f);
void write_uint(FILE* f, unsigned int n);
void write_int(FILE* f, int n);
void write_byte(FILE* f, unsigned char b);
void write_sbyte(FILE* f, char b);
void write_float(FILE* f, float x);
void write_double(FILE* f, double x);
void write_short(FILE* f, short k);
void write_ushort(FILE* f, unsigned short k);
void swap_endian2(unsigned short* x);
void swap_endian4(unsigned int* x);
void swap_endian8(unsigned long* x);

#endif
