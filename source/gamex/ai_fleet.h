#ifndef __GAMEX_FLEETAI__
#define __GAMEX_FLEETAI__


#include "../ais.h"


class FleetFormation
{
};

class FleetAI
{
 protected:
	virtual void attack(Vector2 pos);
};




class ControlHumanFG : public ControlHuman
{
public:
	ControlHumanFG (const char *name);
	virtual void calculate();
};


class ControlWussieFG : public ControlWussie
{
public:
	ControlWussieFG (const char *name);
	virtual void calculate();
};



#endif // __GAMEX_FLEETAI__


