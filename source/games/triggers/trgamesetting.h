
#ifndef __TRGAMESETTING__
#define __TRGAMESETTING__



#include "trtest.h"


class action_mapsize : public action_func
{
	int *sizex, *sizey;
public:
	virtual action_func *create() {return new action_mapsize(); };

	virtual void define();
	virtual void calculate();
};


class action_planet : public action_func
{
	int *x, *y;
public:
	virtual action_func *create() {return new action_planet(); };

	virtual void define();
	virtual void calculate();
};





#endif


