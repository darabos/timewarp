#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../melee/mlog.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../melee/mitems.h"
#include "../melee/manim.h"

#include "../scp.h"
#include "../util/history.h"


#include "gamehyper.h"
#include "gamesolarview.h"
#include "gamemelee.h"

static const int ID_FLEET_HYPER = 0x09837491;

double r_visual = 40.0;	// size of the radar map; outside this, it's no use to draw/ calculate fleets

inline double sqr(double x)
{
	return x*x;
}


class HyperFleet : public SpaceObject
{
public:
	SpaceObject	*follow;
	double		speed;

	HyperFleet(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	virtual void calculate();
	virtual void animate(Frame *f);

	RaceInfo *ri;
};


HyperFleet::HyperFleet(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(creator, opos, oangle, osprite)
{
	follow = 0;
	speed = 0;

	id = ID_FLEET_HYPER;

	layer = LAYER_SHIPS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
}


void HyperFleet::calculate()
{
	if (!follow)
	{
		state = 0;
		return;
	}

	angle = trajectory_angle(follow);
	vel = speed * unit_vector(angle);

	// when the target is out of range (i.e., when the ship's not visible on
	// radar anymore) then this hyperfleet is deleted.
	double R;
	R = distance(follow);

	double s = mapeverything.sub[0]->scalepos;
	if (R > s * r_visual * 1.1)
		state = 0;
}

void HyperFleet::animate(Frame *f)
{
	Vector2 s = sprite->size(sprite_index);
	Vector2 p = corner(pos, s );
	sprite->draw_character(p.x, p.y, sprite_index, makecol(0,0,0), f);

}


void GameHyperspace::calc_enemies()
{
	// check if you're in range of one of the colonies:
	RaceInfo *a;
	a = racelist.first;

	while (a)
	{
		RaceSettlement *b;
		b = a->firstcol;

		while (b)
		{
			Vector2 P;
			P = mapeverything.sub[0]->sub[b->istar]->position;


			double r;
			r = (P - player->pos / scalepos).length();
			
			if (r < b->patrol.range)
			{
				double density, chance, enemyspeed;

				enemyspeed = 0.1;

				density = 0.01;
				density = 0.1;

				chance = (player->vel.length() + enemyspeed) * frame_time / scalepos;
				chance *= density;
				

				if (random(1.0) < chance)
				{
					Vector2 offset;
					offset = r_visual * unit_vector(random(PI2));
					offset *= starmap->scalepos;

					// create a "enemy" object ... but how ?
					HyperFleet *fl;
					fl = new HyperFleet(0, player->pos+offset, 0, a->fleetsprite);
					add(fl);
					fl->speed = enemyspeed;
					fl->follow = player;
					fl->ri = a;
					
				}
			}
			



			b = b->next;
		}

		a = a->next;
	}
}


#define HIST_POWER 4.0

class AnimationHyper : public Animation
{
	WindowInfo *wininfo;
	Vector2 spawnpos, visiblerange;
	double	level, waittime, wait;
	int		Nframes, frame1;
public:
	AnimationHyper(WindowInfo *owininfo, Vector2 opos, SpaceSprite *osprite, 
			int first_frame, int num_frames, double period, double wait, double depth, double olevel,
			Vector2 ovisiblerange);

	virtual void calculate();
	virtual void animate(Frame *space);
};



double wt = 0.0;

AnimationHyper::AnimationHyper(WindowInfo *owininfo, Vector2 opos, SpaceSprite *osprite, 
			int first_frame, int num_frames, double period, double owait, double depth, double olevel,
			Vector2 ovisiblerange)
:
Animation(0, opos, osprite, first_frame, num_frames,
		  1000 * (period / (num_frames - first_frame)), depth, 1.0)
{
	level = olevel;
	wininfo = owininfo;
	// this position is on the physical map
	spawnpos = opos;
	visiblerange = ovisiblerange;

	wait = owait;

	waittime = random(wait + period) - period;

	Nframes = num_frames - first_frame;
	frame1 = first_frame;

	if (waittime < 0)
	{
		// you're within a period.
		frame_step = -waittime;
		//sprite_index = frame1 - Nframes * waittime / period;
		//if (sprite_index >= frame1 + Nframes)
			//sprite_index = frame1 + Nframes - 1;
	}


	// this position is on the screen
	pos = wininfo->mapcenter + level * (spawnpos - wininfo->mapcenter);
}


void AnimationHyper::animate(Frame *space)
{
	
//	scale = 1;
//	sprite->animate(pos, sprite_index, space, scale);
	Vector2 p, s;
	s = sprite->size(sprite_index);
	p = corner(pos, s);

	if (p.x > 0 && p.x+s.x <= space->surface->w &&
		p.y > 0 && p.y+s.y <= space->surface->h )
	{
		//draw(, s * space_zoom, index, space);
		masked_blit(sprite->get_bitmap(sprite_index), space->surface,
			0, 0,  p.x, p.y,  s.x, s.y);
	}
}


void AnimationHyper::calculate()
{

	if (waittime > 0)
	{
		waittime -= frame_time * 1E-3;

	} else {

		frame_step -= frame_time;
		
		while (frame_step < 0)
		{
			frame_step += frame_size;
			sprite_index += 1;
			if (sprite_index == sprite->frames()) sprite_index = 0;
			frame_count -= 1;
			
			if (!frame_count)
			{
				waittime = wait;
				sprite_index = frame1;
				frame_count = Nframes;
			}
		}
	}

	//sprite_index = sprite->frames() - 1;

	// it's a fixed position, but at a different depth level (closer to the eye).
	pos = wininfo->mapcenter + level * (spawnpos - wininfo->mapcenter);

	Vector2 dpos;
	dpos = pos - wininfo->mapcenter;

	// if it's off the viewscreen, it can be deleted
	if (fabs(dpos.x) > visiblerange.x ||
			fabs(dpos.y) > visiblerange.y)
		state = 0;
}




typedef BITMAP *bmpptr;

class StarHyper : public SpaceObject
{
public:
	StarHyper(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	~StarHyper();

	virtual void animate(Frame *f);
	virtual void calculate();

	bmpptr *bmpcache;
};



StarHyper::StarHyper(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(creator, opos, oangle, osprite)
{
	layer = LAYER_SHOTS;

	
	bmpcache = new bmpptr [sprite->frames()];

	int i;
	for ( i = 0; i < sprite->frames(); ++i )
	{
		bmpcache[i] = 0;
	}
}


void StarHyper::animate(Frame *f)
{
	//SpaceObject::animate(f);
	// well, since this is a special case, and the map is "unscaled", and really, pixel-physics
	// is not so important here, you can simply blit the pre-scaled sprite .

	// or
	// do this on the fly ??
	// (big sprite, so it's slow to render ... doh ...)

	
	Vector2 s;
	s = sprite->size(sprite_index) * ::space_zoom;

	if (bmpcache[sprite_index])
	{
		if (bmpcache[sprite_index]->w != int(s.x) || bmpcache[sprite_index]->h != int(s.y))
		{
			destroy_bitmap(bmpcache[sprite_index]);
			bmpcache[sprite_index] = 0;
		}
	}

	BITMAP *bmp;
	bmp = sprite->get_bitmap(sprite_index);

	if (!bmpcache[sprite_index])
	{
		int bpp;
		bpp = bitmap_color_depth(bmp);
		//bmpcache[sprite_index] = create_bitmap_ex(bpp, s.x, s.y);
		bmpcache[sprite_index] = create_video_bitmap(s.x, s.y);		// for faster drawing.

		stretch_blit(bmp, bmpcache[sprite_index],
			0, 0,  bmp->w, bmp->h,
			0, 0,  bmpcache[sprite_index]->w, bmpcache[sprite_index]->h);
	}


	Vector2 p;
	p = corner(pos, s);

	masked_blit(bmpcache[sprite_index], f->surface,
		0, 0, p.x, p.y,
		bmpcache[sprite_index]->w, bmpcache[sprite_index]->h);
}


StarHyper::~StarHyper()
{
	int i;
	for ( i = 0; i < sprite->frames(); ++i )
	{
		if (bmpcache[i])
			destroy_bitmap(bmpcache[i]);
	}

	delete bmpcache;
}


void StarHyper::calculate()
{
	SpaceObject::calculate();
}



GameHyperspace::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	if (playinf->istar >= 0)
	{
		MapSpacebody *starmap = mapeverything.sub[0];
		pos = starmap->sub[playinf->istar]->position;
	}

	layer = LAYER_SHIPS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
}


void inline changebit(int *byte, int bitnum)
{
	int bitval;
	bitval = 1 << bitnum;

	if ( (*byte & bitval) != 0 )
		*byte &= (~bitval);
	else
		*byte |= bitval;
}


int semirandom(int n)
{
	int count, m, N;
	N = 16;
	
	for ( count = 0; count < 2*N; ++count )
	{
		
		for ( m = 0; m < N; ++m )
		{
			int m1, m2;
			
			m1 = m-5;
			while (m1 < 0) m1 += N;
			m2 = m+5;
			while (m2 > N-1) m2 -= N;
			
			changebit(&n, m);
			if (n & (1<<m))
			{
				//if (~n & (1<<m1))
				changebit(&n, m1);
				//if (~n & (1<<m2))
				changebit(&n, m2);
				n = ~n;
			}
		}
	}
	
	return n;
}



void GameHyperspace::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/hyperspace", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");
	makevideobmp(maparea->backgr);	// for (much) faster drawing? Cause we're dealing with a large area here!


	bradar = new Area(T, "radar_");
	makevideobmp(bradar->backgr);
}




void GameHyperspace::init()
{
	int i;

	
	GameBare::init();
//	mapwrap = false;



	escapetime = 0;
	
	// initialize ... things ?
	// it should be a big map, compared to the bitmap (star) sizes !!
	// (those bitmaps are used for physics).
	// in a way, this size is in pixel size ... (doh).

	// the visible (ref/scaled)screen is 800 pixels wide (pixel coord = screen-plot-coord) ...
	// but, you should see only about 8.0 measured in the starmap reference frame (star-coord)...
	// which is a factor 100 different.
	// the real starmap is 1,000 wide/high (I think?). So...
	double H = 1000;
	scalepos = 100;
	H *= scalepos;

	size = Vector2(H, H);
	// this is a very large map !! 100,000 x 100,000


	tempframe->set_background ( 82, 0, 0 );

	prepare();


//	wininfo.init( Vector2(800,800), 800.0, tempframe );
	// but ... do you see the edge of the map, or not ??
	// you can make the map a bit bigger to prevent that ...

	// create star objects
	starmap = mapeverything.sub[0];	// use the starmap of the 1st region
	if (mapeverything.Nsub == 0) {tw_error("something is wrong with the map - there are no stars");}


	// initialization of sprites
	int bpp;
	bpp = 32;//bitmap_color_depth(f->surface);
//	submap = create_bitmap_ex(bpp, 100, 100);
	
	// also, init certain sprites ?
	// well, all the different stars big/med/small /color...
	
	for ( i = 0; i < startypelist->N; ++i )
	{
		char tmp[512];
		strcpy(tmp, "gamex/hyperspace/star_");
		strcat(tmp, startypelist->type[i].type_string);
		strcat(tmp, "_01.bmp");

		// hmm, you should scale it as well ... (doh)
		//starbmp[i] = load_bitmap(tmp, 0);
		star_hyperspr[i] = create_sprite( tmp, SpaceSprite::MASKED );

		strcpy(tmp, "gamex/hyperspace/radar_");
		strcat(tmp, startypelist->type[i].type_string);
		strcat(tmp, "_01.bmp");

		star_radarspr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}

	radarenemyspr = create_sprite( "gamex/hyperspace/radar_enemy_01.bmp", SpaceSprite::MASKED );

	radarplayerspr = create_sprite( "gamex/hyperspace/radar_player_01.bmp", SpaceSprite::MASKED );


	for ( i = 0; i < starmap->Nsub; ++i )
	{
		SpaceObject *star;
//		SpaceSprite *spr;
		// hmm ... you create too many sprites this way !!
		// this should be changed.
//		char txt[512];
//		sprintf(txt, "gamex/hyperspace/star_%s_01.bmp", starmap->sub[i]->type);
//		spr = create_sprite( txt, SpaceSprite::MASKED );
		star = new StarHyper(0, starmap->sub[i]->position * scalepos, 0.0,
									star_hyperspr[starmap->sub[i]->type]);
		starmap->sub[i]->o = star;
		add(star);
	}

	SpaceSprite *mousespr;
	mousespr = create_sprite( "gamex/mouse/pointer_starmap.bmp", SpaceSprite::MASKED );
	ptr = new MousePtr (mousespr);
	add(ptr);


	// the player
	SpaceSprite *playerspr;
	playerspr = create_sprite( "gamex/hyperspace/player_01.bmp", SpaceSprite::MASKED, 64 );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);


