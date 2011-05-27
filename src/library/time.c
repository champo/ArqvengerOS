#include "library/time.h"
#include "system/call/codes.h"
#include "library/string.h"
#include "library/stdio.h"

extern size_t systemCall(int eax, int ebx, int ecx, int edx);

static int dayOfWeek(int year, int month, int day);

time_t time(time_t *tp) {
    return systemCall(_SYS_TIME,tp,0,0);
}

char* asctime(struct tm *tp ) {
    int i = 0;
    int curPos = 0;
    char string[30];
    char* day,*month;
    char aux[5];
    int wday = dayOfWeek(tp->year,tp->mon,tp->mday);
    
    switch (wday) {
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
    curPos += (tp->mday > 10)? 3:2;
     
    itoa(aux,tp->hour);
    strcpy(string + curPos, aux);
    curPos += (tp->hour > 10)? 2:1;
    string[curPos++] = ':';

    itoa(aux,tp->min);
    strcpy(string + curPos, aux);
    curPos += (tp->min > 10)? 2:1;
    string[curPos++] = ':';
    
    itoa(aux,tp->sec);
    strcpy(string + curPos, aux);
    curPos += (tp->sec > 10)? 2:1;
    string[curPos++] = ' ';
    
    itoa(aux,tp->year);
    strcpy(string + curPos, aux);
    curPos += 4;
    string[curPos++] = '\n';
    string[curPos] = '\0';

    for(i = 0; i < curPos;i++) {
        printf("%c",string[i]);
    }
    return string;
}


// This is a very know algorith, Sakamoto's algorithm. And was obteined from the Wikipedia.
// http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
static int dayOfWeek(int year, int month, int day) {
       static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
       year -= month < 3;
       return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
}
