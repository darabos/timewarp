
#include <allegro.h>
#include <string.h>
#include <stdio.h>
#include "../melee.h"
REGISTER_FILE
#include "../id.h"
#include "../scp.h"
#include "../frame.h"

#include "mgame.h"
#include "mview.h"
#include "mcbodies.h"
#include "manim.h"
#include "mship.h"
#include "mgame.h"

Asteroid::Asteroid() 
:	SpaceObject(NULL, random(map_size), random(PI2), game->asteroidSprite), explosion(game->asteroidExplosionSprite)
{STACKTRACE
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_CBODIES;
	set_depth(DEPTH_ASTEROIDS);
	id    |= ID_ASTEROID;
	step  = 0;
	speed = (random(3) + 1);
	mass  = 10 + random(10);

	vel = unit_vector(angle) * (speed * 0.05);
}

void Asteroid::calculate()
{STACKTRACE
	step-= frame_time;
	while(step <= 0) {
		step += speed * time_ratio;
		sprite_index++;
		if(sprite_index == ASTEROID_FRAMES)
			sprite_index = 0;
	}

	SpaceObject::calculate();
}

int Asteroid::handle_damage(SpaceLocation *source, double normal, double direct) {STACKTRACE
	if (!exists()) return 0;
	if (!normal && !direct) return 0;

	state = 0;
	return 1;
	}

void Asteroid::death() {STACKTRACE

	Animation *a = new Animation(this, pos,
			explosion, 0, explosion->frames(), time_ratio, get_depth());
	a->match_velocity(this);
	game->add(a);
	
	game->add(new Asteroid());
	return;
	}


Planet::Planet(Vector2 loc, SpaceSprite *sprite, int index) 
	:
	SpaceObject(NULL, loc, 0.0, sprite) 
	{STACKTRACE
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_CBODIES;
	set_depth(DEPTH_PLANETS);
	id         |= ID_PLANET;
	mass        = 9999999.0;
//use remote .ini file
	game->log_file ("server.ini");
	sprite_index = index;
	gravity_mindist = scale_range(get_config_float("Planet", "GravityMinDist", 0));
	gravity_range = scale_range(get_config_float("Planet", "GravityRange", 0));
	gravity_power = get_config_float("Planet", "GravityPower", 0);
	gravity_force = 
		scale_acceleration(get_config_float("Planet", "GravityForce", 0), 0);
	gravity_whip = get_config_float("Planet", "GravityWhip", 0);
	gravity_whip2 = get_config_float("Planet", "GravityWhip2", 0);

	}

void Planet::inflict_damage(SpaceObject *other) {STACKTRACE
	int i = 1;
	if (other->isShip()) {
		i = (int) ceil(((Ship*)other)->crew / 3.0);
		}
	if (other->mass == 0) other->state = 0;
	damage(other, 0, i);
	i /= 2;
	if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
	if (!other->isShot()) 
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	return;
	}

void Planet::calculate() {STACKTRACE
	SpaceObject::calculate();
	SpaceObject *o;
	Query a;
	a.begin(this, OBJECT_LAYERS, gravity_range);
	for (;a.currento;a.next()) {
		o = a.currento;
		if (o->mass > 0) {
			double r = distance(o);
			if (r < gravity_mindist) r = gravity_mindist;
			double sr = 1;
//gravity_power rounded up here
			if (gravity_power < 0) {
				r /= 40 * 5;
				for (int i = 0; i < -gravity_power; i += 1) sr *= r;
				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force / sr, MAX_SPEED);
			}
			else {
				r = 1 - r/gravity_range;
				for (int i = 0; i < gravity_power; i += 1) sr *= r;
				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force * sr, MAX_SPEED);
			}
		}
	}
	return;
}



