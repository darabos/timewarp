#include <stdio.h>
#include "../ship.h"
#include "../melee/mview.h"
//#include "link.h"
REGISTER_FILE

inline double sqr(double x)
{
	return x*x;
}

//This is the base class for CargoLink and Chain.
//It is nothing but a basic SpaceObject with pointers to the next
//and previous links.
class Link : public SpaceObject
{
public:
	Link(SpaceLocation *creator, Vector2 opos, double oangle,
		SpaceSprite *osprite, SpaceObject *Prev_Object,
		SpaceObject *Next_Object);
	Link *Next_Link;
	Link *Prev_Link;
};

Link::Link(SpaceLocation *creator, Vector2 opos, double oangle,
	SpaceSprite *osprite, SpaceObject *Prev_Object,
	SpaceObject *Next_Object):SpaceObject(creator,
	opos,oangle,osprite)
{
	Prev_Link=(Link *)Prev_Object;
	Next_Link=(Link *)Next_Object;
}

//A CargoLink is pretty much the same, except it rotates and has mass.
//Needs 64 images.
class CargoLink : public Link
{
public:
	CargoLink(SpaceLocation *creator, Vector2 opos, double oangle,
		SpaceSprite *osprite, SpaceObject *Prev_Object,
		SpaceObject *Next_Object, double omass);
	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

class Chain : public Link
{
	void ChainPhysics(SpaceObject *first, SpaceObject *second);
	void ChainRecur(Link *other, int num);
	virtual void calculate();
	double Link_Distance;
public:
	Chain(SpaceLocation *creator, Vector2 opos, double oangle,
	SpaceSprite *osprite, int oLinks, double ospacing, double omass);
	void Uncouple();
	int handle_damage(SpaceLocation *source, double normal, double direct);
	void Pull_Last_Link();
};




int CargoLink::handle_damage(SpaceLocation *source, double normal, double direct)
{
	int totalDamage=normal+direct;

	if(totalDamage==0) return 0;

	//If there are no previous links, the link is destroyed.
	if(Prev_Link==NULL)
	{
		state=0;
		return totalDamage;
	}

	//If a previous link exists, the damage is passed to it.
	//directDamage is used so that we don't play the damage sound
	//again.

	return damage(Prev_Link, 0, totalDamage);
}

void CargoLink::calculate()
{
	Link::calculate();
	sprite_index = get_index(angle);
	if (Prev_Link && !Prev_Link->exists()) Prev_Link = NULL;
}


CargoLink::CargoLink(SpaceLocation *creator, Vector2 opos, double oangle,
		SpaceSprite *osprite, SpaceObject *Prev_Object,
		SpaceObject *Next_Object,double omass):Link(creator,opos,oangle,osprite,Prev_Object,Next_Object)
{
	mass=omass;
	layer = LAYER_SPECIAL;

}






void Chain::Pull_Last_Link()
{
	Link *l=this;

	//Seek to the end of the list
	while(l->Next_Link!=NULL) l=l->Next_Link;

	//decelerate the last link
	l->vel *= 0.8;
}

int Chain::handle_damage(SpaceLocation *source, double normal, double direct)
{
	return damage(Prev_Link, 0, normal+direct);
}

void Chain::Uncouple()
{
	for(Link *l=this->Next_Link; l!=NULL; l=l->Next_Link)
	{
		l->Prev_Link->Next_Link=NULL;
		l->Prev_Link=NULL;
	}
	state=0;
}


Chain::Chain(SpaceLocation *creator, Vector2 opos, double oangle,
	SpaceSprite *osprite, int olinks,double ospacing, double omass):Link(creator,opos,
	oangle,osprite,NULL,NULL)
{
	Prev_Link=(Link *)creator;	//Attach the first link to the ship

	mass=omass;

	Link *Cur_Link=this;		//Move this pointer to the first link
	Link_Distance=ospacing;

	//Each new link is placed (ospacing) far away from the previous,
	//forming an angled line.
	Vector2 dd = - Link_Distance * unit_vector(oangle);


	//Loop that creates all the other links
	for(int num=1; num<olinks; num++)
	{
		//Calculate position of new link
		Vector2 ppos = dd*num + opos;

		//Create a new link that knows that it's attached to Cur_Link
		Cur_Link->Next_Link = new CargoLink(creator,ppos,oangle,osprite,
			Cur_Link,NULL,omass);

		//Add it to the game
		add(Cur_Link->Next_Link);

		//Move the pointer to the new link
		Cur_Link=Cur_Link->Next_Link;
	}
}

//A recursuve function, the most efficient way to go thru a linked list
//backwards.
void Chain::ChainRecur(Link *other,int num)
{
	//Call ChainRecur for the next link UNLESS you're at the last link
	if(other->Next_Link!=NULL) ChainRecur(other->Next_Link,num+1);

	//The current link is jerked to a halt by the previous link.
	//If the first link ain't budging, CHECK THE MASS!
	ChainPhysics(other->Prev_Link,other);
}

void Chain::calculate()
{
	if(!ship->exists())
	{	//If the mothership is killed, the chain dies.
		Uncouple();		//Shatters chain, links become independent
		state=0;
		return;
	}

	ChainRecur(this,0);	//Do the physics for the chain

	sprite_index = get_index(angle);


	SpaceObject::calculate();	//Let the base class do it's stuff
}

void Chain::ChainPhysics(SpaceObject *first, SpaceObject *second)
{


	// first = towards the head, second = towards the tail

	if((first==NULL)||(second==NULL))	return;
	
	double a = second->trajectory_angle((SpaceLocation *)first);	// arg - class
	double b = first->angle;//trajectory_angle((SpaceLocation *)second);

	double da = b - a;
	if (da >  PI) da -= PI2;
	if (da < -PI) da += PI2;


	double maxangle = 30 * ANGLE_RATIO;

	if ( da > maxangle )
		da -= maxangle;		// overshoot correction
	else
	if ( da < -maxangle )
		da += maxangle;
	else
		da = 0;
	
	a += da * 0.1;	// small correction for overshoot, *0.1 to make it _much_ more smooth

	angle = a;
	
	//Jerking the link back into place if it's gone too far
//	first->pos = second->pos - Link_Distance*unit_vector(a);
	
	first->pos = second->pos + Link_Distance*unit_vector(a);
	//second->angle = a;
	second->angle = a - 0.5 * da; 
	first->angle = b - 0.5 * da;
	// the nodes are already updated at the back (second) - and values are updated
	// forward.
	// Yeah! This is a great source for weird behaviour :)


	// if such a "limit" occurs, you've extra "force" on the ends ...
	// accelerations ?! velocities ?!


	/*
	//Calculate final velocity of both using the Law of
	//Conservation of Momentum,
	//vfinal=(m1*v1+m2*v2)/(m1+m2)
	Vector2 v_f = (first->mass*first->vel + second->mass*second->vel)/(first->mass+second->mass);
	
	  first->vel = v_f;
	  second->vel = v_f;
	  // this is wrong, since it won't allow rotation
	*/
	
	// right - this method doesn't treat freedom of rotation properly.
	// but is needed to make it look more real. The velocities in the direction of
	// the connection should be set equal.
	
	Vector2  L;
	L = unit_vector(a);
	
	double v1, v2;
	v1 = L.dot(first->vel);
	v2 = L.dot(second->vel);
	// the velocities projected onto the bar
	
	//Conservation of Momentum,
	// as above, but now only in the direction of the bar:
	double vfinal;
	vfinal = ( first->mass * v1 + second->mass * v2) / (first->mass + second->mass);
	
	first->vel += (vfinal - v1) * L;
	second->vel += (vfinal - v2) * L;

		
	// accelerations of "second" due to centripetal force on either side :

	Vector2 accel;
	double v;

	accel = 0;

	v = magnitude( first->vel - L*first->vel.dot(L) );	// the perpendicular component
	accel += L * (first->mass / second->mass) * sqr(v) / Link_Distance;

	SpaceObject *third;

	third = this->Next_Link;
	//third = 0;
	if ( third != 0 )
	{
		Vector2 L2;
		L2 = unit_vector(third->angle);

		v = magnitude( third->vel - L2*third->vel.dot(L2) );	// the perpendicular component
		accel -= L2 * (third->mass / second->mass) * sqr(v) / Link_Distance;
	}

	// apply these accelerations:

	second->vel += 1E-3 * accel * frame_time;


	
	// By default, let the element become neutral in rotational velocity by
	// including drag in the rotation direction.

	second->vel -= 0.2 * frame_time*1E-3 * (second->vel - L*second->vel.dot(L));

}


class ConfedCargotran : public Ship {
	int	weaponColor;
	double	weaponRange;
	int	weaponDamage;