	wininfo.center(playerinfo.pos);

	set_config_file("gamex/hyperspace/info.txt");

	int refresolution;
	double sprscale;

	int abpp = bitmap_color_depth(view->frame->window->surface);

	hyperexpl = new HyperspaceExplosions(&wininfo);


	refresolution = get_config_int(0, "Res", 800);
	Nlayers = get_config_int(0, "N", 4);

	sprscale = double(screen->w) / refresolution;

	for ( i = 0; i < Nlayers; ++i )
	{
		char id[64];
		sprintf(id, "depth%02i", i+1);

		double depthfactor, period, wait, density;
		char name[128];
		int N;

		depthfactor = get_config_float(id, "Depthfactor", 1.0);
		strncpy(name, get_config_string(id, "Name", "hi"), 128);
		N = get_config_int(id, "N", 1);
		period = get_config_float(id, "Period", 1.0);
		wait = get_config_float(id, "Wait", 1.0);
		density = get_config_float(id, "Density", 1.0);
		

		
		//	AnimationHyper::AnimationHyper(SpaceLocation *oref, Vector2 opos, SpaceSprite *osprite, 
		//			int first_frame, int num_frames, int frame_size, double depth, double olevel,
		//			Vector2 ovisiblerange)
		
		// you should also use "refresolution" to scale the stuff you load ?
		char sprname[128];
		strcpy(sprname, "gamex/hyperspace/");
		strcat(sprname, name);
		strcat(sprname, "_01.bmp");

		// this defaults to a 32 bit depth (doh) !!
		bool vidmem = true;	// try to store the sprites in video-memory, for speed
		// too bad; true or false makes no difference...
		spr[i] = create_sprite( sprname, SpaceSprite::MASKED | SpaceSprite::IRREGULAR, N, 32, sprscale, vidmem );
		//sprA = create_sprite( "gamex/hyperspace/star_blobA_01.bmp", SpaceSprite::MASKED | SpaceSprite::IRREGULAR, 6 );
		//sprB = create_sprite( "gamex/hyperspace/star_blobB_01.bmp", SpaceSprite::MASKED | SpaceSprite::IRREGULAR, 6 );
		//sprC = create_sprite( "gamex/hyperspace/star_blobC_01.bmp", SpaceSprite::MASKED | SpaceSprite::IRREGULAR, 1 );
		

		hyperexpl->addlevel(spr[i], depthfactor, period, wait, density);
		//hyperexpl->addlevel(sprB, 1.0);
		//hyperexpl->addlevel(sprA, 1.5);
		//hyperexpl->addlevel(sprC, 0.75);
		
	}


