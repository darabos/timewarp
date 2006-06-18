/* $Id$ */ 
#include <math.h>
#include <string.h>

#include <allegro.h> //only used for endianness

#include "base.h"
#include "errors.h"
#include "types.h"
#include "random.h"

/*------------------------------
		RANDUM NUMBER GENERATOR
------------------------------*/

RNG rng;

inline Uint32 _rng_dist_32_flat ( Uint32 m, Uint32 r ) {
#	if defined(_MSC_VER) && defined(__i386__) && !defined(NO_ASM)
		//figure out if this works on all MSVC versions
		//figure out how to detect x86 on MSVC
		_asm {
			mov eax, [r]
			mov edx, [m]
			mul edx
			mov [r], edx
		}
		return r;
#	elif defined(__GNUC__) && defined(__i386__) && !defined(NO_ASM)
		asm ("mull %0" : "=d" (r) : "%a" (m), "0" (r) );
		return r;
#	else //add gcc asm sometime
		return (Uint32)((r * (Uint64)m) >> 32);
#	endif
}

enum {
	MULTIPLIER = 1812433253,
	ADDITIVE   = 123456789
};
Uint32 RNG_lcg64a::randi(Uint32 max) {
	return _rng_dist_32_flat(max, raw32());
}


Uint32 RNG_lcg64a::raw32()
{
	/* this algorithm is too repetitive
#	if defined(_MSC_VER) && defined(__i386__) && !defined(NO_ASM)
		split_int_64 i64 = s64;
		_asm { mov eax, [i64.s.high] } 
		_asm { mov ebx, MULTIPLIER } 
		_asm { imul eax, ebx } 
		_asm { mov ecx, eax } 
		_asm { mov eax, [i64.s.low] } 
		_asm { mul ebx } 
		_asm { add eax, ADDITIVE } 
		_asm { adc ecx, edx} 
		_asm { mov [i64.s.low], eax } 
		_asm { mov [i64.s.high], ecx } 
		s64 = i64;
#	elif 0 && defined(__GNUC__) && !defined(NO_ASM)
		Uint32 _blah;
		asm (
			"imull %3, %5"	"\n\t"
			"mull  %3"		"\n\t"
			"addl  %6, %1"	"\n\t"
			"adcl  %0, %2"
			: "=d" (_blah),      "=a" (i64.s.low), "=b" (i64.s.high)
			: "0"  (MULTIPLIER), "1"  (i64.s.low),  "2" (i64.s.high),  "i" (ADDITIVE) 
		);
#	else
		s64.whole = (s64.whole * MULTIPLIER) + ADDITIVE;
#	endif
		*/

	// a little less repetitive
	const int ran_mult[4] = {1812433253, 2094358901, 1809384569, 1740025465};
	const int ran_add[4]  = { 123456789,  239048723,  958178125,  281010345};

	int k = (s64.s.high) & 3;
	//s64.whole = ((s64.whole + ran_add[k]) * ran_mult[k]);// + ran_add[k];
	s64.whole = (s64.whole * ran_mult[k]) + ran_add[k];

	return s64.s.high;

}
Uint64 RNG_lcg64a::raw64() {
	Uint64 bob = raw32();
	bob = (bob << 32) | raw32();
	return bob;
}
void RNG_lcg64a::fast_forward ( Uint64 how_far ) {

	if (how_far == 0) return;

	Uint64 tm = MULTIPLIER;
	Uint64 ta = ADDITIVE;

	while (1) {
		if (how_far & 1) s64.whole = s64.whole * tm + ta;
		how_far >>= 1;
		if (how_far == 0) return;
		ta = ta * tm + ta;
		tm = tm * tm;
	}
	return;
}

void RNG_lcg64a::seed( int s )
{
	s64.s.low = s;
	s64.s.high = 0;

}
void RNG_lcg64a::seed_more( int s ) {
	if (s64.s.high & 0x80000000) raw32();
	raw32();
	s64.s.low += ((s64.s.high >> 27) | (s64.s.high << 5)) ^ s;
	return;
}






static unsigned int tw_random_value;
static const int N = 32;
static unsigned int ran_temp[N];


