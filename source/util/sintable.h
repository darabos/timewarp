
#include "round.h"

#define SINTABLE_WIDTH_BITS 8


extern float _sintable_table[(1 << SINTABLE_WIDTH_BITS) + 1];

inline double fixed_sin(int angle) { 
	int angle2 = angle >> (24 - SINTABLE_WIDTH_BITS);
	angle2 &= (1 << SINTABLE_WIDTH_BITS) - 1;
	float f1 = _sintable_table[angle2];
	return f1;
}
inline double fixed_cos(int angle) { 
	int angle2 = angle >> (24 - SINTABLE_WIDTH_BITS);
	angle2 += (1 << (SINTABLE_WIDTH_BITS - 2));
	angle2 &= (1 << SINTABLE_WIDTH_BITS) - 1;
	float f1 = _sintable_table[angle2];
	return f1;
}
double fixed_sin2(int angle);
double fixed_cos2(int angle);

inline double fast_sin ( double angle ) {
	return fixed_sin(iround(angle * ((1 << 23) / 3.14159265358979323)));
}
inline double fast_cos ( double angle ) {
	return fixed_cos(iround(angle * ((1 << 23) / 3.14159265358979323)));
}
inline double fast_sin2 ( double angle ) {
	return fixed_sin2(iround(angle * ((1 << 23) / 3.14159265358979323)));
}
inline double fast_cos2 ( double angle ) {
	return fixed_cos2(iround(angle * ((1 << 23) / 3.14159265358979323)));
}