	hyperexpl->init();

	// performance check
	tic_history = new Histograph(128);
	render_history = new Histograph(128);


}



void GameHyperspace::quit()
{
	// copy values of the player position - since, exiting hyperspace implies,
	// exiting the game (should not be needed in other gametypes).

	if (!hardexit)	// game is not quitted completely
		playerinfo.sync2(player);

	delete tic_history; tic_history = NULL;
	delete render_history; render_history = NULL;

	int i;
	for ( i = 0; i < Nlayers; ++i )
		delete spr[i];
	//delete sprA;
	//delete sprB;
	//delete sprC;
	delete hyperexpl;


	// it's not terribly efficient to create/ delete all the sprites, each time
	// you change game mode ...
	for ( i = 0; i < startypelist->N; ++i )
	{
		delete star_hyperspr[i];
		delete star_radarspr[i];
	}

//	destroy_bitmap(submap);

	GameBare::quit();
}

void GameHyperspace::refocus()
{
	if (!hardexit)
	{
		playerinfo.sync(player);		// local copies from global player-info

		// check if the starmap hasn't changed (by editing)
		// NOTE: deletion isn't supported (yet).
		int i;
		for ( i = 0; i < starmap->Nsub; ++i )
		{
			SpaceObject *o;
			o = starmap->sub[i]->o;
			if (!o)
			{
				//add
				SpaceObject *star;
				star = new StarHyper(0, starmap->sub[i]->position * scalepos, 0.0,
					star_hyperspr[starmap->sub[i]->type]);
				starmap->sub[i]->o = star;
				add(star);
			} else {
				//reposition (by default) (pos could be changed)
				o->pos = starmap->sub[i]->position * scalepos;
				// reset sprite type as well (color could be changed) ?
				o->set_sprite(star_hyperspr[starmap->sub[i]->type]);
			}

		}
	}
}

