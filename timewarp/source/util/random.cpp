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


#define tw_random_length0 8
#define tw_random_length1 7
COMPILE_TIME_ASSERT(tw_random_length0 >= 3);
COMPILE_TIME_ASSERT(tw_random_length1 >= 4);

//tw_random_length0 and tw_random_length1 MUST have no common factors!

struct {
	Uint32 array0[tw_random_length0];	//sets of 32 random bits
	Uint32 array1[tw_random_length1];	//sets of 32 random bits
	char index0, index1;//indeces into array0 and array1 respectively
	unsigned char carry;//either 0 or 1
	char cycle_next;   //an index into array1
	//the division line between frequently and infrequently accessed data
	Uint32 cycle_count;
	Uint32 cycle_state0;
	Uint32 cycle_state1;
} _tw_random_state;
/*
This struct contains the complete state of the random number generator
The entire Random Number Generator is split into 3 portions:
the primary RNG, the secondary RNG, and the seeder

The primary RNG is the function _tw_random().  The data it uses is placed 
at the start of the struct.  It is fast, and well suited for 
implementation in hardware or use on embedded processors.  

The secondary RNG is called by the primary RNG occaisonally, to modify 
the internal state of the primary RNG.  While the primary RNG has a 
sufficiently long peroid for any application, it is expected to show some 
patterns observable over long peroids of time.  The second RNG is there to 
make sure it doesn't.  The second RNG is tw_random_cycle().  The current 
implementation is moderately fast, but makes use of some integer 
multiplication, making it more difficult / slow / expensive to do in 
hardware.  Because the secondary RNG is only called occaisonally, it 
doesn't have to be fast.  However, I prefer it to be fast so that 
_tw_random() takes a relatively fixed duration.  

The seeder initializes the state of both the primary RNG and the 
secondary RNG.  I think the current implementation may be the limiting 
factor in the overal output quality of the RNG as a whole, particularly 
if the seed is small.  The seeder is implemented in tw_random_seed() 
and tw_random_seed_more().  

*/
static void tw_random_cycle () {
	//adjust cycle_count
	_tw_random_state.cycle_count += 1;

	//adjust cycle_state0
	_tw_random_state.cycle_state0 = 
		(_tw_random_state.cycle_state0 * 0x343FD) + 0x269EC3;

	int i, j;

	//pick an index into array0
	i = 15 & ((_tw_random_state.cycle_state0 >> 8) + _tw_random_state.carry);
	while (i > tw_random_length0) i -= tw_random_length0;

	//pick a bit to start reading at
	j = 15 & ((char)_tw_random_state.cycle_count * 5);

	//adjust cycle_state1 using array0 & cycle_state0
	_tw_random_state.cycle_state1 *= 
		(((_tw_random_state.array0[i] >> j) & 0xff) | 1);
	_tw_random_state.cycle_state1 += _tw_random_state.cycle_state0;

	//adjust array1 using cycle_state0
	Uint32 k;
	k = (_tw_random_state.cycle_state1 >> 16) & 0xff;
	_tw_random_state.array1[_tw_random_state.cycle_next] ^= 
		_tw_random_state.cycle_count + (k << i);


	//pick next cycle_index
	int l;
	l = _tw_random_state.cycle_state1 >> 24;
	while (l > tw_random_length1) l = (l - tw_random_length1) >> 1;
	_tw_random_state.cycle_next = l;
}

int tw_random_save_state ( void *buffer, int *size ) {
	int s = sizeof(_tw_random_state);
	if (*size < s) return s;
	*size -= s;
	memcpy(buffer, &_tw_random_state, s);
	return 0;
}

