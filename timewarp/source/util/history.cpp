#include <stdlib.h>
#include <math.h>
#include "history.h"


////////////////////////////////////////////////////////////////////////
//				Histograph stuff
////////////////////////////////////////////////////////////////////////

/*

X axi is "max"
Y axi is # of samples
This shows memory used, in bytes, not counting heap overhead.  
Numbers are estimated.  


 \  max   10      100     1000    10k     100k    1M
samples
10^1      0.04k   0.4k    4k      40k     400k    4M
10^2      0.16k   0.4k    4k      40k     400k    4M
10^3      0.28k   1.6k    4k      40k     400k    4M
...
10^6      0.68k   5.6k    44k     280k    1.6M    4M
10^9      1.08k   9.6k    84k     680k    5.6M    44M
10^12     1.48k   13.6k   124k    1080k   9.6M    84M
10^15     1.88k   17.6k   164k    1480k   13.6M   124M
10^18     2.28k   21.6k   204k    1880k   17.6M   164M
*/

Histograph::Histograph(Uint16 max) {
	num = 0;
	base = 0;
	this->max = max;
	element = new HISTOGRAPH_ELEMENT_TYPE[max];
	next = NULL;
}
Histograph::~Histograph() {
	if (next) delete next;
	delete element;
}
void Histograph::add_element(double v) {
	_add(v);
}
void Histograph::_add(double v) {
	int i;
	if (num == max) {
		num -= next_ratio;
		double sum = 0;
		if (base <= next_ratio) {
			for (i = base - 1; i >= 0; i -= 1) {
				sum += element[i];
			}
			for (i = base - next_ratio + max; i < max; i += 1) {
				sum += element[i];
			}
		}
		else {
			for (i = base - next_ratio; i < base; i += 1) {
				sum += element[i];
			}
		}
		if (!next) next = new Histograph(max);
		next->_add(sum / next_ratio);
	}
	if (base == 0) base = max;
	base -= 1;
	element[base] = (HISTOGRAPH_ELEMENT_TYPE) v;
	num += 1;
	return;
}
double Histograph::_get (Sint64 back) const {
	if (back < num) {
		int i = base + (int)back;
		if (i >= max) i -= max;
		return element[i];
	}
	if (!next) return 0;
	back -= num;
//	double a = next->_get ( 1 + back/next_ratio );
	double b = next->_get ( back/next_ratio );
	return b;
}
double Histograph::get_element (Sint64 back) const {
	return _get(back);
}
double Histograph::get_average (double back1, double back2) const {
	if (back1==back2) return get_element((Sint64)back1);
	return get_integral(back1,back2)/(back2-back1);
}
double Histograph::get_integral (double back1, double back2) const {
	if (back1 == back2) return 0;
	else if (back1 > back2) return -1 * get_integral(back2, back1);
	if (back1 < 0) back1 = 0;
	if (back2 < 0) back2 = 0;
	return _get_integral(back1, back2);
}
double Histograph::_get_integral (double back1, double back2) const {
	int i;
	int bi1 = (int) back1;
	if (bi1 >= num) {
		if (!next) return 0;
		return next_ratio * next->_get_integral ( 
			(back1 - num) / next_ratio, 
			(back2 - num) / next_ratio
		);
	}
	int bi2 = (int)ceil(back2) - 1;
	if (bi1 == bi2) {
		return _get(bi1) * (back2 - back1);
	}
	double sum;
	sum = _get(bi1) * (1.0 - (back1 - bi1));
	if (bi2 >= num) {
		for (i = bi1+1; i < num; i += 1) {
			sum += _get(i);
		}
		if (!next) return sum;
		return sum + next_ratio * next->_get_integral ( 
			0, 
			(back2 - num) / next_ratio
		);
	}
	for (i = bi1+1; i < bi2; i += 1) {
		sum += _get(i);
	}
	return sum + _get(bi2) * (back2 - bi2);
}




