
#ifndef __TRSHIPS__
#define __TRSHIPS__



#include "trtest.h"


class action_playership : public action_func
{
	char **shipname;
	int *ix, *iy, *team;
public:
	virtual action_func *create() {return new action_playership(); };

	virtual void define();
	virtual void calculate();
};



class action_AIship : public action_func
{
	char **shipname;
	int *ix, *iy, *team;
public:
	virtual action_func *create() {return new action_AIship(); };

	virtual void define();
	virtual void calculate();
};



class action_newteam : public action_func
{
	int *team;
public:
	virtual action_func *create() {return new action_newteam(); };

	virtual void define();
	virtual void calculate();
};








#endif


