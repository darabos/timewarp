#ifndef _TYPES_H
#	include "types.h"
#endif

#ifndef __RANDOM_H
#define __RANDOM_H

/*
random.h & random.cpp contain 2 random number generators
tw_random and fs_random

tw_random is a standard random number generator

Speed:       ~20 cycles per number? (peak ~50 cycles?)
Output:      32 random bits
State:       configurable, generally about 600 bits
Worst Bits:  none (almost bit symmetric)
Seed Size:   Any multiple of 32 bits
Seed Speed:  ~1000 cycles per 32 bits of seed

*/

double tw_random(double a);
inline double tw_random(double min, double max) {return tw_random(max-min) + min;}
int tw_random( int a );
Uint32 _tw_random();
int tw_random();
int tw_random_state_checksum();
void tw_random_seed(Uint32 seed);
void tw_random_seed_more(Uint32 seed);

int tw_random_save_state ( void *buffer, int *size );
int tw_random_load_state ( const void *buffer, int *size );

/*
fs_random() is a special random number generator


Speed:       ~10 cycles per number? (no variation)
Output:      32 random bits
State:       64 bits
Worst Bits:  highest 4 (28-31) and lowest 8 (0-7)
Seed Size:   *** see below
Seed Speed:  *** see below

It splits it's seeding stage into two parts, the slow part and the fast part.  

Name:     Init            Seed
Function: fs_random_init  fs_random_seed
Input:    192 bits        32 or 64 bits
State:    512 bits        64 bits
Speed:    1000 cycles?    10 cycles?

This is done so that extremely fast reseeding can be done.  
*/
Uint32 fs_random() ;
//int fs_random_save_state ( void *buffer, int *size );
//int fs_random_load_state ( const void *buffer, int *size );

void fs_random_seed64( Uint64 seed);
Uint64 fs_random_get_seed64();
void fs_random_seed32 (Uint32 s);
//fs_random_init() shoulbe be called before fs_random_seed()
void fs_random_init ( const unsigned int *seed = 0 );//array of 6 integers to seed from
#endif