bool GameHyperspace::handle_key(int k)
{
	return GameBare::handle_key(k);
}


int GameHyperspace::enterstar(SpaceObject *p)
{
	// check all the stars ... yeah, you need to find the star-number !!

	int i = -1;

	for ( i = 0; i < starmap->Nsub; ++i )
	{
		Vector2 D;
		D = p->pos - starmap->sub[i]->position * scalepos;
		if (D.length() < 50.0)
			return i;
	}

	if ( i == starmap->Nsub )
		i = -1;

	return i;
}


void GameHyperspace::calculate()
{
	if (next)
		return;

	double t = get_time2();
	double dt = frame_time * 1E-3;

	// just in case you jump to another game and back and this setting can be
	// changed then ...
	starmap->scalepos = scalepos;


	GameBare::calculate();

	if (key[KEY_UP])
		player->accelerate(0, player->angle, 0.1*dt, 1.0);

	if (key[KEY_LEFT])
		player->angle -= 0.5*PI2 * dt;

	if (key[KEY_RIGHT])
		player->angle += 0.5*PI2 * dt;


	wininfo.center(player->pos);

	hyperexpl->expand();

	message.print(1500, 14, "speed = %f", player->vel.length() * 1E3);


	t = get_time2() - t;// - paused_time;
	tic_history->add_element(pow(t, HIST_POWER));

	calc_enemies();
}



