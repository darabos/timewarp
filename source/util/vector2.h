#ifndef _VECTOR2_H
#define _VECTOR2_H

class Vector2;
class Vector2i;

class Vector2 {
public:
	double x, y;
	Vector2 () {}
	Vector2 (double a, double b) : x(a), y(b) {}
	Vector2 (Vector2i v2i);
	Vector2 (_Ignore_Me *nothing) : x(0), y(0) {}
	Vector2 &operator= (_Ignore_Me *nothing) {x = y = 0; return *this;}
	Vector2 &operator+=(Vector2 a) { x+=a.x;y+=a.y; return *this;}
	Vector2 &operator-=(Vector2 a) { x-=a.x;y-=a.y; return *this;}
	Vector2 &operator*=(double a) { x*=a; y*=a; return *this;}
	Vector2 &operator/=(double a) { double b = 1/a; x*=b; y*=b; return *this;}
	Vector2 operator-() const { return Vector2(-x,-y); }
	bool operator==(Vector2 a) const { return ((x==a.x) && (y==a.y)); }
	bool operator!=(Vector2 a) const { return ((x!=a.x) || (y!=a.y)); }

	Vector2i round();
	Vector2i truncate();

	double length() const;
	double abs() const {return length();}//just another name
	double magnitude() const {return length();}//just another name

	double angle() const;
	double atan() const {return angle();}//just another name

	double dot(Vector2 other) const {return x*other.x+y*other.y;}
	double dot_product(Vector2 other) const {return dot(other);}//just another name

	Vector2 product ( Vector2 other ) const {
		return Vector2 ( x * other.x, y * other.y );
	}
	Vector2 operator* ( Vector2 other) const {return product(other);}

	Vector2 complex_multiply ( Vector2 other) const {return Vector2(x*other.x-y*other.y,x*other.y+y*other.x);}
	Vector2 complex_divide   ( Vector2 other) const;
	//treats vectors as complex numbers

	Vector2 rotate(double angle) const ;
};

inline Vector2 operator+ (Vector2 a, Vector2 b) { return Vector2(a.x+b.x,a.y+b.y); }
inline Vector2 operator- (Vector2 a, Vector2 b) { return Vector2(a.x-b.x,a.y-b.y); }
inline Vector2 operator* (Vector2 a, double b) { return Vector2(a.x*b,a.y*b); }
inline Vector2 operator* (double b, Vector2 a) { return Vector2(a.x*b,a.y*b); }
inline Vector2 operator/ (Vector2 a, double b) { double c = 1/b; return Vector2(a.x*c,a.y*c); }

inline double magnitude_sqr ( Vector2 vect ) { return (vect.x*vect.x+vect.y*vect.y); }
//returns the square of the magnitude of vect (fast)

inline double magnitude ( Vector2 v ) {return v.length();}
inline double length (Vector2 v) {return v.length();}
inline double abs (Vector2 v) {return v.length();}
//returns the magnitude of vect

inline double atan (Vector2 a) {return a.angle();}

Vector2 unit_vector       ( double angle );
//returns a unit vector at a particular angle
inline Vector2 unit_vector( Vector2 vec ) {return vec / vec.length();}
//returns a unit vector with the same angle as vec


inline Vector2 product ( Vector2 a, Vector2 b) {return a.product(b);}

inline double dot_product ( Vector2 a, Vector2 b) {return a.dot(b);}
//returns the dot product of vector a with vector b

inline Vector2 complex_multiply ( Vector2 a, Vector2 b) {return a.complex_multiply(b);}
inline Vector2 complex_divide   ( Vector2 a, Vector2 b) {return a.complex_divide(b);}

inline Vector2 rotate ( Vector2 v, double angle ) {return complex_multiply( v, unit_vector(angle));}
//rotates vector v by angle angle

Vector2 tw_random ( Vector2 max ) ;
inline Vector2 tw_random ( Vector2 min, Vector2 max ) {return tw_random(max-min) + min;}
//returns a random point within the rectangle with corners (0,0) and max

/*  Thus:

double fa, fb, fc;
Vector2 va, vb, vc, vd;

va = Vector2(3, 4);
fa = sqrt(3 * 3 + 4 * 4);

if (magnitude(va) != fa) an_error_occurred();
if (magnitude_sqr(va) != 5*5) an_error_occurred();
if (atan(va) != atan3(4, 3)) an_error_occurred();

vb = unit_vector ( va );
vc = unit_vector ( atan( va ) );

if (vb != vc) an_error_occurred();
if (magnitude(vb) != 1) an_error_occurred();

...

fa = dot ( va, vb );
fb = atan ( va ) - atan ( vb );
vd = complex_multiply ( va, vb );

if ( magnitude(vc) != fa * cos(fb) ) an_error_occurred();
if ( magnitude(vc) != magnitude(va) * magnitude(vb) ) an_error_occurred();
if ( atan ( vc ) != atan(va) + atan(vb) ) an_error_occurred();

...

*/

class Vector2i {
public:
	int x, y;
	Vector2i () {}
	Vector2i (int a, int b) : x(a), y(b) {}
	Vector2i (_Ignore_Me *nothing) : x(0), y(0) {}
	Vector2i &operator= (_Ignore_Me *nothing) {x = y = 0; return *this;}
	Vector2i &operator+=(Vector2i a) { x+=a.x;y+=a.y; return *this;}
	Vector2i &operator-=(Vector2i a) { x-=a.x;y-=a.y; return *this;}
//	Vector2i &operator*=(int a) { x*=a; y*=a; return *this;}
//	Vector2i &operator/=(int a) { x/=a; y/=a; return *this;}
	Vector2i operator-() const { return Vector2i(-x,-y); }
	bool operator==(Vector2i a) const { return ((x==a.x) && (y==a.y)); }
	bool operator!=(Vector2i a) const { return ((x!=a.x) || (y!=a.y)); }

/*	Vector2i product ( Vector2i other ) const {
		return Vector2i ( x * other.x, y * other.y );
	}
	Vector2i operator* ( Vector2i other) const {return product(other);}*/
};


#endif
