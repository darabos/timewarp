
#ifndef __TRMATH__
#define __TRMATH__



#include "trtest.h"


class action_addI : public action_func
{
	int *idest, *isrc;
public:
	virtual action_func *create() {return new action_addI(); };

	virtual void define();
	virtual void calculate();
};


class action_subI : public action_func
{
	int *idest, *isrc;
public:
	virtual action_func *create() {return new action_subI(); };

	virtual void define();
	virtual void calculate();
};


class action_mulI : public action_func
{
	int *idest, *isrc;
public:
	virtual action_func *create() {return new action_mulI(); };

	virtual void define();
	virtual void calculate();
};


class action_divI : public action_func
{
	int *idest, *isrc;
public:
	virtual action_func *create() {return new action_divI(); };

	virtual void define();
	virtual void calculate();
};




#endif


