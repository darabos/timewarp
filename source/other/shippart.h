#ifndef __SHIPPART__
#define __SHIPPART__

#include "../ship.h"


void removefromtargetlist(SpaceObject *o);
bool isintargetlist(SpaceObject *o);


class BigShipPart;


class BigShip : public Ship
{
protected:
	int Nparts;
	BigShipPart **parts;

public:

	BigShip(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	
	virtual void animate(Frame *space);
	virtual void calculate();

	virtual void change_vel(Vector2 dvel);
	virtual void change_pos(Vector2 dpos);
};


class BigShipPart : public Ship
{

protected:


	Vector2 oldpos;
	Vector2 oldvel;

	Ship *owner;
	SpaceSprite *sprite_uncrewed;

public:

	Vector2	relpos;
	double	relangle;

	Vector2 relposrot;
	SpaceObject *collider;

	BigShipPart(Ship *aowner, Vector2 orelpos, double orelangle,
				SpaceSprite *spr, SpaceSprite *spr_uncrewed);

	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);
	virtual void inflict_damage(SpaceObject *other);

	virtual void syncpos();
	virtual bool isdisabled();

	virtual void change_vel(Vector2 dvel);
	virtual void change_pos(Vector2 dpos);
};




class BigShipPartDevice : public SpaceObject
{
protected:
	BigShipPart *ownerpart;

public:
	BigShipPartDevice(BigShipPart *aownerpart, SpaceSprite *ospr);
	virtual void calculate();
	virtual void animate(Frame *space);
};


#endif
