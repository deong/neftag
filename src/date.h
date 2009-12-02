/*
 * date.h
 * basic date math functionality
 */

#ifndef _DATE_H_
#define _DATE_H_

#include <time.h>

int last_day_of_month(int mon, int year);
int is_leap_year(int year);
void add_offset(struct tm* t, int hours);

#endif _DATE_H_
