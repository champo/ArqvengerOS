#include "system/rtc.h"
#include "system/io.h"
#include "library/stdio.h"

#define RTCADDRESS          0x70
#define RTCDATA             0x71

#define SECONDSREGISTER     0
#define MINUTESREGISTER     2
#define HOURSREGISTER       4
#define DAYREGISTER         7
#define MONTHREGISTER       8
#define YEARREGISTER        9
#define CENTURYREGISTER     0x32


#define BCDTOBINARY(X)      (((X) & 0xF0) >> 1) + (((X) & 0xF0) >> 3) + ((X) & 0xf) 
#define ISLEAPYEAR(i)       (((i) % 4 == 0 && (i) % 100 != 0) || (i) % 400 == 0)
#define SECONDSINDAY        86400

static enum months {JAN = 1, FEB, MAR, APRIL, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC };

int readSeconds(void);
int readRinutes(void);
int readHours(void);
int readDay(void);
int readMonth(void);
int readYear(void);
int readCentury(void);


time_t getTime(time_t *tp) {
    RTCRegisters regs;
    readRTCRegisters(&regs); 
    //epoch jan 1 1970
    int thisYear = (regs.century * 100) + regs.year;
    unsigned int daysSinceEpoch = 0;
    int i;
    for(i = 1970; i < thisYear; i++) {
        if (ISLEAPYEAR(i) ) {
            daysSinceEpoch += 366;
        } else {
            daysSinceEpoch += 365;
        }
    }
    for( i = 1; i < regs.month; i++) {
        switch (i) {
        case JAN: case MAR: case MAY: case JUL: case AUG: case OCT: case DEC:
            daysSinceEpoch += 31;
            break;
        case APRIL: case JUN: case SEP: case NOV:
            daysSinceEpoch += 30;
            break;
        case FEB:
            daysSinceEpoch += (ISLEAPYEAR(i))?29:28;
            break;   
        }
    }
    daysSinceEpoch += regs.day;
    return (daysSinceEpoch * SECONDSINDAY) + (regs.hours * 3600) + (regs.minutes * 60) + regs.seconds ;    
}


void readRTCRegisters(RTCRegisters *regs){
    int format;
    //unsigned char aux = 0;
   
    //TODO this is something we should do but apparently it doesn't work
    //  stays or goes??
    // Getting the RTC register A
    // If RTC in update mode wait until it finish updating
    //while (aux != 0x80 ) { 
     //   outB(RTCADDRESS, 10);    
      //  aux= inB(RTCDATA);
    //printf("aux = %d\n",aux);
    //}
    
    //outB(RTCADDRESS, 11);
    //outB(RTCDATA, 4);
    
    // Getting format register    
    outB(RTCADDRESS, 11);
    format = inB(RTCDATA);
    // Getting seconds
    regs->seconds = readSeconds();
    // Getting minutes
    regs->minutes = readMinutes();
    // Getting hours (value may be 12hs o 24hs format)
    regs->hours = readHours();
    // Getting day of the month
    regs->day = readDay();
    // Getting month
    regs->month = readMonth();
    // Getting year
    regs->year = readYear();
    // Getting century
    regs->century = readCentury();
    // If in BCD mode, convert to binary
    if ((format & 0x02) ==  0x02 ) {
       regs->seconds = BCDTOBINARY(regs->seconds);
       regs->minutes = BCDTOBINARY( regs->minutes);
       regs->hours = BCDTOBINARY(regs->hours);
       regs->day = BCDTOBINARY(regs->day );
       regs->month = BCDTOBINARY(regs->month);
       regs->year = BCDTOBINARY(regs->year);
       regs->century = BCDTOBINARY(regs->century);
    }
    // If in 12 hs mode, convert to 24 hs mode 
    if ((format & 0x04) == 0x04) {
        // Masking off the pm/am bit
        if ( (regs->hours & 0x80) == 0x80 ) {
            regs->hours &= 0x7F;
            // Setting 12 pm as 0 regs->hours and the rest adjusting the rest
            regs->hours = (regs->hours == 12)? 0 : regs->hours + 12;
        }
    }
}


int readSeconds(void) {
    outB(RTCADDRESS, SECONDSREGISTER);
    return  inB(RTCDATA);
}

int readMinutes(void) { 
    outB(RTCADDRESS, MINUTESREGISTER);
    return  inB(RTCDATA);
}

int readHours(void) {
    outB(RTCADDRESS, HOURSREGISTER);
    return inB(RTCDATA);
}

int readDay(void) {
    outB(RTCADDRESS, DAYREGISTER);
    return  inB(RTCDATA);
}

int readMonth(void) {
    outB(RTCADDRESS, MONTHREGISTER);
    return  inB(RTCDATA);
}

int readYear(void) {
    outB(RTCADDRESS, YEARREGISTER);
    return inB(RTCDATA);
}

int readCentury(void) {
    outB(RTCADDRESS, CENTURYREGISTER);
    return inB(RTCDATA);
}
