/* $Id$ */ 

#include "mtarget.h"


Targets *targets;


Targets::Targets()
{
	N = 0;
	item = 0;
}


Targets::~Targets()
{
	reset();
}

void Targets::reset()
{
	if (item)
		free(item);
	item = 0;
	N = 0;
}


void Targets::add(SpaceObject *a) {STACKTRACE
	N += 1;
	item = (SpaceObject **) realloc(item, sizeof(SpaceObject *) * N);
	item[N - 1] = a;
	a->attributes |= ATTRIB_TARGET;
}


void Targets::rem(int i)
{
	-- N;
	item[i]->attributes &= ~ATTRIB_TARGET;
	item[i] = item[N];
}


void Targets::rem(SpaceObject *r)
{
	int i;
	for ( i = 0; i < N; ++i )
		if (item[i] == r)
			break;

	if (i == N)
		return;


	rem(i);
}


void Targets::calculate()
{
	int i;
	
	for (i = 0; i < N; i += 1)
	{
		if (!item[i]->exists())
		{
			rem(i);
			-- i;
		}
	}
}




int Targets::findindex(SpaceObject *o)
{
	int i;

	for (i = 0; i < N; i += 1)
	{
		if (item[i] == o)
		{
			return i;
		}
	}

	return -1;
}


bool Targets::isintargetlist(SpaceObject *o)
{
	return findindex(o) >= 0;
}