void _draw_starfield_raw (
	//surface to draw starfield on
	Frame *frame,
	//star sprite
	SpaceSprite *sprite, 
	//index into star sprite
	int index, int num_indeces, 
	//number of stars
	int n, 
	//center of screen for drawing purposes
	double cx, double cy, 
	//scrolly amount
	double x, double y, 
	//size of starfield (usually the same as wrap point, sometimes smaller)
	double w, double h, 
	//wrap point
	double mx, double my, 
	//size of stars
	double zoom,
	//anti-aliasing mode to use
	int aa_mode
) 
{STACKTRACE
	int i;
	double wx, wy;
	wx = sprite->width() * zoom;
	wy = sprite->height() * zoom;
	if (!wx || !wy) return;
	x = normalize ( x , mx);
	y = normalize ( y , my);
	if ( ((x > frame->surface->w-cx) && (x+w < mx)) ||
		((y > frame->surface->h-cy) && (y+h < my)) ) 
		return;
	cx -= wx/2 ;
	cy -= wy/2 ;
	int old_aa = get_tw_aa_mode();
	set_tw_aa_mode((old_aa &~(aa_mode>>12)) | (aa_mode&0x0fff));
	for (i = 0; i < n; i += 1) {
		double tx, ty;
		tx = int((fs_random() & 65535)-32768) * (w / 65536.0) + x;
		ty = int((fs_random() & 65535)-32768) * (h / 65536.0) + y;
		if (tx > mx/2) tx -= mx;
		if (ty > my/2) ty -= my;
		sprite->draw(Vector2(cx+tx, cy+ty), Vector2(wx, wy), index + (i % num_indeces), frame);
	}
	set_tw_aa_mode(old_aa);
	return;
}

void _draw_starfield_cached (
	//surface to draw starfield on
	Frame *frame,
	//star sprite
	SpaceSprite *sprite, 
	//index into star sprite
	int index,
	//number of stars
	int n, 
	//center of screen for drawing purposes
	double cx, double cy, 
	//scrolly amount
	double x, double y, 
	//size of starfield (usually the same as wrap point, sometimes smaller)
	double w, double h, 
	//wrap point
	double mx, double my, 
	//size of stars
	double zoom,
	//anti-aliasing mode to use
	int aa_mode
) 
{STACKTRACE
	int i;
	double wx, wy;
	int iwx, iwy;
	wx = sprite->width() * zoom;
	wy = sprite->height() * zoom;
	if (aa_mode) {
		iwx = iround_up(wx);
		iwy = iround_up(wy);
	}
	else {
		iwx = iround_down(0.5 + wx);
		iwy = iround_down(0.5 + wy);
	}
	if (!iwx || !iwy) return;
	x = normalize ( x , mx);
	y = normalize ( y , my);
	if ( ((x > frame->surface->w-cx) && (x+w < mx)) ||
		((y > frame->surface->h-cy) && (y+h < my)) ) 
		return;

	int old_aa = get_tw_aa_mode();
	set_tw_aa_mode((old_aa &~(aa_mode>>12)) | (aa_mode&0x0fff));
	
	BITMAP *bmp = NULL;
	bmp = create_bitmap(iwx, iwy);
	if (!bmp) return;
	//clear_to_color(bmp, background_color);
	clear_to_color(bmp, 0);
	if (aa_mode) {
		sprite->draw( Vector2((iwx - wx) / 2, (iwy-wy)/2), Vector2(wx, wy), index, bmp);
	}
	else {
		sprite->draw(Vector2(0, 0), Vector2(iwx, iwy), index, bmp);
	}

	set_tw_aa_mode(0);
	cx = (cx - wx/2);
	cy = (cy - wy/2);
	for (i = 0; i < n; i += 1) {
		double tx, ty;
		tx = (fs_random() & 65535) * (w / 65536.0) + x;
		ty = (fs_random() & 65535) * (h / 65536.0) + y;
		if (tx > mx) tx -= mx;
		if (ty > my) ty -= my;
		int ix = iround(cx+tx);
		int iy = iround(cy+ty);
		blit(bmp, frame->surface,0,0,
			ix, iy,
			iwx, iwy);
		frame->add_box(ix, iy, iwx, iwy);
	}

	set_tw_aa_mode(old_aa);
	return;
}




