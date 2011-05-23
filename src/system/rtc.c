#include "system/rtc.h"
#include "system/io.h"
#include "library/stdio.h"

#define RTCADDRESS  0x70
#define RTCDATA     0x71
void readRTCRegisters(void);

time_t getTime(time_t *tp) {
    readRTCRegisters();
    return 10;    
}


void readRTCRegisters(void){
    int format,seconds,minutes,hours,day,month,year;
    unsigned char aux = 0;
    // Getting the RTC register A
    // If RTC in update mode wait until it finish updating
    printf("1  asasasasasasa\n");
    //while (aux != 0x80 ) { 
     //   outB(RTCADDRESS, 10);    
      //  aux= inB(RTCDATA);
    //printf("aux = %d\n",aux);
    //}
    printf("asasasasasasa\n");
    outB(RTCADDRESS, 11);
    outB(RTCDATA, 6);
    // Getting format register
    outB(RTCADDRESS, 11);
    format = inB(RTCDATA);
    // Getting seconds
    outB(RTCADDRESS, 0);
    seconds = inB(RTCDATA);
    // Getting minutes
    outB(RTCADDRESS, 2);
    minutes = inB(RTCDATA);
    // Getting hours (value may be 12hs o 24hs format)
    outB(RTCADDRESS, 4);
    hours = inB(RTCDATA);
    // Getting day of the month
    outB(RTCADDRESS, 7);
    day = inB(RTCDATA);
    // Getting month
    outB(RTCADDRESS, 8);
    month = inB(RTCDATA);
    // Getting month
    outB(RTCADDRESS, 9);
    year = inB(RTCDATA);
    
    printf("format  %d \n",format);
    printf("seconds %d \n",seconds);
    printf("minutes %d \n",minutes);
    printf("hours %d \n",hours);
    printf("day %d \n",day);
    printf("month %d \n",month);
    printf("year %d \n",year);

}
