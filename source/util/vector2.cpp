#include <math.h>
#include "base.h"
#include "vector2.h"
#include "random.h"
#include "round.h"
#include "../melee.h"

Vector2::Vector2(Vector2i v2i) : x(v2i.x), y(v2i.y) {}
Vector2i Vector2::round()
{
	return Vector2i(iround(x), iround(y));
}
Vector2i Vector2::truncate()
{
	return Vector2i(int(x), int(y));
}

double Vector2::length () const {
	return sqrt(magnitude_sqr(*this));
}

double Vector2::angle() const {
	if (*this == 0) return 0;
	double a = atan2(y, x);
	if (a < 0) a += PI2;
	return a;
}

Vector2 Vector2::complex_divide(Vector2 other) const {
	other.y *= -1;
	return complex_multiply(other) / magnitude_sqr(other);
}

Vector2 Vector2::rotate(double angle) const {
	return complex_multiply(unit_vector(angle));
}

Vector2 tw_random ( Vector2 max ) {
	return Vector2(tw_random(max.x), tw_random(max.y));
}

Vector2 unit_vector ( double angle ) {
	Vector2 r;
//stupid hack that assumes that VisualC always compiles for i386
//because I can't figure out how to detect this

#	if (defined _MSC_VER) && !(defined __ia32__)
#		define __ia32__
#	endif

//now the normal processor detection
//and various platform specific vesions

#	if (defined i386) || (defined __ia32__) || (defined __i386__)
#		if defined __GNUC__
#			define ASM_SINCOS
			asm ("fsincos" : "=t" (r.x), "=u" (r.y) : "0" (angle));
#		elif defined _MSC_VER
#			define ASM_SINCOS
			__asm fld angle
			__asm fsincos
			__asm fstp r.x
			__asm fstp r.y
#		endif
#	endif

//and the fall-back version in C

#	ifndef ASM_SINCOS
		r.x = cos(angle);
		r.y = sin(angle);
#	endif
	return r;
}


Vector2 unit_vector( Vector2 vec )
{

	double L;
	L = vec.length();

	if (L > 0)
		return vec / L;

	else
		return Vector2(1,0);
	// GEO: added default (1,0) vector returned if vec=0, ie, if the
	// result is undetermined.
}