	double	specialMass;
	int	specialLinks;
	double	specialSpacing;

	public:

	Chain *Head;

	ConfedCargotran(Vector2 opos, double angle, ShipData *data, unsigned int code);
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void animate(Frame *space);
};



ConfedCargotran::ConfedCargotran(Vector2 opos, double angle, ShipData *data, unsigned int code)
	:
	Ship(opos, angle, data, code)

{
	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage = get_config_int("Weapon", "Damage", 0);

	specialLinks = get_config_int("Special","Links",0);
	specialMass  = get_config_float("Special","LinkMass",0);
	specialSpacing=get_config_float("Special","LinkDistance",0);

	add(Head = new Chain(this,pos,angle,data->spriteSpecial,specialLinks,specialSpacing,specialMass));
}

int ConfedCargotran::activate_weapon()
{
	if(fire_special) return(FALSE);

	//Add weapon code here
	add(new Laser(this, angle,
	0x00ff2222, 100, 1, 1,
	this, Vector2(0, (size.y / 2.07)), true));

	return(TRUE);
}

int ConfedCargotran::activate_special()
{
	//Cuts loose all links and remembers that it did so.
	if(Head==NULL)	return(FALSE);
	else if(fire_weapon)
	{
		CargoLink *l;

		add(l=new CargoLink(this,Head->pos,Head->angle,data->spriteSpecial,NULL,NULL,specialMass));

		l->vel = Head->vel;

		Head->Uncouple();
		Head=NULL;
		return(TRUE);
	}
	else
	{
		Head->Pull_Last_Link();
		return(FALSE);
	}
}

void ConfedCargotran::animate(Frame *space)
{
	double back_x=size.x/3.60, back_y=-size.y/2.33,
	       frnt_x=size.x/5.55, frnt_y=+size.y/17.01,
	       back_y_1=-size.y/2.06;

	double tx = cos(angle);
	double ty = sin(angle);
	int s_index = get_index(angle);

	if (turn_right)
		data->spriteWeapon->animate(Vector2(pos.x + frnt_y * tx - frnt_x * ty,
		     pos.y + frnt_y * ty + frnt_x * tx), s_index + ((rand()%3) << 6), space);
	if (turn_left)
		data->spriteWeapon->animate(Vector2(pos.x + frnt_y * tx + frnt_x * ty,
		     pos.y + frnt_y * ty - frnt_x * tx), s_index + ((rand()%3) << 6), space);

	s_index += 32; s_index &= 63;

	if (thrust) {
		data->spriteExtra->animate(Vector2(pos.x + back_y_1 * tx - back_x * ty,
		     pos.y + back_y_1 * ty + back_x * tx), s_index + ((rand()%3) << 6), space);
		data->spriteExtra->animate(Vector2(pos.x + back_y_1 * tx + back_x * ty,
		     pos.y + back_y_1 * ty - back_x * tx), s_index + ((rand()%3) << 6), space); }
	else {
		if (turn_left)
			data->spriteWeapon->animate(Vector2(pos.x + back_y * tx - back_x * ty,
			     pos.y + back_y * ty + back_x * tx), s_index + ((rand()%3) << 6), space);
		if (turn_right)
			data->spriteWeapon->animate(Vector2(pos.x + back_y * tx + back_x * ty,
			     pos.y + back_y * ty - back_x * tx), s_index + ((rand()%3) << 6), space); }

	Ship::animate(space);
};


REGISTER_SHIP(ConfedCargotran)