static unsigned int tw_random_value_pushed;
static unsigned int ran_temp_pushed[N];

void tw_random_push_state()
{
	tw_random_value_pushed = tw_random_value;

	int k;
	for ( k = 0; k < N; ++k )
	{
		ran_temp_pushed[k] = ran_temp[k];
	}
}

void tw_random_pop_state()
{
	tw_random_value = tw_random_value_pushed;

	int k;
	for ( k = 0; k < N; ++k )
	{
		ran_temp[k] = ran_temp_pushed[k];
	}
}


unsigned int tw_random_unsigned()
{
	// some other thingy...
	int i;
	int carrybit = 0;
	for ( i = 0; i < N; ++i )
	{
		unsigned int bit1 = tw_random_value & 0x080000000;
		unsigned int bit2 = ran_temp[i]     & 0x080000000;
		if (bit1 && bit2)
			++ carrybit;	// preserve the bit; otherwise you may lose information in the lower bit(s)

		tw_random_value += ran_temp[i];
		ran_temp[i] = tw_random_value;// << 1) | (s64.s.high & 1);
	}
	tw_random_value += carrybit;

	return tw_random_value;
}

// integer output is needed, otherwise an equation like "1 - tw_random()" can return a value of 4 billion.
int tw_random()
{
	return int(tw_random_unsigned());
}

#ifdef _DEBUG
#include "../melee/mview.h"
void test_random()
{
	double count[N][4];

	// a test of the random procedure:
	const int Ntest = 1000000;
	int i, k;

	for ( k = 0; k < N; ++k )
	{
		count[k][0] = 0;
		count[k][1] = 0;
		count[k][2] = 0;
		count[k][3] = 0;
	}
	
	int last_ran = 0;
	int new_ran = 0;
	for ( i = 0; i < Ntest; ++i )
	{
		last_ran = new_ran;
		new_ran = tw_random();

		for ( k = 0; k < N; ++k )
		{
			int bit1, bit2;
			bit1 = (last_ran >> k) & 1;
			bit2 = (new_ran  >> k) & 1;

			int b = bit1 + (bit2<<1);
			count[k][b] += 1;

			// change observed in bit k:
			// i=0:  0 -> 0
			// i=1:  1 -> 0
			// i=2:  0 -> 1
			// i=3:  1 -> 1
		}


		// extra test...
		int maxval = 1000;
		int w;
		w = tw_random(maxval);
		if (w < 0 || w >= maxval)
			tw_error("unsigned int value out of bounds");

	}

	// display results
	for ( k = 0; k < N; ++k )
	{
		message.print(100, 15, "%2i %4.2lf  %4.2lf  %4.2lf  %4.2lf",
			k, count[k][0]/Ntest, count[k][1]/Ntest, count[k][2]/Ntest, count[k][3]/Ntest);
	}
	message.print(100, 14, "press a key");
	message.animate(0);
	readkey();

}
#endif

#include "../melee/mlog.h"
void seed_ohmy()
{
	tw_random_value = rand();
	share(-1, &tw_random_value);

	int i;
	for ( i = 1; i < N; ++i )
	{
		ran_temp[i] = rand();
		share(-1, &ran_temp[i]);
	}
	share_update();

	// only for debugging purpose.
	//test_random();

	return;
}


double tw_random(double a) 
{
	double val;
	//* ((int*) &val + 0)           = tw_random();
	//* ((int*) &val + sizeof(int)) = tw_random();
	val = a * (double(tw_random_unsigned()) / double(0x0100000000));


	if ((a != 0) && (val < 0 || val >= a))
	{
		tw_error("random (double), out of bounds!");
	}


	return val;
}

double tw_random(double min, double max) 
{
	return min + tw_random(max - min);
}


int tw_random( int a )
{
	if (a < 0)
	{
		tw_error("random: needing to convert a negative integer to an unsigned integer: should not happen");
	}

	//	int k = int( tw_random_unsigned() % unsigned int(a) );
	int k = int( tw_random_unsigned() % int(a) );

	if (k < 0 || k >= a)
	{
		tw_error("random (int), out of bounds!");
	}

	return k;
}