void GameHyperspace::checknewgame()
{
	playerinfo.istar = -1;
	
	int istar;
	// check if the player can enter some star
	istar = enterstar(player);

	// if the player is close enough ...
	if (istar >= 0)
	{
		playerinfo.istar = istar;
		playerinfo.angle = player->angle;
		
		// if you can add a game, then let "someone" know that, by allocating
		// a game, and by setting a request pointer to that allocated game :)
		gamerequest = new GameSolarview();
	}

	// if you're close to a hyperfleet
	if (player->collisionwith)
	{
		SpaceObject *o;
		o = player->collisionwith;
		if ( o->id == ID_FLEET_HYPER)
		{
			// then enter melee ...
			player->vel = 0;

			// what's the race ?
			XFleet *f;
			f = new XFleet();

			f->add(((HyperFleet*)o)->ri->shipid, 3);

			// spawn a subgame
			if (!gamerequest && !next)
			{
				GameMelee *g;
				g = new GameMelee();
				g->set_xfleet(f);

				gamerequest = g;
			}

			o->state = 0;
		}
	}
}




void GameHyperspace::animate(Frame *frame)
{
	if (next)
		return;


	plot_submap(bradar->backgr);

	GameBare::animate(frame);

}



void GameHyperspace::plot_submap(BITMAP *submap)
{

	int i;

//	BITMAP *t;
//	t = f->surface;
//	rectfill(t, 0, t->h*0.8, t->w, t->h, 0);

	// drawing - check all the stars ...

	clear_to_color(submap, makecol(0,100,0));

	double L, W;
	L = 40.0;		// total view width = 80, which is 4 blocks.
	W = 20.0;		// grid line separation


	// draw grid lines

	Vector2 P;
	int ix, iy, N;

	P = player->pos / scalepos;
	ix = int(P.x / W);
	iy = int(P.y / W);
	N = int(L / W) + 1;

	for ( i = ix-N; i <= ix+N; ++i)
	{
		int xp;

		xp = i*W - P.x;

		xp *= submap->w / (2*L);

		xp += 0.5 * submap->w;

		line ( submap, xp, 0, xp, submap->h-1, makecol(0,128,0));
	}

	for ( i = iy-N; i <= iy+N; ++i)
	{
		int yp;

		yp = i*W - P.y;

		yp *= submap->w / (2*L);

		yp += 0.5 * submap->w;

		line ( submap, 0, yp, submap->w-1, yp, makecol(0,128,0));
	}


	// should be done centrally

	BITMAP *bmp;

	for ( i = 0; i < starmap->Nsub; ++i )
	{
		Vector2 D;

		// star position relative to player position (in star coord system).
		D = starmap->sub[i]->position - (player->pos / scalepos);

		if ( fabs(D.x) < L  &&  fabs(D.y) < L )
		{
			int k;
			k = starmap->sub[i]->type;
			D *= submap->w / double(2*L);		// note, L is the half-width...

			Vector2 s;
			s = star_radarspr[k]->size(0);
			D -= 0.5 * Vector2(s.x, s.y);

			D += 0.5 * Vector2(submap->w, submap->h);

			bmp = star_radarspr[k]->get_bitmap(0);
			masked_blit(bmp, submap, 0, 0, D.x, D.y, bmp->w, bmp->h);
		}
	}

	// draw all (enemy) hyperspace fleets

	for ( i = 0; i < num_items; ++i )
	{
		SpaceLocation *o = item[i];

		if (o->id == ID_FLEET_HYPER)
		{
			// fleet position relative to player position (in star coord system).
			Vector2 D;
			D = (o->pos - player->pos) / scalepos;
			
			if ( fabs(D.x) < L  &&  fabs(D.y) < L )
			{
				D *= submap->w / double(2*L);		// note, L is the half-width...
				
				Vector2 s;
				s = radarenemyspr->size(0);
				D -= 0.5 * Vector2(s.x, s.y);
				
				D += 0.5 * Vector2(submap->w, submap->h);
				
				bmp = radarenemyspr->get_bitmap(0);
				masked_blit(bmp, submap, 0, 0, D.x, D.y, bmp->w, bmp->h);
			}
		}
	}



	bmp = radarplayerspr->get_bitmap(0);

	P = 0.5 * bitmap_size(submap) - 0.5 * bitmap_size(bmp);
	masked_blit(bmp, submap, 0, 0, P.x, P.y, bmp->w, bmp->h);

	// plot the radar submap onto the (temp) screen surface.
//	P = bitmap_size(t) - bitmap_size(submap);
//	blit(submap, surface, 0, 0, P.x, P.y, submap->w, submap->h);
}