SpaceSprite *Stars2::starpics = NULL;
Stars2::Stars2() 
{STACKTRACE
	int i;
	set_depth(DEPTH_STARS);
	if (!starpics) {
		DATAFILE *stardat = load_datafile("stars.dat");
		if (!stardat) i = 1;
		else for (i = 0; (stardat[i].type == DAT_RLE_SPRITE) || (stardat[i].type == DAT_BITMAP); i += 1) ;
		starpics = new SpaceSprite(stardat, i, 
			SpaceSprite::MASKED | 
			SpaceSprite::IRREGULAR | 
			SpaceSprite::MIPMAPED
		);
		unload_datafile(stardat);
	}
	num_layers = 0;
	layer = NULL;
	set_config_file("stars.ini");
	StarLayer tmp = {
		0,0,0,0,//size, wrap
		1.0,1.0,  //depth
		1.0,      //scale
		0,0,      //twinkle
		0,        //seed
		0,        //number
		0x0020,   //aa_mode
		0,        //type
		0,1       //pics
	};
	load_next( "SC2", &tmp );
	for (i = 0; i < num_layers; i += 1) {
		if (layer[i].seed == 0) layer[i].seed = rand() + (rand() << 16);
		if (layer[i].wx == 0) layer[i].wx = layer[i].sx;
		if (layer[i].wy == 0) layer[i].wy = layer[i].sx;
	}
}

void Stars2::load_next ( const char *sect, const StarLayer *old ) {STACKTRACE
	if (!sect) return;
	int l = num_layers;
	num_layers += 1;
	if (num_layers > 100 ) {
		tw_error("Stars::load_next - too many layers!!!");
		return;
	}
	layer = (StarLayer*) realloc ( layer, num_layers * sizeof(StarLayer) );

	layer[l].sx = get_config_float ( sect, "SizeX", old->sx );
	layer[l].sy = get_config_float ( sect, "SizeY", old->sy );
	layer[l].wx = get_config_float ( sect, "WrapX", old->wx );
	layer[l].wy = get_config_float ( sect, "WrapY", old->wy );
	layer[l].minz = get_config_float ( sect, "MinZ", old->minz );
	layer[l].maxz = get_config_float ( sect, "MaxZ", old->maxz );
	layer[l].scale = get_config_float ( sect, "Scale", old->scale );
	layer[l].twinkle_amp = get_config_float ( sect, "Tinkle", old->twinkle_amp );
	layer[l].twinkle_freq = get_config_float ( sect, "TinkleFreq", old->twinkle_freq );
	layer[l].seed = get_config_int ( sect, "Seed", old->seed );
	layer[l].type = get_config_int ( sect, "Type", old->type );
	layer[l].aa_mode = get_config_int ( sect, "AA_mode", old->aa_mode );
	layer[l].first_pic = get_config_int ( sect, "Image", old->first_pic);
	layer[l].num_pics = get_config_int ( sect, "NumImages", old->num_pics);
	layer[l].n = get_config_int ( sect, "Number", old->n );
	int n = get_config_int ( sect, "Children", 0 );
	int j;
	for (j = 0; j < n; j += 1) {
		char buffy[16];
		sprintf(buffy, "Child%d", j);
		load_next ( get_config_string(sect, buffy, NULL), &layer[l]);
	}
}

Stars2::~Stars2() {STACKTRACE
	if (layer) delete layer;
}

void Stars2::animate ( Frame * space ) 
{STACKTRACE
//	return;
	if (starpics->frames() == 0) return;
	int i;
	for (i = 0; i < num_layers; i += 1) {
		StarLayer *l = &layer[i];
		if (!l->n) continue;
		if (l->type == 0) continue;
		double w = l->sx * space_zoom;
		double h = l->sy * space_zoom;
		int x, y;
		double d = space_zoom * l->minz; //pow(1.0 - depth / 260.0, (num_layers-layer)/(double)num_layers);
		for (y = 0; y * h < space->surface->h; y+=1) {
			for (x = 0; x * w < space->surface->w; x+=1) {
				fs_random_seed32(l->seed);
				_draw_starfield_raw ( 
					space, 
					starpics, 
					l->first_pic, l->num_pics, 
					l->n,
					(x+0.5)*w, (y+0.5)*h, 
					-space_center_nowrap.x * d + space_view_size.x / 2, 
					-space_center_nowrap.y * d + space_view_size.y / 2, 
					space_zoom * l->sx, space_zoom * l->sy, 
					space_zoom * l->wx, space_zoom * l->wy, 
					d,
					l->aa_mode
				);
			}
		}
	}
}



void Stars::select_view( View **view) {STACKTRACE
	v = view;
}

void Stars::_event( Event *e) {STACKTRACE
	if (e->type == Event::TW_CONFIG) {
		ConfigEvent *ce = (ConfigEvent *) e;
		if (0) ;
		else if (!strcmp(ce->name, "server.ini/stars/depth")) {
			switch (ce->subtype) {
				case ConfigEvent::GET: {
					char blah[64];
					sprintf(ce->value, "%f", depth);
					ce->value = strdup(blah);
				} break;
				case ConfigEvent::SET: {
					depth = atoi(ce->value);
				} break;
				case ConfigEvent::FIND: {
					//later
				} break;
			}
		}
	}
	return;
}

