#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"
#include "../frame.h"
#include "../util/aastr.h"


/* How does this ship work

  The ship has a main pea-shooter that does 1 damage by default.

  It also fires mines, which create a damaging field around their host when
  they attach themselves.

  The damaging field increases ANY damage that is being taken.

  Still to do:
  graphics.

*/


// Based on the Kzer-Za dreadnought and numerous other
// fragments of timewarp/ allegro code.



class KaboHaze : public SpaceObject
{
public:
	KaboHaze	*next, *prev;
	Ship		*mother, *host;
	double		power, basepower;
	double		decay_time;
	int			oldcrew, newcrew;
	int			*edge_left, *edge_right;

	BITMAP		*shield_bmp[64];

	KaboHaze(Ship *mother, Ship *ohost, double obasepower, double odecaytime);
	virtual void calculate();
	virtual void animate(Frame *space);
	//virtual void death();
	~KaboHaze();
};


// This keeps track of all hazes fired by all ships, and is located
// static in memory.

static KaboHaze *KaboHazeFirst = 0;



class KaboWeakener : public Ship {
public:
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	int    weaponArmour;

	int    specialFrames;
	double specialVelocity1, specialVelocity2;
	double specialRange;
	int    specialArmour;

	double	Mine_RangeAcc, Haze_basepower, Haze_decaytime;

	public:
	KaboWeakener(Vector2 opos, double angle, ShipData *data, unsigned int code);

	int activate_weapon();
	int activate_special();
	};



class KaboMine : public Missile {
	int    air_frames;
	int    max_air_frames;
	double spriteindextime;
	double rangeacc;
	double Haze_basepower, Haze_decaytime;
	double velocity_one, velocity_two;

	SpaceObject *thetarget;

	public:
	KaboMine(Vector2 opos, double oangle, double ov1, double ov2,
		double orange, int oarmour, int oair_frames, double orangeacc,  double obasepower, double odecaytime,
		Ship *oship, SpaceSprite *osprite);

	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	};




KaboWeakener::KaboWeakener(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code) 
	{

	// for the pea shooter:
	weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = get_config_int("Weapon", "Damage", 0);
	weaponArmour        = get_config_int("Weapon", "Armour", 0);

	// for the KaboMine:
	specialFrames          = get_config_int("Mine", "Frames", 0);
	specialVelocity1       = scale_velocity(get_config_float("Mine", "Velocity1", 0));
	specialVelocity2       = scale_velocity(get_config_float("Mine", "Velocity2", 0));
	specialArmour          = get_config_int("Mine", "Armour", 0);
	Mine_RangeAcc        = scale_range(get_config_float("Mine", "Range", 0));

	// for the haze:
	Haze_basepower =	get_config_float("Haze", "Power", 1.0);
	Haze_decaytime =	get_config_float("Haze", "DecayTime", 10000.0);

	}