HyperspaceExplosions::HyperspaceExplosions(WindowInfo *owininfo)
{
	Nlevels = 0;
	wininfo = owininfo;
}


void HyperspaceExplosions::addlevel(SpaceSprite *ospr, double olevel,
							double operiod, double owait, double odensity)
{
	spr[Nlevels] = ospr;
	level[Nlevels] = olevel;

	period[Nlevels] = operiod;
	wait[Nlevels] = owait;
	density[Nlevels] = odensity;

	++Nlevels;
}



void HyperspaceExplosions::init_cell(int ix, int iy, int ilevel)
{
	Vector2 P;
	
	// the position on the unscaled map.
	P = Vector2(ix+0.5,iy+0.5) * W[ilevel];
	
	if ( P.x < 0 || P.y < 0 || P.x > map_size.x || P.y > map_size.y )
		return;
	
	int n;
	n = ix + iy*mapNy[ilevel] + ilevel * mapNx[ilevel]*mapNy[ilevel];
	
	n = semirandom(n);
	// advice from Orz, to use the tw-internal random number generatpor (rng):
//	RNG_FS tmp;
//	tmp.set_state64(n);
//	tmp.raw32();
	// this rng isn't good enough ...
	
	n = n & 1023;
	
	if ( n < density[ilevel] )
	{
		physics->add ( new AnimationHyper(wininfo, P, 
			spr[ilevel], 0, spr[ilevel]->frames(), period[ilevel], wait[ilevel], DEPTH_HOTSPOTS,
			level[ilevel], Vector2(winNx[ilevel]+0.5,winNy[ilevel]+0.5)*W[ilevel]*level[ilevel]  ) );
	}
				
}



