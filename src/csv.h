/*
 * csv.h
 * parse csv file into arrays of tokens
 */

#ifndef _CSV_H_
#define _CSV_H_

void parse_line(char* line, char* sep, char** toks, unsigned int num_tokens);

#endif
