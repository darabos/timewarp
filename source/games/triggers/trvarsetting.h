
#ifndef __TRVARSETTING__
#define __TRVARSETTING__



#include "trtest.h"


class action_setI : public action_func
{
	int *idest, *isrc;
public:
	virtual action_func *create() {return new action_setI(); };

	virtual void define();
	virtual void calculate();
};


class action_setS : public action_func
{
	char **idest, **isrc;	// pointer to a string
public:
	virtual action_func *create() {return new action_setS(); };

	virtual void define();
	virtual void calculate();
};


class action_convID : public action_func
{
	int *idest;
	double *dsrc;
public:
	virtual action_func *create() {return new action_convID(); };

	virtual void define();
	virtual void calculate();
};






#endif