int tw_random_load_state ( const void *buffer, int *size ) {
	int s = sizeof(_tw_random_state);
	if (*size < s) return s;
	*size -= s;
	memcpy(&_tw_random_state, buffer, s);
	return 0;
}
/* untested example of state saving & loading:
static void test_tw_random_save_and_load() {
	char some_memory[1024];
	int s = 1024;
	int temp;
	temp = tw_random_save_state ( some_memory, &s );
	if (temp) {
		printf("tw_random_save_state failed\n");
		printf("because it would needs %d bytes\n", temp);
		printf("which is more than the 1024 we gave it\n");
		return;
	}
	temp = tw_random();
	printf("tw_random_save_state succeeded\n");
	printf("it used %d bytes\n", 1024 - s);
	s = 1024 - s;
	if (tw_random_load_state ( some_memory, &s )) {
		printf("something went wrong\n");
		printf("tw_random_load_state reported an error when it shouldn't\n");
		return;
	}
	if (s != 0) {
		printf("something went wrong\n");
		printf("tw_random_load_state had %d bytes left over\n", s);
		return;
	}
	if (tw_random() != temp) {
		printf("something went wrong\n");
		printf("the state was not properly saved\n");
		printf("even though no errors were reported\n");
		return;
	}
	printf("tw_random_save_state and tw_random_load_state worked perfectly\n");
	return;
}
*/

Uint32 _tw_random() {
	Uint32 i;
	Uint64 sum;
	sum = 0 +
		(Uint64) _tw_random_state.array0[
			_tw_random_state.index0 ] + 
		(Uint64) _tw_random_state.array1[
			_tw_random_state.index1 ] + 
		(Uint64) _tw_random_state.carry;
	_tw_random_state.carry = (Uint32) (sum >> 32);
	i = (Uint32) sum;
	_tw_random_state.array0[_tw_random_state.index0] ^= i;
	_tw_random_state.array1[_tw_random_state.index1] ^= i;
	_tw_random_state.index0 -= 1;
	if (_tw_random_state.index0 < 0) {
		_tw_random_state.index0 += tw_random_length0;
		if (_tw_random_state.index1 == _tw_random_state.cycle_next) 
			tw_random_cycle();
	}
	_tw_random_state.index1 -= 1;
	if (_tw_random_state.index1 < 0) {
		_tw_random_state.index1 += tw_random_length1;
	}
	return i;
}
int tw_random() {
	return _tw_random() & 0x7fffffff;
}
int tw_random( int a ) {
	return (Uint32) (( ((Uint64)a) * _tw_random()) >> 32);
}
double tw_random(double a) {
	a *= _tw_random();
	return ldexp(a, -32);
}
int tw_random_state_checksum() {
	int i;
	i = _tw_random_state.array0[_tw_random_state.index0] + _tw_random_state.array1[_tw_random_state.index1] + _tw_random_state.carry;
	return i & 0x7fffffff;
}
void tw_random_seed (Uint32 _s) {
	int i;
	Uint64 s = _s;

	//initialize _tw_random() stuff (except arrays)
	_tw_random_state.carry = (s >> 30) & 1; //random is better
	_tw_random_state.index0 = 0;            //0 is fine
	_tw_random_state.index1 = 0;            //0 is fine

	//initialize tw_random_cycle() stuff
	_tw_random_state.cycle_next = 0;        //???
	_tw_random_state.cycle_count = 0;       //0 is fine
	_tw_random_state.cycle_state0 = 0;      //must be random
	_tw_random_state.cycle_state1 = 0;      //???

	s ^= 0x47E51038ul;

	//initialize arrays
	for (i = 0; i < tw_random_length0; i += 1) {
		s = s*1664525+2401;
		_tw_random_state.array0[i] = s;
	}
	_tw_random_state.index0 = 0;

	for (i = 0; i < tw_random_length1; i += 1) {
		s = s*(s+1);
		_tw_random_state.array1[i] = s;
	}
	_tw_random_state.index1 = 0;

	_tw_random_state.cycle_state0 = 
		((_tw_random_state.array0[1] - _tw_random_state.array1[1]) *
		(_tw_random_state.array0[0] ^ _tw_random_state.array1[2])) ^
		_tw_random_state.array1[0];


	//burn-in
	for (i = 0; i < tw_random_length0 + tw_random_length1; i += 1) tw_random();
	return;
}
void tw_random_seed_more (Uint32 s) {
	s ^= ((s >> 30) << 16);
	s = (s << 2) + 2;
	int i;
	for (i = 0; i < 6; i += 1) s = s*(s+1);
	for (i = 0; i < tw_random_length0; i += 1) {
		_tw_random_state.array0[i] ^= s;
		s = s*(s+1);
		tw_random();
	}
	s ^= 0x00E51030;
	for (i = 0; i < tw_random_length1; i += 1) {
		_tw_random_state.array1[i] += s;
		s = s*1664525+2401;
		tw_random();
	}
	for (i = 0; i < 4; i += 1) tw_random();
	return;
}