Stars::Stars() {STACKTRACE
	int i;
	v = NULL;
	set_depth(DEPTH_STARS);
	DATAFILE *stardat = load_datafile("stars.dat");
	if (!stardat) {
		tw_error("stars.dat not found!");
		num_pics = 0;
		pic = NULL;
		num_stars = 0;
		num_layers = 0;
		return;
	}
	for (i = 0; (stardat[i].type == DAT_RLE_SPRITE) || (stardat[i].type == DAT_BITMAP); i += 1) ;
	num_pics = i;
	pic = new SpaceSprite*[num_pics];
	seed = tw_random();
	seed = (seed * (Uint64)tw_random()) >> 16;
	seed += tw_random();
	for(i = 0; i < num_pics; i++) {
		pic[i] = new SpaceSprite(&stardat[i], 1);
		}
	game->log_file("server.ini");
	width = get_config_int("Stars", "Width", 4000);
	height = get_config_int("Stars", "Height", 4000);
	num_stars = get_config_int("Stars", "Number", 150);
	num_layers = get_config_int("Stars", "Layers", 5);
	depth = get_config_int("Stars", "Depth", 192);
	set_config_file("client.ini");
	aa_mode = get_config_int("Stars", "Quality", 5);
	unload_datafile(stardat);
	}
/*
Stars::Stars() {
	int i;
	v = NULL;
	set_depth(DEPTH_STARS);
	num_pics = 3;
	pic = new SpaceSprite*[num_pics];
	seed = tw_random();
	seed ^= ((Uint64)tw_random()) << 24;
	seed ^= ((Uint64)tw_random()) << 48;
	for(i = 0; i < num_pics; i++) {
		pic[i] = new SpaceSprite(&(melee[MELEE_STAR + i]), 1);
		}
	game->log_file("server.ini");
	width = get_config_int("Stars", "Width", 4000);
	height = get_config_int("Stars", "Height", 4000);
	num_stars = get_config_int("Stars", "Number", 150);
	num_layers = get_config_int("Stars", "Layers", 5);
	depth = get_config_int("Stars", "StarDepth", 192);
	set_config_file("client.ini");
	aa_mode = get_config_int("Stars", "Quality", 5);
	}
*/
Stars::~Stars() {STACKTRACE
	int i;
	for(i = 0; i < num_pics; i++) delete pic[i];
	delete pic;
	}


void Stars::animate(Frame *space) {STACKTRACE
	if (v && (space_view != *v)) return;

	double d = space_zoom;
	double w = width * d;
	double h = height * d;
	int x, y, layer;
	for (layer = 0; layer < num_layers; layer += 1) {
		d = space_zoom * pow(1.0 - depth / 260.0, (num_layers-layer)/(double)num_layers);
		for (y = 0; y * h < space->surface->h; y+=1) {
			for (x = 0; x * w < space->surface->w; x+=1) {
				fs_random_seed32(seed + layer);
				if (aa_mode & 0x80000000) {
					_draw_starfield_cached ( 
						space, 
						pic[layer%num_pics], 
						0,
						num_stars / num_layers,
						(x+0.5)*w, (y+0.5)*h, 
						-space_center_nowrap.x * d + space_view_size.x / 2, 
						-space_center_nowrap.y * d + space_view_size.y / 2, 
						w, h, 
						w, h, 
						d + d*fabs(50-(((game->game_time / 10 + layer * 70)) % 100)) / 100.0,
						aa_mode
					);
				}
				else {
					_draw_starfield_raw ( 
						space, 
						pic[layer%num_pics], 0, 1,
						num_stars / num_layers,
						(x+0.5)*w, (y+0.5)*h, 
						-space_center_nowrap.x * d + space_view_size.x / 2, 
						-space_center_nowrap.y * d + space_view_size.y / 2, 
						w, h, 
						w, h, 
						d + d*fabs(50-(((game->game_time / 10 + layer * 70)) % 100)) / 100.0,
						aa_mode & 0x7fffFFFF
					);
				}
			}
		}
	}
	return;
}