void HyperspaceExplosions::init()
{
	int ix, iy;

	int i;

	for ( i = 0; i < Nlevels; ++i )
	{
		W[i] = 10;	// physically, they're all equally big. Some are closer to the eye, that's all...
		
		// the number of cells on the map
		mapNx[i] = map_size.x / W[i];
		mapNy[i] = map_size.y / W[i];
		
		// the number of those cells that you can actually see.
		double sc;		// should be 0.5: half the screen
		sc = 0.35;	// for testing, to see if the correct square is made
		sc = 0.5;
		winNx[i] = sc * wininfo->maparea.x / (W[i] * level[i]);
		winNy[i] = sc * wininfo->maparea.y / (W[i] * level[i]);
			
		ixpos[i] = int(wininfo->mapcenter.x / W[i]);
		iypos[i] = int(wininfo->mapcenter.y / W[i]);
		
		for ( iy = iypos[i]-winNy[i]; iy <= iypos[i]+winNy[i]; ++iy )
		{
			for ( ix = ixpos[i]-winNx[i]; ix <= ixpos[i]+winNx[i]; ++ix )
			{
				init_cell(ix, iy, i);
			}
		}
		
	}

}


// if you move over the screen, create new explosions in the part that's seen for
// the first time.
void HyperspaceExplosions::expand()
{

	int ix, iy, ixold, iyold;

	int i;

	for ( i = 0; i < Nlevels; ++i )
	{
		
		ixold = ixpos[i];
		iyold = iypos[i];
	
		ixpos[i] = int(wininfo->mapcenter.x / W[i]);
		iypos[i] = int(wininfo->mapcenter.y / W[i]);

		if (iypos[i] != iyold)
		{
			int iy1, iy2;

			if ( iypos[i] > iyold )
			{
				iy1 = iyold + winNy[i]+1;	// +1 cause the old one was already filled till winNy cells
				iy2 = iypos[i] + winNy[i];
			} else {
				iy1 = iypos[i] - winNy[i];
				iy2 = iyold - winNy[i]-1;
			}
			
			// from old to new, fill rows,.
			for ( iy = iy1; iy <= iy2; ++iy )
				
				// create a new row:
				for ( ix = ixpos[i]-winNx[i]; ix <= ixpos[i]+winNx[i]; ++ix )
					init_cell(ix, iy, i);
				
		}

		// same for the horizontal stuff :)
		if (ixpos[i] != ixold)
		{
			int ix1, ix2;

			if ( ixpos[i] > ixold )
			{
				ix1 = ixold + winNx[i]+1;	// +1 cause the old one was already filled till winNy cells
				ix2 = ixpos[i] + winNx[i];
			} else {
				ix1 = ixpos[i] - winNx[i];
				ix2 = ixold - winNx[i]-1;
			}
			
			// from old to new, fill rows,.
			for ( ix = ix1; ix <= ix2; ++ix )
				
				// create a new column:
				for ( iy = iypos[i]-winNy[i]; iy <= iypos[i]+winNy[i]; ++iy )
					init_cell(ix, iy, i);
				
		}
	}


}



