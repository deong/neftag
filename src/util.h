/*
 * util.h
 * utility functions for reading and writing binary data
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include "types.h"

unsigned byte read_byte(FILE* f);
void write_byte(FILE* f, unsigned byte b);

byte read_sbyte(FILE* f);
void write_sbyte(FILE* f, byte b);

int16 read_int16(FILE* f);
void write_int16(FILE* f, int16 k);

unsigned int16 read_uint16(FILE* f);
void write_uint16(FILE* f, unsigned int16 k);

int32 read_int32(FILE* f);
void write_int32(FILE* f, int32 n);

unsigned int32 read_uint32(FILE* f);
void write_uint32(FILE* f, unsigned int32 n);

float32 read_float32(FILE* f);
void write_float32(FILE* f, float32 x);

float64 read_float64(FILE* f);
void write_float64(FILE* f, float64 x);

void swap_endian2(unsigned int16* x);
void swap_endian4(unsigned int32* x);
void swap_endian8(unsigned int64* x);

#endif
