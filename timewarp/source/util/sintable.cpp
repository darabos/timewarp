#include <math.h>
#include "base.h"
#include "sintable.h"

float _sintable_table[(1 << SINTABLE_WIDTH_BITS) + 1];

static char sintable_inited = 0;
void init_sintable() {
	COMPILE_TIME_ASSERT(SINTABLE_WIDTH_BITS >= 2);
	if (sintable_inited) return;
	int i;
	for (i = 0; i < 1 + (1 << SINTABLE_WIDTH_BITS); i += 1) {
		double f = i / (double)(1 << SINTABLE_WIDTH_BITS);
		f = sin(f * 3.14159265358979323 * 2);
		_sintable_table[i] = (float)f;
	}
	sintable_inited = 1;
}

double fixed_sin2(int angle) { 
	int angle2 = angle >> (24 - SINTABLE_WIDTH_BITS);
	angle2 &= (1 << SINTABLE_WIDTH_BITS) - 1;
	double f1 = _sintable_table[angle2];
	double f2 = _sintable_table[angle2+1];
	angle &= (1<<(24-SINTABLE_WIDTH_BITS)) - 1;
	f2 = (f2 - f1) * angle;
	f2 = ldexp(f2, -(24-SINTABLE_WIDTH_BITS));
	return f1 + f2;
}
double fixed_cos2(int angle) { 
	angle += (1 << (24 - 2));
	return fixed_sin2(angle);
}

CALL_BEFORE_MAIN(init_sintable);