static Uint32 _fs_random_array[16] = {
	3794150496ul, 1473219749ul, 3209682484ul, 
	1731974425ul, 2943695494ul, 3805535487ul, 
	2723338447ul, 3305679311ul, 3893065612ul, 
	2404355475ul,  259610600ul, 4248670353ul,
	2476056498ul, 1212015016ul, 2794733892ul,
	1193601896ul
};
static union {
	Uint64 full_state;
	struct {
#		if defined ALLEGRO_LITTLE_ENDIAN
			Uint32 b;
			Uint32 a;
#		elif defined ALLEGRO_BIG_ENDIAN
			Uint32 a;
			Uint32 b;
#		else
#			error unknown endianness
#		endif
	} num;
} _fs = {0};

Uint64 fs_random_get_seed64 () {
	return _fs.full_state;
}
void fs_random_seed64 ( Uint64 state ) {
	_fs.full_state = state;
}
void fs_random_seed32 (Uint32 s) {
	int j, k;
	j = _fs_random_array[s&15];
	k = _fs_random_array[(~s>>2)&15];
	_fs.num.a = (j*s) ^ k;
	_fs.num.b = _fs.num.a + s - j;
	return;
}

Uint32 fs_random() {
	Uint32 tmp;
	_fs.num.a *= 0x343FD; //a standard A = (A * P + C) % M random number generator
	_fs.num.a += 0x269EC3;//with the constants matching those used by MS Visual C++'s rand()
	_fs.num.b ^= _fs.num.a;
	tmp = _fs.num.b;
	_fs.num.b += _fs_random_array[_fs.num.b >> 28];
	return tmp;
}

static const unsigned int fs_default_seed[6] = { 
	3794150496ul, 1473219700ul, 3209682484ul, 
	2723338447ul, 3305679311ul, 3893065612ul 
};
void fs_random_init ( const unsigned int *seed ) {
	if (!seed) seed = fs_default_seed;
	_fs.num.a = seed[0];
	_fs.num.b = seed[1];
	Uint32 i, a, b;
	a = seed[2];
	for (i = 0; i < 16; i += 1) {
		a = a * (a+1);
		_fs_random_array[i] = a;
	}
	a = seed[3];
	for (i = 0; i < 16; i += 2) {
		a = a*1664525+2401;
		_fs_random_array[i] ^= a;
	}
	a = seed[4];
	for (i = 0; i < 16; i += 1) {
		Uint32 tmp = fs_random();
		if ((a>>i) & 1) _fs_random_array[i] ^= tmp;
		else _fs_random_array[i] += tmp;
	}
	a = seed[5];
	for (i = 0; i < 16; i += 1) {
		b = a * (_fs_random_array[i] - _fs_random_array[i ^ 15]);
		_fs_random_array[i] += a;
		a = b;
	}
	a ^= seed[3];
	for (i = 1; i < 16; i += 2) {
		a = a*1664525+2401;
		_fs_random_array[i] ^= a;
	}
	return;
}

