#include "library/time.h"
#include "system/call/codes.h"
#include "library/string.h"
#include "library/stdlib.h"
#include "library/call.h"


static int dayOfWeek(int year, int month, int day);
static void initTm(struct tm *date);
static struct tm sharedTm;
void dateFromDayNumber(int *date, time_t daysSinceEpoch);

static char string[30];

/**
 * Get current time.
 *
 * Return the current calendar time as a time_t.
 *
 * The function returns this value, and if the argument is not a null pointer,
 * the value is also set to the object pointed by timer.
 *
 * @param tp Pointer to time_t where there time can be stored, provided it is not null.
 *
 * @return The current time.(Number of seconds since Epoch)
 */
time_t time(time_t *tp) {
    return SYS2(_SYS_TIME, tp);
}


/**
 * Returns a the local time in a human-readable formtat.
 *
 * Interprets the contents of the tm structure pointed by tp as a calendar time and
 * converts it to a C string containing a human-readable version of the corresponding
 * date and time.
 *
 * The returned string has the following format:
 *
 * Www Mmm dd hh:mm:ss yyyy
 * Where Www is the weekday, Mmm the month in letters, dd the day of the month,
 * hh:mm:ss the time, and yyyy the year.
 *
 * The string is followed by a new-line character ('\n') and the terminating
 * null-character.
 *
 * This function, as the one from the C standard library returns a char pointer.
 * This represents a problem if we can't dinamically allocate memory, so we decided to
 * restrict the function and store the variable as  global. This means that succesive
 * calls to asctime will overwrite it. The user should take this into consideration when
 * using it.
 *
 * @param tp Pointer to tm structure, representing the local time.
 *
 * @return A string representing the local time in a human-readable format.
 */
char* asctime(const struct tm *tp) {

    int i = 0;
    int curPos = 0;
    char* day,*month;
    char aux[5];

    switch (tp->wday) {
        case 0:
            day = "Sun";
            break;
        case 1:
            day = "Mon";
            break;
        case 2:
            day = "Tue";
            break;
        case 3:
            day = "Wed";
            break;
        case 4:
            day = "Thu";
            break;
        case 5:
            day = "Fri";
            break;
        case 6:
            day = "Sat";
            break;
    }
    strcpy(string,day);
    curPos += 3;
    string[curPos++] = ' ';

    switch(tp->mon) {
        case 1:
            month = "Jan";
            break;
        case 2:
            month = "Feb";
            break;
        case 3:
            month = "Mar";
            break;
        case 4:
            month = "Apr";
            break;
        case 5:
            month = "May";
            break;
        case 6:
            month = "Jun";
            break;
        case 7:
            month = "Jul";
            break;
        case 8:
            month = "Aug";
            break;
        case 9:
            month = "Sep";
            break;
        case 10:
            month = "Oct";
            break;
        case 11:
            month = "Nov";
            break;
        case 12:
            month = "Dec";
            break;
    }
    strcpy(string + curPos,month);
    curPos += 3;

    string[curPos++] = ' ';

    itoa(aux,tp->mday);
    strcpy(string + curPos, aux);
    curPos += (tp->mday >= 10)? 2:1;
    string[curPos++] = ' ';

    itoa(aux,tp->hour);
    if (tp->hour < 10) {
        string[curPos++] = '0';
    }
    strcpy(string + curPos, aux);
    curPos += (tp->hour >= 10)? 2:1;
    string[curPos++] = ':';

    itoa(aux,tp->min);
    if (tp->min < 10) {
        string[curPos++] = '0';
    }
    strcpy(string + curPos, aux);
    curPos += (tp->min >= 10)? 2:1;
    string[curPos++] = ':';

    itoa(aux,tp->sec);
    if (tp->sec < 10) {
        string[curPos++] = '0';
    }
    strcpy(string + curPos, aux);
    curPos += (tp->sec >= 10)? 2:1;
    string[curPos++] = ' ';

    itoa(aux,tp->year);
    strcpy(string + curPos, aux);
    curPos += 4;
    string[curPos] = '\0';

    return string;
}

/**
 * Return a tm structure filled out with the data for the time timer.
 *
 * @param timer A pointer to the time_t to use.
 *
 * @return A shared structure with the data.
 */
struct tm* localtime(const time_t* timer) {
    time_t secsSinceEpoch = *timer;
    time_t daysSinceEpoch = secsSinceEpoch / (3600 * 24);
    int aux[3];
    dateFromDayNumber(aux, daysSinceEpoch);
    sharedTm.year = aux[0];
    sharedTm.mon = aux[1];
    sharedTm.mday = aux[2];
    sharedTm.wday = dayOfWeek(sharedTm.year,sharedTm.mon,sharedTm.mday);
    int secsOfDay = secsSinceEpoch % (3600 * 24);
    sharedTm.hour = secsOfDay / 3600;
    sharedTm.min = (secsOfDay % 3600) / 60;
    sharedTm.sec = ((secsOfDay % 3600) % 60);

    return &sharedTm;
}

/**
 * Returns the number of the day of the week.
 *
 * The weeks starts at Sunday which is 0 and finishes in Saturday, 6.
 *
 * This is a very know algorith, Sakamoto's algorithm. And was obtained from the Wikipedia.
 *
 * http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
 *
 * @param year  The current year.
 * @param month The current month.
 * @param day   The current day.
 *
 * @return A number representing the day of the week.
 */
static int dayOfWeek(int year, int month, int day) {
       static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
       year -= month < 3;
       return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
}

/**
 * Initializes an array of 3 integers with the current year, month and day.
 *
 *  This a known algorithm to calculate the year, month and day of the month
 * given a number day (since Epoch).
 *
 *  http://alcor.concordia.ca/~gpkatch/gdate-algorithm.html
 *
 * @param date	Array of three integers containing year,month and day.
 * @param daysSinceEpoch Number of seconds since Epoch.
 *
 */
void dateFromDayNumber(int *date, time_t daysSinceEpoch) {
    unsigned int year,month,day,ddd,mi;
    daysSinceEpoch -= 60;   // adjusting the reference date of the algorith from Mar 1 1970
                            // to epoch (Jan 1 1970)
    year = (10000 * daysSinceEpoch + 14780) / 3652425;  //year ~= days / 365.2425
    ddd = daysSinceEpoch - (365 * year + year / 4 - year / 100 + year / 400);
    if (ddd < 0) {
        year = year - 1;
        ddd = daysSinceEpoch - (365 * year + year / 4 - year / 100 + year / 400);
    }
    mi = (100 * ddd + 52) / 3060;
    month = (mi + 2) % 12 + 1;
    year = year + (mi + 2) / 12;
    day = ddd - (mi * 306 + 5) / 10 + 1;
    date[0] = year + 1970;
    date[1] = month;
    date[2] = day ;
}