// non-linear physics for all objects in this gametype.

void GameHyperspace::calc_kinetic(SpaceLocation *s)
{
	s->pos += s->vel * frame_time;
	// slow down the object
	double dt = frame_time * 1E-3;
	s->vel *= (1.0 - 0.5 * dt);


}



// bounce back off the edges:

void GameHyperspace::handle_edge(SpaceLocation *s)
{
	Vector2 &P = s->pos;
	Vector2 &vel = s->vel;

	Vector2 Dist;
	
	Dist = vel * frame_time;

	while (P.x < 0 || P.y < 0 || P.x > map_size.x || P.y > map_size.y)
	{
		if (P.x < 0)
		{
			if (fabs(P.x) >= fabs(Dist.x))
			{
				P.x = 0;
				continue;
			}

			Dist *= (P.x/Dist.x);	// this is the part that's been travelled on the wrong side

			P.x = 0;
			P.y -= Dist.y;

			Dist.x = -Dist.x;	// reflection
			P += Dist;	// travelling the remaining reflected distance

			vel.x = -vel.x; // corresponding reflection of velocity.
			continue;
		}

		if ( P.y < 0 )
		{
			//P.y = 0;
			if (fabs(P.y) >= fabs(Dist.y))
			{
				P.y = 0;
				continue;
			}

			Dist *= (P.y/Dist.y);	// this is the part that's been travelled on the wrong side

			P.y = 0;
			P.x -= Dist.x;

			Dist.y = -Dist.y;	// reflection
			P += Dist;	// travelling the remaining reflected distance

			vel.y = -vel.y; // corresponding reflection of velocity.
			continue;
		}

		if (P.x > map_size.x)
		{
			//P.x = map_size.x;
			if (fabs(P.x-map_size.x) >= fabs(Dist.x))
			{
				P.x = map_size.x;
				continue;
			}

			Dist *= ((P.x-map_size.x)/Dist.x);	// this is the part that's been travelled on the wrong side

			P.x = map_size.x;
			P.y -= Dist.y;

			Dist.x = -Dist.x;	// reflection
			P += Dist;	// travelling the remaining reflected distance

			vel.x = -vel.x; // corresponding reflection of velocity.
			continue;
		}

		if (P.y > map_size.y)
		{
			//P.y = map_size.y;
			if (fabs(P.y-map_size.y) >= fabs(Dist.y))
			{
				P.y = map_size.y;
				continue;
			}

			Dist *= ((P.y-map_size.y)/Dist.y);	// this is the part that's been travelled on the wrong side

			P.y = map_size.y;
			P.x -= Dist.x;

			Dist.y = -Dist.y;	// reflection
			P += Dist;	// travelling the remaining reflected distance

			vel.y = -vel.y; // corresponding reflection of velocity.
			continue;
		}
	}

}