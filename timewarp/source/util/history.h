#ifndef _HISTORY_H
#define _HISTORY_H

#ifndef _TYPES_H
#include "types.h"
#endif

/*

Histograph "max"		Memory Usage
					Minimum		Typical	Maximum
								
10					64 bytes	0.8 k	2 kbytes
100					416 bytes	7.6 k	22 kbytes
1,000				4 kbytes	60  k	200 kbytes
10,000				40 kytes	480 k	1.8 Mbytes
100,000				400 kbytes	3.6 M	17.5 Mbytes
1,000,000			4 Mbytes	24  M	164 Mbytes

That table assumes that floats are used for the elements.  
If doubles are used instead, double all the memory usage numbers.  
This tables also assumes that next_ratio is 2 (highly recommended).  

Histograph should be able to handle at least 2**52 samples.  
If "max" is at least 2,048 then it can probably handle a 
full 2**63 samples.  The maximum memory consumption is an 
estimate based upon about 2**60 samples.  Typical memory 
consumption is based up 10**7.5 (~30 million) samples.  The 
numbers listed are not precise.  

If "max" is a power of 2 or slightly less, the heap manager may 
be more efficient when allocating memory for a Histograph.  
Inefficiencies in the heap manager might increase effective memory 
consumption by a factor of 2.  

*/

#define HISTOGRAPH_ELEMENT_TYPE float
//#define HISTOGRAPH_ELEMENT_TYPE double

class Histograph {
	public:
		Histograph ( Uint16 max = 250 );
		~Histograph ();
		void add_element    ( double v );
		double get_element  ( Sint64 back ) const;
		double get_average  ( double back1, double back2 ) const;
		double get_integral ( double back1, double back2 ) const;
	private:
		void _add ( double v );
		double _get ( Sint64 back ) const;
		double _get_integral (double back1, double back2) const;

		Uint16 num;        // 0 <= num < max
		Uint16 max;        // 0 < max, recommended 8 <= max
		Uint16 base;       // 0 <= base < max
		Uint16 _filler;//just for alignment
		enum { next_ratio = 2 };

		HISTOGRAPH_ELEMENT_TYPE *element;
		Histograph *next;
};

#endif