/*
 * csv.h
 * parse csv file into arrays of tokens
 */

#ifndef _CSV_H_
#define _CSV_H_

#define NUM_TOKENS 20
#define MAX_TOKEN_LEN 80

void parse_line(char* line, char** toks);

#endif