int KaboWeakener::activate_weapon()
{
	add(new Missile(this, Vector2(0.0, 0.5*get_size().y),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}

int KaboWeakener::activate_special() {
	game->add( new KaboMine(Vector2(0.0, -0.5*get_size().y),
			180+angle, specialVelocity1, specialVelocity2, specialRange, specialArmour,
			specialFrames, Mine_RangeAcc, Haze_basepower, Haze_decaytime, this, data->spriteSpecial));
	return TRUE;
	}


KaboMine::KaboMine (Vector2 opos, double oangle, double ov1, double ov2,
	double orange, int oarmour, int oair_frames, double orangeacc, double obasepower, double odecaytime,
	Ship *oship, SpaceSprite *osprite) 
	:
	Missile(oship, opos, oangle, ov1, 0.1, orange, oarmour, oship, osprite),
	air_frames(oair_frames),
	max_air_frames(oair_frames),
	rangeacc(orangeacc),
	Haze_basepower(obasepower),
	Haze_decaytime(odecaytime),
	velocity_one(ov1),
	velocity_two(ov2)
	{
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	spriteindextime = 0;

	// find the closest enemy; that one will be the target !
	// otherwise, it'll sometimes shoot at the planet (doh)
	// the target must be a ship.
	// hmmmm ... for some reason, that doesnt help.

	thetarget = 0;

	Query q;
	double range = 10000;
	int qlayers = OBJECT_LAYERS;
	for (q.begin(this, qlayers, range); q.currento; q.next() )
	{
		if (!q.currento->isShip())
			continue;

		if (q.currento->ally_flag == ship->ally_flag)
			continue;

		// so ... who's the closest enemy?
		if (!thetarget)
			thetarget = q.currento;
		else
			if (q.currento->distance(this) < thetarget->distance(this))
				thetarget = q.currento;
	}

	if (!thetarget)
		state = 0;

}

void KaboMine::calculate() {

	if (!ship || !thetarget || !ship->exists() || !thetarget->exists()) {
		state = 0;
		return;
		}

	air_frames -= frame_time;
	if (air_frames <= 0) {
		state = 0;
		return;
		}


	// if close enough, they increase speed

	double L = distance(thetarget);
	if ( L > rangeacc )
		v = velocity_one;
	else
		v = velocity_two;


	// make the sprite/mines rotate around their axes
	spriteindextime += frame_time / 25.0;
	sprite_index = (int)(spriteindextime);
	sprite_index &= 63;

	// turn toward the target:
	if ( thetarget && !thetarget->isInvisible() )
	{
		angle = atan(min_delta(thetarget->pos, pos, map_size));
		vel = v * unit_vector(angle);
	}

	Missile::calculate();
}


int KaboMine::handle_damage(SpaceLocation *source, double normal, double direct)
{
	state = 0;
	return 0;
}

void KaboMine::inflict_damage(SpaceObject *other)
{

	if ( !other->isShip() )	// only attack ships !
	{
		state = 0;
		return;
	}
	
	// first check, if a haze already exists,
	// attached to the same "enemy"

	KaboHaze *haze;
	for ( haze = KaboHazeFirst ; haze != 0 ; haze = haze->next)
	{
		if ( other == haze->host )
			break;		// hey ! this ship is already targeted !!
	}

	if ( haze != 0 )	// hey ! this ship is already targeted !!
	{
		haze->power += haze->basepower;
	} else {
		// it's the first time a haze is applied to this ship ...
		haze = new KaboHaze( ship, (Ship*) other, Haze_basepower, Haze_decaytime);

		// update the list (insert at the start)
		KaboHaze *hazenext = KaboHazeFirst;
		if (hazenext)
			hazenext->prev = haze;
		haze->next = hazenext;
		haze->prev = 0;
		KaboHazeFirst = haze;

		game->add(haze);
	}

	// the seed mine has done it's job well, it can rest now ...
	state = 0;

	return;
}







void KaboHaze::calculate()
{

	if (!host || !host->exists())
	{
		state = 0;
		return;
	}

	// the following is used for drawing, really ; but has to be calculated here
	// because it's a globally accessible/ used variable.

	sprite_index = get_index(host->angle, 0.5*PI);	// + 16;
	sprite_index &= 63;

	// decrease strength over time ... and when strength = 0, then it should be removed

	if ( power <= 0.01 )	// too weak, the field loses integrity
		state = 0;	// CHANGE THIS BACK TO ZERO LATER !
	else
		power *= exp( - frame_time / decay_time );


	// check damage taken this turn ... increase that damage !

	oldcrew = newcrew;
	newcrew = host->getCrew();
	if ( newcrew < oldcrew  )
	{
		// add extra damage !!
		//... but how ... fractional, or what ??

		int extradeaths = int( (oldcrew - newcrew) * ( power - 1 ) );

		if ( extradeaths > 0 )
		{
			//this->damage_factor = extradeaths;
			//this->inflict_damage(host);		// that poor host !
			this->damage(host, extradeaths);
		}

		newcrew = host->getCrew();

		power *= 0.66;		// big powerdrain from the shield !


	}

}



void blit_singlecolor(BITMAP *src, BITMAP *dest, int copycolor)
{
	if ( src->w != dest->w || src->h != dest->h )
		tw_error("error in copying color in shpysr2");

	for ( int iy = 0; iy < src->h; ++iy )
	{
		for ( int ix = 0; ix < src->w; ++ix )
		{
			int color = getpixel(src, ix, iy);
			if ( color == copycolor )
				putpixel(dest, ix, iy, color);
		}
	}

}



void blit_resize( BITMAP* *bmpold, int wnew, int hnew)
{
	int wold = (*bmpold)->w;
	int hold = (*bmpold)->h;

	BITMAP *bmpnew = create_bitmap(wnew, hnew);

	stretch_blit(*bmpold, bmpnew, 0, 0, (*bmpold)->w, (*bmpold)->h, 0, 0, bmpnew->w, bmpnew->h);

	destroy_bitmap(*bmpold);

	*bmpold = bmpnew;	// point to the scaled bitmap !
}


// WHY DOES THIS TAKE SO LONG ?!?!
// an image of 100x100 pixels means,
// there's 10,000 calculations involved !
// no wonder.
void blit_blur(BITMAP *src, int R)
{
	int *blurmap;

	if ( R <= 2 )
		return;

	// first, create a sinusoid blur template

	blurmap = new int [R*R];

	double radius = R / 2.0;
	double xmid = radius;
	double ymid = radius;

	int ix, iy, k;

	k = 0;
	for ( iy = 0; iy < R; ++iy )
	{
		for ( ix = 0; ix < R; ++ix )
		{
			double x = ix+0.5 - xmid;	// 0.5, so that you're in the center of the pixel
			double y = iy+0.5 - ymid;
			double range = sqrt(x*x + y*y);
			if (range > radius) range = radius;
			blurmap[k] = int(255 * cos(0.5*PI * range/R));
			++k;
		}
	}

	// next, create a copy of the picture to which we write the result

	BITMAP *temp_bmp = create_bitmap(src->w, src->h);

	// next, apply the blur

	for ( iy = 0; iy < src->h; ++iy )
	{
		for ( ix = 0; ix < src->w; ++ix )
		{

			// create a new pixel at this position:

			int xblur, yblur;
			int newcolorR = 0;
			int newcolorG = 0;
			int newcolorB = 0;
			int totweight = 0;

			k = 0;
			for ( yblur = 0; yblur < R; ++yblur )
			{
				for ( xblur = 0; xblur < R; ++xblur )
				{
					int color = getpixel(src, ix+xblur-R/2, iy+yblur-R/2);
					if ( color != -1 )
					{
					
						int weight = blurmap[k];
						++k;
						
						newcolorR += getr(color) * weight;
						newcolorG += getg(color) * weight;
						newcolorB += getb(color) * weight;
						totweight += weight;
					}
				}
			}
			newcolorR /= totweight;
			newcolorG /= totweight;
			newcolorB /= totweight;
			
			int newcolor = makecol(newcolorR, newcolorG, newcolorB);
			putpixel(temp_bmp, ix, iy, newcolor);

		}
	}

	// and then, copy the blurred image on top of the old one, so that becomes blurred.

	blit(temp_bmp, src, 0, 0, 0, 0, src->w, src->h);



	destroy_bitmap(temp_bmp);

	delete[] blurmap;
}


//void KaboHaze::death()
KaboHaze::~KaboHaze()
{
	// update the pointers, remove this item from the list
	KaboHaze *p1, *p2;
	p1 = this->prev;
	p2 = this->next;

	if (p1)
		p1->next = p2;

	if (p2)
		p2->prev = p1;

	if ( KaboHazeFirst == this )
		KaboHazeFirst = p2;

	for ( int i = 0; i < 64; ++i )
		if ( shield_bmp[i] )
			destroy_bitmap(shield_bmp[i]);

	delete[] edge_left;
	delete[] edge_right;

}


KaboHaze::KaboHaze(Ship *omother, Ship *ohost, double obasepower, double odecaytime)
:
SpaceObject(omother, Vector2(0.0, 0.0), 0.0, omother->data->spriteExtra)
{
	mother = omother;
	host = ohost;
	basepower = obasepower;

	power = basepower;

	decay_time = odecaytime;	// in milliseconds

	newcrew = host->getCrew();
	oldcrew = newcrew;


	// I'll make 64 rotated versions in total, if needed, which I'll store
	// in memory:

	for ( int i = 0; i < 64; ++i )
	{
		shield_bmp[i] = 0;
	}
	sprite_index = 0;	// no rotation.


	// this item cannot collide

	collide_flag_anyone = 0;


	// this is probably important ... otherwise a thing like a flipping wedge indicator
	// can avoid a shield from being drawn ?!?!

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS + 0.1);
	// The +0.1 places this presence at a higher level, so that this routine is
	// always done after the ship was drawn! Thnx Orz, for telling.


	// Graphics init stuff for the shield !

	// copy the ship sprite (yes, a COPY because we need to do some operations on it !)

	SpaceSprite *ship_spr;
	ship_spr = host->get_sprite();
	if (!ship_spr)
	{
		state = 0;
		return;
	}

	//BITMAP *ship_bmp;
	int wship = ship_spr->width();
	int hship = ship_spr->height();

	BITMAP *ship_bmp = create_bitmap(wship, hship);
	clear_to_color(ship_bmp, 0);	// important otherwise it contains artefacts

	int index = 0;
	ship_spr->draw(Vector2(0, 0), Vector2(wship, hship), index, ship_bmp);
	// this does a (masked?) blit

	// create a blurred image from this:
	int R = 3;
	blit_blur(ship_bmp, R);		// a complex and costly funtion ! Inefficiently programmed as well of course (by me).

	// now, create a masked shield - only the area that covers the
	// blurred image of the ship:

	shield_bmp[sprite_index] = create_bitmap(wship, hship);
	clear_to_color(shield_bmp[sprite_index], 0);	// important otherwise it contains artefacts


	// scale/draw a shield:

	// the raw shield image
	BITMAP *raw_bmp = this->sprite->get_bitmap_readonly(0);

	int wraw = raw_bmp->w;
	int hraw = raw_bmp->h;

	stretch_blit(raw_bmp, shield_bmp[sprite_index], 0, 0, wraw, hraw, 0, 0, wship, hship );
	

	// mask out the areas outside the ship, so that the shield only covers
	// the ship.
	blit_singlecolor(ship_bmp, shield_bmp[sprite_index], makecol(0,0,0));


	destroy_bitmap(ship_bmp);


	// ok ! this is what we need - only things left are
	// resize
	// rotate
	// trans-draw.
	// which we've to do repeatedly.

	// we may also cache the rotated images !



	// flashes can occur within the edges of the shield ... check where the
	// edges are !! (assuming here, it's a closed shape).
	
	edge_left = new int [hship];
	edge_right = new int [hship];

	for ( int j = 0; j < hship; ++j )
	{
		edge_left[j] = -1;
		edge_right[j] = -1;
		for ( int i = 0; i < wship; ++i )
		{
			int color = getpixel(shield_bmp[0], i, j);

			if ( color != 0 )
			{
				if ( edge_left[j] == -1 )
					edge_left[j] = i;

				edge_right[j] = i;
			}

		}
	}

}


void KaboHaze::animate(Frame *space)
{
	// IMPORTANT: physics must not be changed, that is, NON-LOCAL variable must not
	// be changed in this subroutine !
	// Reason: the TimeWarp engine can decide to skip animations in case of low
	// frame-rate, thus, leading to a desynch between computers!
	
/*

  Aaaaah, lots of work to do for a simple job, namely, create a transparent overlay
  of the shield onto the host:

  - Init (see constructor):
  Take the host-ship picture.
  Create a (blurred) mask image of it.
  Draw the shield and
  Overlay the mask.

  - Animate:
  Scale and draw the masked shield,
  Rotate and
  Draw transparent onto the screen.

*/

	if (!host || !host->exists())
		return;

	if ( state == 0 )
		return;

	// the host can die in-between calculate and animate, therefore I use this; it's
	// not allowed to change state of this presence though; that's done only in
	// calculate().

	// Create a rotated copy of the shield sprite ... but only, if such a thing
	// does not exist, yet ! The purpose of this is, to spread the amount of
	// calculations over different frames, and to limit them to when they're
	// needed.

	int wshield = shield_bmp[0]->w;
	int hshield = shield_bmp[0]->h;

	if ( !shield_bmp[sprite_index] )
	{
		shield_bmp[sprite_index] = create_bitmap(wshield, hshield);
		clear_to_color(shield_bmp[sprite_index], 0);	// important otherwise it contains artefacts
		rotate_sprite(shield_bmp[sprite_index], shield_bmp[0], 0, 0, (1<<24)*(host->angle + 0.5*PI)/PI2 );
		// result is in sprite_bmp[sprite_index]   ( nice conventions, huh !)
	}
	

	//sprite_index = 0;	// also needed - for collision detection??
	// note, I've turned the collision of, since collide_flag_anyone = 0, but
	// otherwise, the collision detector would use this sprite_index to access
	// a ship sprite that doesnt exist !!


	// next, animate ...

	// first, reserve space for the target image, but ... how big should it be?
	// well, as big as the ship_bmp, but then, zoomed in space:

	int wfinal = int(wshield * space_zoom);
	int hfinal = int(hshield * space_zoom);

	BITMAP *final_bmp;
	final_bmp = create_bitmap(wfinal, hfinal);


	// scale/draw a shield:

	stretch_blit(shield_bmp[sprite_index], final_bmp, 0, 0, wshield, hshield, 0, 0, wfinal, hfinal );
	// result is in final_bmp


	// I need to calculate screen coordinates (using the original bmp size).
//	double xhost = host->normal_pos().x;
//	double yhost = host->normal_pos().y;
	Vector2 Vcorner;
	Vcorner = corner(host->normal_pos(), Vector2(wshield, hshield) );

	int xplot = Vcorner.x;	//wcorner(xhost, wshield);
	int yplot = Vcorner.y;	//hcorner(yhost, hshield);
	// these routines are the standard way to calculate screen coordinates !
	

	double power_scaled = power / 10.0;	// local
	if ( power_scaled > 1.0 )
		power_scaled = 1.0;		// max brightness.

	// change the brightness of the shield:
	int brightness = int(255 * power_scaled);

	set_add_blender(0, 0, 0, brightness);
	
	draw_trans_sprite(space->surface, final_bmp, xplot, yplot);
	space->add_box(xplot, yplot, wshield, hshield);

	// also, draw a (few) flashes, at twice the brightness:

	brightness = 255;

	for ( int i = 0; i < int(power); ++i )
	{
		int dx, dy;
		
		dx = wshield;
		dy = hshield;

		int icheck = 0;
		for (;;)
		{
			++icheck;
			if (icheck > 100)
				break;	// too bad !!

			dy = random() % hshield;
			if (edge_left[dy] == -1)
				continue;
			
			if ( !(random() % 2) )
				dx = edge_left[dy];
			else
				dx = edge_right[dy];
		}

		dx -= wshield / 2;
		dy -= hshield / 2;

		double a = host->angle + 0.5*PI;
		int dx2 = wshield/2 + dx * cos(a) - dy * sin(a);	// rotated around the center
		int dy2 = hshield/2 + dy * cos(a) + dx * sin(a);

		dx = int( dx2 * space_zoom);
		dy = int( dy2 * space_zoom);

		int x = xplot + dx;
		int y = yplot + dy;

		putpixel(space->surface, x, y, makecol(brightness,brightness,0) );

		space->add_pixel(x, y, 0, 0);
		
	}
	
	
	// release the temporary bitmap:
	destroy_bitmap(final_bmp);
}




REGISTER_SHIP ( KaboWeakener )

