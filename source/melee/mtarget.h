
#ifndef _MTARGET_H_
#define _MTARGET_H_


#include "../melee.h"
#include "mframe.h"

class Targets
{
public:

	Targets();
	virtual ~Targets();

	int N;

	SpaceObject **item;

	virtual void add (SpaceObject *a);

	virtual void rem(int i);
	virtual void rem(SpaceObject *r);

	virtual void calculate();

	virtual void reset();


	int findindex(SpaceObject *o);
	bool isintargetlist(SpaceObject *o);
};

extern Targets *targets;


#endif
