#include "library/time.h"
#include "system/call/codes.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"

extern size_t systemCall(int eax, int ebx, int ecx, int edx);

static int dayOfWeek(int year, int month, int day);
static void initTm(struct tm *date); 
void dateFromDayNumber(int *date, time_t daysSinceEpoch);

time_t time(time_t *tp) {
    return systemCall(_SYS_TIME,tp,0,0);
}

char* asctime(struct tm *tp ) {
    struct tm *date;
    if ( tp == NULL  ) {
        initTm(date);    
    } else {
        date = tp;
    }

    int i = 0;
    int curPos = 0;
    char string[30];
    char* day,*month;
    char aux[5];
    
    switch (date->wday) {
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
    
    switch(date->mon) {
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
    
    itoa(aux,date->mday);
    strcpy(string + curPos, aux);
    curPos += (date->mday >= 10)? 3:2;
     
    itoa(aux,date->hour);
    strcpy(string + curPos, aux);
    curPos += (date->hour >= 10)? 2:1;
    string[curPos++] = ':';

    itoa(aux,date->min);
    strcpy(string + curPos, aux);
    curPos += (date->min >= 10)? 2:1;
    string[curPos++] = ':';
    
    itoa(aux,date->sec);
    strcpy(string + curPos, aux);
    curPos += (date->sec >= 10)? 2:1;
    string[curPos++] = ' ';
    
    itoa(aux,date->year);
    strcpy(string + curPos, aux);
    curPos += 4;
    string[curPos++] = '\n';
    string[curPos] = '\0';

    for(i = 0; i < curPos;i++) {
        printf("%c",string[i]);
    }
    return string;
}

void initTm(struct tm *date) {
    time_t secsSinceEpoch = time(NULL);
    time_t daysSinceEpoch = secsSinceEpoch / (3600 * 24);
    int aux[3];
    dateFromDayNumber(aux,daysSinceEpoch);
    date->year = aux[0];
    date->mon = aux[1];
    date->mday = aux[2];
    date->wday = dayOfWeek(date->year,date->mon,date->mday);
    int secsOfDay = secsSinceEpoch % (3600 * 24);
    date->hour = secsOfDay / 3600;
    date->min = (secsOfDay % 3600) / 60;
    date->sec = ((secsOfDay % 3600) % 60); 
}

// This is a very know algorith, Sakamoto's algorithm. And was obtained from the Wikipedia.
// http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
static int dayOfWeek(int year, int month, int day) {
       static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
       year -= month < 3;
       return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
}


//  This a known algorithm to calculate the year, month and day of the month given a number day (since Epoch).
//  http://alcor.concordia.ca/~gpkatch/gdate-algorithm.html
void dateFromDayNumber(int *date, time_t daysSinceEpoch) {
    unsigned int year,month,day,ddd,mi;
    year = (10000 * daysSinceEpoch + 14780) / 3652425;
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
    date[1] = month - 2;
    date[2] = day + 1; 
}

