#ifndef _GET_TIME_H
#define _GET_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

void init_time();      //to initialize time stuff
void deinit_time();    //to de-initialize time stuff

volatile int    get_time();     //to get the current time in milliseconds
volatile double get_time2();    //to get the current time in milliseconds at high precision
volatile Sint64 get_time3();    //to get the current time in unspecified units
//in all cases, the current time is measured relative to the call to init_time();

int idle ( int time ); //to yeild a number of milliseconds to the OS
extern int _no_idle;   //to disable the above function

#ifdef __cplusplus
}
#endif


#endif
