/*
 * date.c
 * basic date math functionality
 */

#include "date.h"

/*
 * figure out what is the last day of a given month
 * days numbered from 1 to N 
 */
int last_day_of_month(int mon, int year)
{
    int max_day;
    switch(mon)
    {
    case 0:
    case 2:
    case 4:
    case 6:
    case 7:
    case 9:
    case 11:
        max_day = 31;
        break;
    case 3:
    case 5:
    case 8:
    case 10:
        max_day = 30;
        break;
    case 1:
        if(is_leap_year(year)+1900)
            max_day = 29;
        else
            max_day = 28;
        break;
    }
    return max_day;
}

/*
 * check if year is a leap year
 */
int is_leap_year(int year)
{
    if(year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
        return 1;
    return 0;
}

/*
 * add a given number of hours to a given time, updating
 * the time to handle rolling over of days, months, years
 */
void add_offset(struct tm* t, int hours)
{
    /* if offset makes us go negative on hours */
    if(t->tm_hour + hours < 0)
    {
        /* roll back to previous day and make sure day still valid */
        t->tm_hour = 24 - hours;
        if(--(t->tm_mday) <= 0)
        {
            /* back up one month */
            if(--(t->tm_mon) < 0)
            {
                /* back up one year */
                t->tm_mon = 11;
                --(t->tm_year);
            }
            t->tm_mday = last_day_of_month(t->tm_mon, t->tm_year);
        }
    }
    /* if offset makes hours go greater than 23 */
    else if(t->tm_hour + hours > 23)
    {
        /* roll hours over and figure out how to increment day */
        int max_day = last_day_of_month(t->tm_mon, t->tm_year);
        t->tm_hour = (t->tm_hour + hours) % 24;
        /* make sure day of month still valid */
        if(++(t->tm_mday) > max_day)
        {
            /* if not, roll it over and increment month */
            t->tm_mday = 1;
            if(++(t->tm_mon) > 11)
            {
                /* if month goes too far, roll it and bump year */
                t->tm_mon = 0;
                ++(t->tm_year);
            }
        }
    }
    else
    {
        t->tm_hour += hours;
    }
}
