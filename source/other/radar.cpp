/* $Id$ */ 
#include <allegro.h>
#include "../melee.h"
REGISTER_FILE
#include "../id.h"
#include "../scp.h"
#include "../frame.h"
#include "../ship.h"

#include "../melee/mgame.h"
#include "../melee/mview.h"
#include "../melee/mcbodies.h"
#include "../melee/manim.h"
#include "../melee/mship.h"

#include "radar.h"

void ZRadar::toggleActive()
{
	active^=1;
}

void ZRadar::setSize(double Size)
{
	size=Size;
}

void ZRadar::setTarget(SpaceLocation *target)
{
	t=target;
}

ZRadar::~ZRadar()
{
	destroy_bitmap(Painted);
}

ZRadar::ZRadar(BITMAP *BlankSlate, Presence *target, double Size)
{
	Blank=BlankSlate;
	Painted = create_bitmap_ex(bitmap_color_depth(screen),Blank->w,Blank->h);
	t=target;
	size=Size;
	active=TRUE;
	set_depth(DEPTH_STARS + 0.1);
}

double ZRadar::shiftscale(double r_center, double v_center, double scale, double n)
{
	//Used to scale game coordinates onto RADAR screen coordinates
	return(((n - r_center)*scale)+v_center);
}

void ZRadar::animate(Frame *space)
{
	STACKTRACE

	//If the radar is disabled, don't do anything.
	if(active==FALSE) return;

	//Tell the frame to redraw this space
	space->add_box(0,0,Blank->w,Blank->h);

	//Copy the blank slate onto the temporary bitmap Painted
	if (Blank) blit(Blank, Painted, 0,0,0,0,Blank->w, Blank->h);
	else clear(Painted);

	if(t!=NULL)
	{
		SpaceLocation *l = t->get_focus();
		Paint(Painted,l->pos);
		if(!t->exists()) t=NULL;
	}
	else		Paint(Painted, game->size/2.);

	//Copy Painted onto space->frame, which will then paint it on the screen.
	blit(Painted,space->surface,0,0,0,0,Blank->w,Blank->h);
}

//The default RADAR.  Shows ships, planets, and asteroids.
void ZRadar::Paint(BITMAP *Slate, Vector2 T)
{
	STACKTRACE

	for(int num=0; num<physics->num_items; num++)
	{
		int xpos=0,ypos=0;

		SpaceLocation *o=physics->item[num];
		double Scale = Slate->w/(2.*size);

		xpos=(int)shiftscale(T.x,Slate->w/2,Scale,o->pos.x);
		ypos=(int)shiftscale(T.y,Slate->w/2,Scale,o->pos.y);

		if(o->isShip())			circle(Slate,xpos,ypos,2,makecol(255,0,0));
		else if(o->isAsteroid())	circlefill(Slate,xpos,ypos,1,makecol(174,131,66));
		else if(o->isPlanet())		circlefill(Slate,xpos,ypos,4,makecol(200,200,200));
	}
}
