#ifndef __GET_TIME_H__
#define __GET_TIME_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void init_time();      //to initialize time stuff
void deinit_time();    //to de-initialize time stuff

int is_time_initialized(); //returns non-zero if init_time() has been called, otherwise 0

volatile int    get_time();     //to get the current time in milliseconds
volatile double get_time2();    //to get the current time in milliseconds at high precision
volatile Sint64 get_time3();    //to get the current time in unspecified units
//in all cases, the current time is measured relative to the call to init_time();

int idle ( int time ); //to yield a number of milliseconds to the OS
extern int _no_idle;   //to disable the above function

#ifdef __cplusplus
}
#endif


#endif // __GET_TIME_H__

