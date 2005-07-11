/* $Id$ */ 
#include <allegro.h>

#include <math.h>
#include <string.h>
#include <stdio.h>


#include "../melee.h"
REGISTER_FILE
#include "../libs.h"
#include "../util/pmask.h"
#include "../util/aastr.h"
#include "../frame.h"
#include "../scp.h"

#include "mview.h"

BITMAP *bmp_rot(BITMAP *ref, double a);

bool showshademaps = false;

const int RotAngles = 64;	// number of rotation-angles
const int RotSize = 128;	// max size of the image, should be a power of 2 ?
int xy_rotation_table[2*RotAngles*RotSize*RotSize];

bool initialized_rotations = false;
void init_rotation_table();

bool use_shademaps = false;



// should really be an array of angles/factors
BITMAP *SpaceSprite::add_shades(BITMAP *ref, double amb, Light *light, int Nlights, double ref_angle)
{
	if (!shademap)
		return 0;

	int w, h;


	w = ref->w;
	h = ref->h;

	if (w > 125 || h > 125)
		return 0;

	double *totlight = new double [w*h];	// rough first version, should be r,g,b

	int ilight;
	for ( ilight = 0; ilight < Nlights; ++ilight )
	{
		double a = light[ilight].angle + ref_angle;
		while (a < 0)
			a += PI2;
		while (a >= PI2)
			a -= PI2;

		int index;
		index = a * count / PI2;

		//message.print(1500, 15, "index[%i]", index);

		int k;
		k = index * w * h;

		int m = 0;

		int ix, iy;
		for ( iy = 0; iy < h; ++iy )
		{
			for ( ix = 0; ix < w; ++ix )
			{
				if (ilight == 0)
					totlight[m] = 0;

				totlight[m] += light[ilight].intensity * shademap[k];

				++k;
				++m;
			}
		}
	}

	BITMAP *dest = create_bitmap_ex(32, w, h);
	clear_to_color(dest, makecol(255,0,255));

	int m;
	m = 0;

	int ix, iy;
	for ( iy = 0; iy < h; ++iy )
	{
		for ( ix = 0; ix < w; ++ix )
		{
			// only needed to skip transparent colors.
			int col;
			col = getpixel(ref, ix, iy);
			if (col == makecol(255,0,255))
			{
				++m;
				continue;
			}
			
			// the average (or reference, ambient-only image), and the extra
			// lights hitting the ship (casting shadows)
			int r, g, b;
			r = amb*rmap[m] + totlight[m] * rmap[m];
			g = amb*gmap[m] + totlight[m] * gmap[m];
			b = amb*bmap[m] + totlight[m] * bmap[m];

			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;

			putpixel(dest, ix, iy, makecol(r, g, b));

			++m;
		}
	}

	delete[] totlight;

	return dest;
}


void SpaceSprite::init_shademaps()
{
	if (!initialized_rotations)
	{
		initialized_rotations = true;
		init_rotation_table();
	}

	
	shademap = new double [count * w * h];

	// the reference intensity of the different colors, needed to scale the
	// light that falls on the ship.
	rmap = new double [w * h];
	gmap = new double [w * h];
	bmap = new double [w * h];

	int n;

	int k;


	// first, create an average intensity map of the ship:
	double *Iref = new double [w*h];

	for ( n = 0; n < count; ++n )
	{
		k = 0;
		BITMAP *bmp = bmp_rot(b[0][n], -n*PI2/count);

		int ix, iy;
		for ( iy = 0; iy < h; ++iy )
		{
			for ( ix = 0; ix < w; ++ix )
			{
				int col;
				col = getpixel(bmp, ix, iy);

				int r, g, b;
				r = getr(col);
				g = getg(col);
				b = getb(col);

				double t;
				if (r == 255 && g == 0 && b == 255)
				{
					r = 0;
					g = 0;
					b = 0;
				}

				t = r + g + b;

				if (n == 0)
				{
					Iref[k] = 0;
					rmap[k] = 0;
					gmap[k] = 0;
					bmap[k] = 0;
				}

				Iref[k] += t/3;

				rmap[k] += r;
				gmap[k] += g;
				bmap[k] += b;

				if (n == count-1)
				{
					Iref[k] /= count;
					rmap[k] /= 256 * count;		// scaled between 0 and 1
					gmap[k] /= 256 * count;
					bmap[k] /= 256 * count;
				}

				++k;
			}
		}

		destroy_bitmap(bmp);
	}

	// visual test
	BITMAP *bmp_av = create_bitmap_ex(32, b[0][0]->w, b[0][0]->h);	

	// create rotated shade maps from the images in the .dat file.
	k = 0;
	for ( n = 0; n < count; ++n )
	{
		BITMAP *bmp = bmp_rot(b[0][n], -n*PI2/count);

		BITMAP *test = create_bitmap_ex(32, bmp->w, bmp->h);
		clear_to_color(test, 0);

		int m = 0;
		int ix, iy;
		for ( iy = 0; iy < h; ++iy )
		{
			for ( ix = 0; ix < w; ++ix )
			{
				//int refcol;
				//refcol = getpixel(b[0][0], ix, iy);

				int newcol;
				newcol = getpixel(bmp, ix, iy);

				double t1, t2;

				int r, g, b;
				r = getr(newcol);
				g = getg(newcol);
				b = getb(newcol);

				if (r == 255 && g== 0 && b == 255)	// transparent color.
				{
					shademap[k] = 0;
					++k;
					++m;
					continue;
				}

				t1 =  r + g + b;
				//t2 = getr(refcol) + getg(refcol) + getb(refcol);
				t2 = Iref[m] * 3;

				shademap[k] = t1 / t2;


				// for testing
				int c = 150*shademap[k];
				if (c > 255) c = 255;
				putpixel(test, ix, iy, makecol(c,c,c));

				c = Iref[m];
				putpixel(bmp_av, ix, iy, makecol(c,c,c));
				// end testing

				++k;
				++m;
			}
		}

		// test is disabled
		if (false && showshademaps && bmp->w > 60)
		{
			int x = 120;
			blit(b[0][0], screen, 0, 0, x,100, bmp->w, bmp->h);
			x += bmp->w;
			blit(bmp, screen, 0, 0, x,100, bmp->w, bmp->h);
			x += bmp->w;
			blit(bmp_av, screen, 0, 0, x,100, bmp->w, bmp->h);
			x += bmp->w;
			blit(test, screen, 0, 0, x,100, bmp->w, bmp->h);
			readkey();
		}

		destroy_bitmap(bmp);
	}

	destroy_bitmap(bmp_av);

	delete[] Iref;


	/* this does not work well - it's too unstable, cause the
	original data suck
	// enhance shadows, cause often they're hardly visible !!!!

	double min = 1E6;
	double max = 0;
	for ( k = 0; k < count*w*h; ++k )
	{
		if (shademap[k] > 0)
		{
			if (shademap[k] < min)
				min = shademap[k];
			if (shademap[k] > max)
				max = shademap[k];
		}
	}
	
	if (showshademaps && w > 60)
	{
		int x = 1;
	}

	if (max > 1)
		max = 1;

	for ( k = 0; k < count*w*h; ++k )
	{
		shademap[k] = (shademap[k] - min) / (max - min);
	}
	*/
}


void SpaceSprite::destroy_shademaps()
{
	delete[] shademap;
}


//const int RotAngles = 64;	// number of rotation-angles
//const int RotSize = 128;	// max size of the image, should be a power of 2 ?
//int xy_rotation_table[2*RotAngles*RotSize*RotSize];

//bool initialized_rotations = false;
void init_rotation_table()
{
	int w = RotSize / 2;

	int k = 0;

	int ia;
	for (ia = 0; ia < RotAngles; ++ia)
	{
		double a = ia * PI2 / RotAngles;

		int ix, iy;
		for ( iy = -w; iy < w; ++iy )
		{
			for ( ix = -w; ix < w; ++ix )
			{
				double x, y;
				x = ix + 0.5;
				y = iy + 0.5;

				int px, py;
				px = iround( x * cos(-a) - y * sin(-a) );
				py = iround( y * cos(-a) + x * sin(-a) );

				xy_rotation_table[k] = px;
				++k;

				xy_rotation_table[k] = py;
				++k;
			}
		}
	}
	
}


BITMAP *bmp_rot(BITMAP *ref, double a)
{
	if (!ref)
		return 0;
	
	while (a < 0)
		a += PI2;
	while (a >= PI2)
		a -= PI2;

	if (!initialized_rotations)
	{
		initialized_rotations = true;
		init_rotation_table();
	}

	int w, h;
	w = ref->w;
	h = ref->h;

	BITMAP *dest;
	dest = create_bitmap_ex(32, w, h);
	clear_to_color(dest, makecol(255,0,255));


	if (w > RotSize || h > RotSize)
	{
		blit(ref, dest, 0, 0, 0, 0, w, h);
		return dest;
	}

	// rotate a bitmap ...

	int ia;
	ia = iround(a * 64 / PI2);
	
	if (ia < 0)
		ia = 0;
	
	if (ia > RotAngles - 1)
		ia = RotAngles - 1;

	int k;
	k = 2 * ia*RotSize*RotSize;

	// the default rotation picture is usually too big - get the difference here.
	int dx1, dy1, dx2, dy2;
	dx1 = (RotSize - w) / 2;
	dy1 = (RotSize - h) / 2;
	dx2 = RotSize - w - dx1;
	dy2 = RotSize - h - dy1;

	k += 2 * dy1 * RotSize;

	int ix, iy;
	for ( iy = 0; iy < h; ++iy )
	{
		k += 2 * dx1;

		for ( ix = 0; ix < w; ++ix )
		{
			int px, py;
			px = xy_rotation_table[k] + w/2;
			++k;
			py = xy_rotation_table[k] + h/2;
			++k;

			if (px > 0 && px < w && py > 0 && py < h)
			{
				int col;
				col = getpixel(ref, px, py);
				putpixel(dest, ix, iy, col);
			}
		}

		k += 2 * dx2;
	}


	return dest;
}


int tw_aa_mode = 0;
void set_tw_aa_mode ( int a) {
	tw_aa_mode = a;
}
int get_tw_aa_mode () {
	return tw_aa_mode;
}

int string_to_sprite_attributes ( const char *s, int recommended ) {STACKTRACE
	int a = recommended;
	if (!s) return a;

	if (strstr(s, "+screen")) {
		a |= SpaceSprite::MATCH_SCREEN_FORMAT;
	}
	if (strstr(s, "-screen")) {
		a &=~SpaceSprite::MATCH_SCREEN_FORMAT;
	}

	if (strstr(s, "+mipmap")) {
		a |= SpaceSprite::MIPMAPED;
	}
	if (strstr(s, "-mipmap")) {
		a &=~SpaceSprite::MIPMAPED;
	}

	if (strstr(s, "+irregular")) {
		a |= SpaceSprite::IRREGULAR;
	}
	if (strstr(s, "-irregular")) {
		a &=~SpaceSprite::IRREGULAR;
	}

	if (strstr(s, "+masked")) {
		a |= SpaceSprite::MASKED;
	}
	if (strstr(s, "-masked")) {
		a &=~SpaceSprite::MASKED;
	}

	if (strstr(s, "+alpha")) {
		a |= SpaceSprite::ALPHA;
		a &=~SpaceSprite::MATCH_SCREEN_FORMAT;//remove me
	}
	if (strstr(s, "-alpha")) {
		a &=~SpaceSprite::ALPHA;
	}

	if (strstr(s, "+dither")) {
		a |= SpaceSprite::DITHER;
	}
	if (strstr(s, "-dither")) {
		a &=~SpaceSprite::DITHER;
	}

	if (strstr(s, "+noaa")) {
		a |= SpaceSprite::NO_AA;
	}
	if (strstr(s, "-noaa")) {
		a &=~SpaceSprite::NO_AA;
	}
	return a;
}

void handle_alpha_load ( BITMAP *bmp ) {
	switch (bitmap_color_depth(bmp)) {
		case 16: {
			rgba4444_as_rgb16 (bmp);
			//invert_alpha(bmp);
		} break;
		case 32: {
			convert_alpha(bmp, 1);
		} break;
		default: {
		} break;
	}
	return;
}

void line ( BITMAP *dest, Vector2 p1, Vector2 p2, int color ) {
	line (dest, iround(p1.x), iround(p1.y), iround(p2.x), iround(p2.y), color);
}
void line ( Frame *dest, Vector2 p1, Vector2 p2, int color ) {
	int x1, y1, x2, y2;
	x1 = iround(p1.x);	y1 = iround(p1.y);
	x2 = iround(p2.x);	y2 = iround(p2.y);
	line(dest->surface, x1, y1, x2, y2, color);
	dest->add_line(x1, y1, x2, y2);
}


int SpaceSprite::mip_min = 0;
int SpaceSprite::mip_max = 8;
int SpaceSprite::mip_bias = 0;
inline int find_mip_level(double r, int highest) {
	int a;
	frexp(r, &a);
	a = SpaceSprite::mip_bias - a;
	if (a < SpaceSprite::mip_min) a = SpaceSprite::mip_min;
	if (a > SpaceSprite::mip_max) a = SpaceSprite::mip_max;
	if (a > highest) a = highest;
	return a;
}
//COMPILE_TIME_ASSERT(SpaceSprite::DITHER == AA_DITHER << 16);
//COMPILE_TIME_ASSERT(SpaceSprite::MASKED == AA_MASKED << 16);
//COMPILE_TIME_ASSERT(SpaceSprite::ALPHA  == AA_ALPHA << 16);
inline int find_aa_mode(int general_options) {
	int aa = tw_aa_mode;
	if (general_options & SpaceSprite::MASKED) aa |= AA_MASKED;
	if (~general_options & SpaceSprite::ALPHA) aa &=~AA_ALPHA;
	if (general_options & SpaceSprite::NO_AA)  {
		aa |= AA_NO_AA;
	}
	return aa;
}



void convert_bitmap(BITMAP *src, BITMAP *dest, int aa_mode) {STACKTRACE
	//requires that both be memory bitmaps
	int obpp = bitmap_color_depth(src);
	int bpp = bitmap_color_depth(dest);
	if ((src->w != dest->w) || (src->h != dest->h)) {tw_error("convert_bitmap - wrong size");}
	//if (obpp == bpp) {tw_error("convert_bitmap - color depths match");}
	if ((obpp == bpp) || !(aa_mode & AA_MASKED)) {
		blit(src, dest, 0, 0, 0, 0, src->w, src->h);
		return;
		}
/*	int x, y, om, nm, nnm;
	om = bitmap_mask_color(src);
	nm = bitmap_mask_color(dest);
	if (nm == 0) {
		int i = 0;
		nnm = 0;
		while (nnm == nm) {
			nnm = makecol_depth(bpp, i, 0, i);
			i += 1;
			}
		}
	else nnm = nm - 1;
	for (y = 0; y < src->h; y += 1) {
		for (x = 0; x < src->w; x += 1) {
			int oc = getpixel(src, x, y);
			if (oc == om || oc == 0) {
				putpixel(dest, x, y, nm);
				continue;
				}
			int r, g, b;
			r = getr_depth(obpp, oc);
			g = getg_depth(obpp, oc);
			b = getb_depth(obpp, oc);
			int nc = makecol_depth(bpp, r, g, b);
			if (nc == nm) nc = nnm;
			putpixel(dest, x, y, nc);
			}
		}*/
	aa_set_mode(aa_mode);
	aa_stretch_blit(src, dest, 
		0, 0, src->w, src->h, 
		0, 0, dest->w, dest->h
		);
	return;
	}
/*
void color_correct_bitmap(BITMAP *bmp, int masked) {STACKTRACE
	//return;//remove me!!!
	int x, y, w, h;
	w = bmp->w;
	h = bmp->h;
	int bmc = bitmap_mask_color(bmp);
	for (y = 0; y < h; y += 1) {
		for (x = 0; x < w; x += 1) {
			RGB a;
			int c;
			c = getpixel(bmp, x, y);
			if ((c == bmc) && masked) continue;
			switch (bitmap_color_depth(bmp)) {
				case 8:{
					a.r = getr8(c);
					a.g = getg8(c);
					a.b = getb8(c);
				}
				break;
				case 15:{
					a.r = getr15(c);
					a.g = getg15(c);
					a.b = getb15(c);
				}
				break;
				case 16:{
					a.r = getr16(c);
					a.g = getg16(c);
					a.b = getb16(c);
				}
				break;
				case 24:{
					a.r = getr24(c);
					a.g = getg24(c);
					a.b = getb24(c);
				}
				break;
				case 32:{
					a.r = getr32(c);
					a.g = getg32(c);
					a.b = getb32(c);
				}
				break;
			}
			videosystem.color_effects(&a);
			c = makecol_depth(bitmap_color_depth(bmp), a.r, a.g, a.b);
			if ((c == bmc) && masked) {
				if (c == 0) c = makecol(10,10,10);
				else c = makecol(255, 16, 255);
				}
			putpixel(bmp, x, y, c);
			}
		}
	return;
	}
*/
void color_correct_bitmap(BITMAP *bmp, int masked) {STACKTRACE
	//return;//remove me!!!
	int x, y, w, h;
	w = bmp->w;
	h = bmp->h;
	int bmc = bitmap_mask_color(bmp);
	for (y = 0; y < h; y += 1) {
		for (x = 0; x < w; x += 1) {
			RGB a;
			a.filler = 0;
			int c;
			c = getpixel(bmp, x, y);
			if ((c == bmc) && masked) continue;
			switch (bitmap_color_depth(bmp)) {
				case 8:{
					a.r = getr8(c);
					a.g = getg8(c);
					a.b = getb8(c);
				}
				break;
				case 15:{
					a.r = getr15(c);
					a.g = getg15(c);
					a.b = getb15(c);
				}
				break;
				case 16:{
					a.r = getr16(c);
					a.g = getg16(c);
					a.b = getb16(c);
				}
				break;
				case 24:{
					a.r = getr24(c);
					a.g = getg24(c);
					a.b = getb24(c);
				}
				break;
				case 32:{
					a.r = getr32(c);
					a.g = getg32(c);
					a.b = getb32(c);
					a.filler = geta32(c);
				}
				break;
			}
			videosystem.color_effects(&a);
			c = makeacol_depth(bitmap_color_depth(bmp), a.r, a.g, a.b, a.filler);
			if ((c == bmc) && masked) {
				if (c == 0) c = makecol(10,10,10);
				else c = makecol(255, 16, 255);
				}
			putpixel(bmp, x, y, c);
			}
		}
	return;
	}

void SpaceSprite::generate_mipmaps() {STACKTRACE
	int bpp, level, i;
	if (general_attributes & MIPMAPED) {
		bpp = bitmap_color_depth(b[0][0]);
		for (level = 1; level < MAX_MIP_LEVELS; level += 1) {
			int lw, lh;
			lw = iround(w * pow(0.5, level));
			lh = iround(h * pow(0.5, level));
			if ((lw < 8) || (lh < 8)) break;
			this->highest_mip = level;
			this->b[level] = new BITMAP*[count];
			for (i = 0; i < count; i += 1) {
				BITMAP *src = b[0][i];
				b[level][i] = create_bitmap_ex(bpp, lw, lh);
				BITMAP *dest = b[level][i];
				if (general_attributes & MASKED) clear_to_color(dest, bitmap_mask_color(dest));
				//_aa2_stretch_blit(b[level-1][i], bmp, 0, 0, lw, lh, general_attributes & MASKED);
				//_aa2_stretch_blit(this->b[0][i], bmp, 0,0,bmp->w,bmp->h, 0, 0, lw, lh, (general_attributes & MASKED) >> 16);
				int a = find_aa_mode(general_attributes);
				if (a & AA_ALPHA) a |= AA_RAW_ALPHA;
				a |= AA_MASKED_DEST;
				a &=~AA_BLEND;
				aa_set_mode( a );
				aa_stretch_blit(src, dest, 0,0,src->w,src->h, 0,0,dest->w, dest->h );
				//aa_stretch_blit(b[0][i], b[level][i], 0, 0, w, h, 0, 0, lw, lh);
			}
		}
	}
}

void SpaceSprite::change_color_depth(int newbpp) {STACKTRACE
	int i, l;
	for (l = 0; l <= highest_mip; l += 1) {
		for (i = 0; i < count; i += 1) {
			BITMAP *tmp = create_bitmap_ex(newbpp, w, h);
			convert_bitmap(b[l][i], tmp, (general_attributes & MASKED) ? AA_MASKED : 0);
			if (attributes[i] & DEALLOCATE_IMAGE) destroy_bitmap(b[l][i]);
			attributes[i] |= DEALLOCATE_IMAGE;
			b[l][i] = tmp;
		}
	}
	return;
}

void SpaceSprite::permanent_phase_shift ( int phase ) {STACKTRACE
	int i, mip;
	Surface **tmp = new Surface*[count];
	while (phase < 0) phase += count;
	for (mip = 0; mip <= highest_mip; mip += 1) {
		for (i = 0; i < count; i += 1) {
			tmp[i] = b[mip][(i + phase) % count];
		}
		for (i = 0; i < count; i += 1) {
			b[mip][i] = tmp[i];
		}
	}
	delete[] tmp;

	PMASK **tmp2 = new PMASK*[count];
	while (phase < 0) phase += count;
	for (mip = 0; mip <= highest_mip; mip += 1) {
		for (i = 0; i < count; i += 1) {
			tmp2[i] = m[(i + phase) % count];
		}
		for (i = 0; i < count; i += 1) {
			m[i] = tmp2[i];
		}
	}
	delete[] tmp2;
	return;
}


Vector2 SpaceSprite::size(int i)  const
{
	// in case the sprite is irregular, you cannot return a default size, but must check
	// each bitmap size.

	return Vector2(b[0][i]->w, b[0][i]->h);
}

SpaceSprite::SpaceSprite(const DATAFILE *images, int sprite_count, int _attributes, int rotations) {
	STACKTRACE
	int i, j, obpp=0;
	BITMAP *bmp, *tmp = NULL;

	shademap = 0;

	if (_attributes == -1) _attributes = string_to_sprite_attributes(NULL);

	count = sprite_count * rotations;
	if ((rotations < 1) || (count < 1)) {tw_error("SpaceSprite::SpaceSprite - bad parameters");}

	references = 0;
	highest_mip = 0;
	for (i = 1; i < MAX_MIP_LEVELS; i += 1) {
		b[i] = NULL;
	}

	general_attributes = _attributes;
	if (general_attributes &  MATCH_SCREEN_FORMAT) {
		bpp = videosystem.bpp;
		if (general_attributes & ALPHA) {
			if (bpp <= 16) bpp = 16;
			else bpp = 32;
			bpp = 32; //quick hack, because converting to 16bpp isn't quite working properly
		}
	}
	else bpp = 0;

	originaltype = images[0].type;
	switch (originaltype) {
		case DAT_RLE_SPRITE: {
			w = ((RLE_SPRITE *)images[0].dat)->w;
			h = ((RLE_SPRITE *)images[0].dat)->h;
			obpp = ((RLE_SPRITE *)images[0].dat)->color_depth;
			}
		break;
		case DAT_BITMAP: {
			w = ((BITMAP *)images[0].dat)->w;
			h = ((BITMAP *)images[0].dat)->h;
			obpp = bitmap_color_depth((BITMAP *)images[0].dat);
			}
		break;
		default: {
			{tw_error("SpaceSprite::SpaceSprite - bad data file");}
			}
		break;
		}

	if (bpp == 0) bpp = obpp;
	if (obpp != bpp) tmp = create_bitmap_ex(obpp, w, h);
	if (general_attributes & ALPHA) {
		if (bpp <= 16) bpp = 16;
		else bpp = 32;
	}


	if (general_attributes & IRREGULAR) goto irregular;
	for(i = 1; i < sprite_count; i++) {
		if (images[i].type != originaltype) {tw_error ("SpaceSprite - bad data file");}
		switch (originaltype) {
			case DAT_RLE_SPRITE: {
				if ((w != ((RLE_SPRITE *)images[i].dat)->w) || 
						(h != ((RLE_SPRITE *)images[i].dat)->h) ) {tw_error("SpaceSprite - size changed");}
				if (((RLE_SPRITE *)images[i].dat)->color_depth != obpp) {tw_error("SpaceSprite - changing source color depth not yet allowed");}
				}
			break;
			case DAT_BITMAP: {
				if ((w != ((BITMAP *)images[i].dat)->w) || 
					(h != ((BITMAP *)images[i].dat)->h) ) {tw_error("SpaceSprite - size changed");}
				if (bitmap_color_depth((BITMAP *)images[i].dat) != obpp) {tw_error("SpaceSprite - changing source color depth not yet allowed");}
				}
			break;
			}
		}

	m = new PMASK*[count];
	b[0] = new BITMAP*    [count];
	attributes  = new char [count];

	for(i = 0; i < sprite_count; i += 1) {
		bmp = create_bitmap_ex(bpp, w, h);
		if (!tmp) tmp = bmp;
		if (general_attributes & MASKED) clear_to_color(bmp, bitmap_mask_color(bmp));
/*		if (general_attributes & MASKED) {
			if (general_attributes & ALPHA) {
				if (bpp == 16) clear_to_color(bmp, );
			}
			clear_to_color(bmp, bitmap_mask_color(bmp));
		}*/
		if (tmp != bmp) clear_to_color(tmp, bitmap_mask_color(tmp));
		switch (originaltype) {
			case DAT_RLE_SPRITE: {
				draw_rle_sprite(tmp, (RLE_SPRITE *)(images[i].dat), 0, 0);
				if (general_attributes & ALPHA) handle_alpha_load(tmp);
				if (tmp != bmp) convert_bitmap(tmp, bmp, (general_attributes & MASKED) ? AA_MASKED : 0);
				}
			break;
			case DAT_BITMAP: {
				if (general_attributes & ALPHA) handle_alpha_load((BITMAP *)(images[i].dat));
				if (obpp != bpp) {
					convert_bitmap((BITMAP *)(images[i].dat), bmp, (general_attributes & MASKED) ? AA_MASKED : 0);
					}
				else {
					if (general_attributes & MASKED) draw_sprite(bmp, (BITMAP*)images[i].dat, 0, 0);
					else blit((BITMAP*)images[i].dat, bmp, 0, 0, 0, 0, w, h);
					}
				}
			break;
			}
		color_correct_bitmap(bmp, general_attributes & MASKED);
//		make_alpha(bmp);
		if (tmp == bmp) tmp = NULL;
		for (j = 1; j < rotations; j += 1) {
			BITMAP *tmp = create_bitmap_ex(bpp, w, h);
			clear_to_color(tmp, bitmap_mask_color(tmp));
			rotate_sprite(tmp, bmp, 0, 0, j * ((1<<24)/rotations));
			m[j + (i * rotations)] = create_allegro_pmask(tmp);
			b[0][j + (i * rotations)] = tmp;
			attributes[j + (i * rotations)] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
			}
		m[(i * rotations)] = create_allegro_pmask(bmp);
		b[0][(i * rotations)] = bmp;
		attributes[(i * rotations)] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
		}
	if (general_attributes & MIPMAPED) {
		generate_mipmaps();
/*		for (int level = 1; level < MAX_MIP_LEVELS; level += 1) {
			int lw, lh;
			lw = (int)ceil(w * pow(0.5, level));
			lh = (int)ceil(h * pow(0.5, level));
			if ((lw < 16) || (lh < 16)) continue;
			highest_mip = level;
			b[level] = new BITMAP*    [count];
			for (i = 0; i < count; i += 1) {
				BITMAP *bmp = create_bitmap_ex(bpp, lw, lh);
				b[level][i] = bmp;
				if (general_attributes & MASKED) clear_to_color(bmp, bitmap_mask_color(bmp));
				//_aa2_stretch_blit(b[level-1][i], bmp, 0, 0, lw, lh, general_attributes & MASKED);
				aa_set_mode(find_aa_mode(general_attributes));
				aa_stretch_blit(b[0][i], bmp, 0,0,bmp->w,bmp->h, 0, 0, lw, lh);
				//aa_stretch_blit(b[0][i], b[level][i], 0, 0, w, h, 0, 0, lw, lh);
			}
		}*/
	}


	if (use_shademaps)
		init_shademaps();

	return;//end of normal/masked/autorotated

	irregular:

	if (rotations != 1) {tw_error (" irregular SpaceSprites are not permitted to be autorotated");}

	m = new PMASK*[count];
	b[0] = new BITMAP*    [count];
	attributes  = new char [count];

	for(i = 0; i < sprite_count; i += 1) {
		switch (images[i].type) {
			case DAT_RLE_SPRITE: {
				w = ((RLE_SPRITE *)images[i].dat)->w;
				h = ((RLE_SPRITE *)images[i].dat)->h;
				obpp = ((RLE_SPRITE *)images[i].dat)->color_depth;
				if (!tmp) tmp = create_bitmap_ex ( obpp, w, h);
				if (general_attributes & MASKED) clear_to_color(tmp, bitmap_mask_color(tmp));
				draw_rle_sprite(tmp, (RLE_SPRITE *)(images[i].dat), 0, 0);
				if (general_attributes & ALPHA) handle_alpha_load(tmp);
				if (bpp == obpp) {
					bmp = tmp;
					tmp = NULL;
				}
				else {
					bmp = create_bitmap_ex ( bpp, w, h);

					// added, otherwise maybe the "convert" skips masked parts in
					// tmp, and those are then undefined in bmp.
					if (general_attributes & MASKED)
						clear_to_color(bmp, bitmap_mask_color(bmp));

					convert_bitmap(tmp, bmp, (general_attributes & MASKED) ? AA_MASKED : 0);
					destroy_bitmap(tmp);
					tmp = NULL;
					}
				}
			break;
			case DAT_BITMAP: {
				if (general_attributes & ALPHA) handle_alpha_load((BITMAP *)(images[i].dat));
				w = ((BITMAP *)images[i].dat)->w;
				h = ((BITMAP *)images[i].dat)->h;
				bmp = create_bitmap_ex ( bpp, w, h);
				convert_bitmap((BITMAP *)images[i].dat, bmp, general_attributes & MASKED);
				}
			break;
			}
		color_correct_bitmap(bmp, general_attributes & MASKED);
		m[(i * rotations)] = create_allegro_pmask(bmp);
		b[0][(i * rotations)] = bmp;
		attributes[(i * rotations)] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
		}
	return;//end of irregular/masked
	}

SpaceSprite::SpaceSprite(SpaceSprite &old) {
	STACKTRACE
	int i, l;
	BITMAP *bmp;
	count = old.count;
	bpp = old.bpp;
	highest_mip = old.highest_mip;
	originaltype = -1;
	w = old.w;
	h = old.h;
	m = new PMASK*[count];
	b[0] = new BITMAP*    [count];

	references = 0;
	attributes  = new char [count];
	general_attributes = old.general_attributes;

	for(i = 0; i < count; i++) {
		bmp = create_bitmap(old.b[0][i]->w, old.b[0][i]->h);
		blit(old.b[0][i], bmp, 0, 0, 0, 0, old.b[0][i]->w, old.b[0][i]->h);
		m[i] = create_allegro_pmask(bmp);
		b[0][i] = bmp;
		attributes[i] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
		}
	for (l = 1; l < MAX_MIP_LEVELS; l += 1) if (old.b[l])
		for(i = 0; i < count; i++) {
			bmp = create_bitmap(old.b[l][i]->w, old.b[l][i]->h);
			blit(old.b[l][i], bmp, 0, 0, 0, 0, old.b[l][i]->w, old.b[l][i]->h);
			b[l][i] = bmp;
		}
		else b[l] = NULL;
	}

void SpaceSprite::lock() {
	STACKTRACE
	highest_mip = 0;
	return;
}

void SpaceSprite::unlock() {
	STACKTRACE
	int i, j;
	for (i = 0; i < MAX_MIP_LEVELS; i += 1) {
		if (b[i]) j = i;
	}
	//We aught to rebuild mipmaps	but too much work, so for now we do nothing
	//highest_mip = j;
	return;
}
/*
SpaceSprite::SpaceSprite(const char *sourcename, const char *spritename) {
	int i;

	char buf[512];
	const char *tmpstr, *extension;

	sprintf(buf, "%s/%s.ini", sourcename, spritename);
	set_config_file(buf);
	tmpstr = get_config_string("Main", "Type", "BadType");
	if (strcmp(tmpstr, "SpaceSprite")) {tw_error("SpaceSprite(%s # %s) : %s != SpaceSprite", sourcename, spritename, tmpstr);}
	count = get_config_int("SpaceSprite", "Number", 0);
	w = get_config_int("SpaceSprite", "Width", 0);
	h = get_config_int("SpaceSprite", "Height", 0);
	//tmpstr = get_config_string("SpaceSprite", "SubType", "Normal");
	extension = get_config_string("SpaceSprite", "Extension", NULL);

	m = new PPMASK*[count];
	b = new BITMAP*    [count];
	attributes  = new unsigned char [count];

	general_attributes = MASKED;

	if (count) {
		for (i = 0; i < count; i += 1) {
			if (strchr(extension, '.')) {
				sprintf(buf, "%s/%s%03d%s", sourcename, spritename, i, extension);
				b[i] = load_bitmap(buf, NULL);
				m[i] = create_ppmask(b[i]);
				attributes[i] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
				}
			else {
				sprintf(buf, "%s%03d", spritename, i);
				//maybe force upper case here?
				b[i] = (BITMAP*)(load_datafile_object(sourcename, buf)->dat);
				m[i] = create_ppmask(b[i]);
				attributes[i] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
				}
			}
		}
	}

void save_spacesprite(SpaceSprite *ss, const char *sname, const char *dname, const char *extension) {
	int i;
	char buf[512];
	if (!ss) return;
	if (ss->frames()) {
		for (i = 0; i < ss->frames(); i += 1) {
			if (strchr(extension, '.')) {
				sprintf(buf, "tmp/%s%03d%s", sname, i, extension);
				}
			else {
				sprintf(buf, "tmp/%s%03d.bmp", sname, i);
				}
			save_bitmap(buf, ss->get_bitmap(i), NULL);
			}
		chdir("tmp");
//		if (strchr(extension, '.')) sprintf(buf, "dat ../ships/%s.dat -k -t data -a *", dname);
//		else sprintf(buf, "dat ../ships/%s.dat -t %s -a *", dname, extension);
//		system(buf);
		sprintf(buf, "md ..\\ships\\%s", dname);
		system(buf);
		sprintf(buf, "move * ..\\ships\\%s", dname);
		system(buf);
		delete_file("*");
		chdir("..");
		}
	sprintf(buf, "tmp/%s.ini", sname);
	set_config_file(buf);
	set_config_string("Main", "Type", "SpaceSprite");
	set_config_int("SpaceSprite", "Number", ss->frames());
	set_config_int("SpaceSprite", "Width", ss->width());
	set_config_int("SpaceSprite", "Height", ss->height());
	set_config_string("SpaceSprite", "SubType", "Normal");
	set_config_string("SpaceSprite", "Extension", extension);
	chdir("tmp");
	sprintf(buf, "dat ../ships/%s.dat -k -a *", dname);
//	system(buf);
	sprintf(buf, "move * ..\\ships\\%s", dname);
	system(buf);
//	delete_file("*");
	chdir("..");
	return;
	}
SpaceSprite::SpaceSprite(const char *package, const char *name) {
	const char *ext;
	char buf[512];

	sprintf(buf, "%s#%s.ini", package);
	set_config_file(buf);
	count = get_config_int(name, "Number", 0);
	rotations = get_config_int(name, "Rotation", 1);
	w = get_config_int(name, "Width", 1);
	h = get_config_int(name, "Height", 1);
	if (strcmp(get_config_string(name, "Type", NULL), "SpaceSprite"))
		{tw_error("Error loading SpaceSprite: that's not a SpaceSprite!");}
	ext = get_config_string(name, "Extension", "");

	m = new PPMASK*[count * rotations];
	b = new BITMAP*    [count * rotations];
	for (int i = 0; i < count; i += 1) {
		sprintf(buf, "%s/%s%03d%s", package, name, i, ext);
		BITMAP *bmp = load_bitmap(buf, NULL);
		b[i * rotations] = bmp;
		m[i * rotations] = create_ppmask(bmp);
		if (!m[i * rotations]) {tw_error ("Error loading SpaceSprite: image %s not found", buf);}
		for (int j = 1; j < rotations; j += 1) {
			BITMAP *tmp = create_bitmap(w, h);
			clear_to_color(tmp, bitmap_mask_color(tmp));
			aa_rotate_sprite(tmp, bmp, 0, 0, j * (1<<24)/rotations);
			b[j + i * rotations] = tmp;
			m[j + i * rotations] = create_ppmask(tmp);
			}
		}
	count *= rotations;

	return;
	}

SpaceSprite::SpaceSprite(BITMAP **sprites, int sprite_count) {
	int i, j;
	BITMAP *bmp;
	rotations = 1;
	count = sprite_count;
	if (count < 1) {tw_error("SpaceSprite::SpaceSprite - bad parameters (2)");}
	m = new PPMASK*[count * rotations];
	b = new BITMAP*    [count * rotations];
	w = sprites[0]->w;
	h = sprites[0]->h;
	for(i = 0; i < count; i++) {
		m[(i * rotations)] = create_ppmask(sprites[i]);
		b[(i * rotations)] = sprites[i];
		}
	}
*/
SpaceSprite::~SpaceSprite() {
	STACKTRACE
	int i, l;

	for(i = 0; i < count; i++) {
		if (attributes[i] & DEALLOCATE_MASK) destroy_pmask(m[i]);
		}
	delete[] m;
	m = NULL;

	for (l = 0; l <= highest_mip; l += 1) {
		for(i = 0; i < count; i++) {
			if (attributes[i] & DEALLOCATE_IMAGE) destroy_bitmap(b[l][i]);
		}
		delete b[l];
		b[l] = NULL;
	}

	delete[] attributes;
	attributes = NULL;
	return;
}

BITMAP *SpaceSprite::get_bitmap(int index, int miplevel)
{STACKTRACE
	// changed ROB
	//if (general_attributes & MIPMAPED) if (highest_mip > 0) 
	//	{tw_error ("get_bitmap on a mipmaped sprite!\n(retry likely to work)");}
	if (general_attributes & MIPMAPED) if (miplevel > highest_mip) 
		{tw_error ("get_bitmap on undefined mipmap level");}
	if (index >= count) {tw_error("SpaceSprite::get_bitmap - index %d > count %d", index, count);}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count);}
	// changed ROB
	//highest_mip = 0;
	// changed ROB
	return(b[miplevel][index]);
}
BITMAP *SpaceSprite::get_bitmap_readonly(int index)
{STACKTRACE
	if (index >= count) {tw_error("SpaceSprite::get_bitmap_readonly - index %d >= count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap_readonly - index %d < 0 (count %d)", index, count); index = 0;}
	return(b[0][index]);
}

void SpaceSprite::animate_character(Vector2 pos, int index, int color, Frame *space, double scale) {
	STACKTRACE
	if (index >= count) {
		tw_error("SpaceSprite::animate_character - index %d >= count %d", index, count);
		index = 0;
	}
	if (index < 0) {
		tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count);
		index = 0;
	}
	pos = corner(pos, size());
	int ix = iround(pos.x);
	int iy = iround(pos.y);
	int spr_w = iround(w * space_zoom * scale);
	int spr_h = iround(h * space_zoom * scale);
	if(spr_w < 1) spr_w = 1;
	if(spr_h < 1) spr_h = 1;
	draw_character(ix, iy, spr_w, spr_h, index, color, space);
	return;
}

void SpaceSprite::overlay (int index1, int index2, BITMAP *dest) {
	STACKTRACE
	int x, y;
	if (general_attributes & MIPMAPED) {tw_error ("overlay on a mipmaped sprite!  oh no!");}
	if (index1 > count) {tw_error("SpaceSprite::overlay - index1 %d > count %d", index1, count);}
	if (index2 > count) {tw_error("SpaceSprite::overlay - index2 %d > count %d", index2, count);}

	for(y = 0; y < h; y += 1) {
		for(x = 0; x < w; x += 1) {
			if (getpixel(b[0][index1], x, y) != getpixel(b[0][index2], x, y)) {
				putpixel(dest, x, y, getpixel(b[0][index2], x, y));
			}
		}
	}
	return;
}



void animate_bmp(BITMAP *bmp, Vector2 p, Vector2 s, Frame *space)
{
	int ix, iy, iw, ih;

	if (p.x + s.x < 0) return;
	if (p.x >= space->surface->w) return;
	if (p.y + s.y < 0) return;
	if (p.y >= space->surface->h) return;
	
	ix = iround(p.x);
	iy = iround(p.y);
	iw = iround(s.x);
	ih = iround(s.y);
	
	aa_stretch_blit(bmp, space->surface, 0,0,bmp->w,bmp->h, ix, iy, iw, ih);
	space->add_box(ix, iy, iw, ih);
}

void animate_bmp(BITMAP *bmp, Vector2 pos, Frame *space)
{
	Vector2 s = Vector2(bmp->w, bmp->h);
	Vector2 p = corner(pos, s );
	s *= space_zoom;

	animate_bmp(bmp, p, s, space);
}
	


void SpaceSprite::draw(Vector2 pos, Vector2 size, int index, BITMAP *surface) {
	STACKTRACE
	if (index >= count) {tw_error("SpaceSprite::draw - index %d > count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	int ix, iy, iw, ih;
	int mip = find_mip_level(size.x / this->w, highest_mip);
	BITMAP *bmp = b[mip][index];
	aa_set_mode(find_aa_mode(general_attributes));
	if (tw_aa_mode & AA_NO_ALIGN) {
		aa_stretch_blit(bmp, surface, 0,0,bmp->w,bmp->h, pos.x, pos.y, size.x, size.y);
	}
	else {
		ix = iround(pos.x);
		iy = iround(pos.y);
		iw = iround_up(size.x);
		ih = iround_up(size.y);
		aa_stretch_blit(bmp, surface, 0, 0, bmp->w, bmp->h, 
					ix, iy, iw, ih);
	}
	return;
}

#include "mgame.h"

void SpaceSprite::draw(Vector2 pos, Vector2 size, int index, Frame *frame) {
	STACKTRACE
	if (index >= count) {tw_error("SpaceSprite::draw - index %d > count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	if (pos.x + size.x < 0) return;
	if (pos.x >= frame->surface->w) return;
	if (pos.y + size.y < 0) return;
	if (pos.y >= frame->surface->h) return;

	int ix, iy, iw, ih;
	//if (ix >= frame->frame->w) return;
	//if (iy >= frame->frame->h) return;
	
	int mip = find_mip_level(size.x / this->w, highest_mip);
	BITMAP *bmp = 0;
	if (use_shademaps)
	{
		int mip = 0;
		
		Light light[2];
		
		light[0].angle = 0.0;//random(PI2);//PI2 * frame_time * 1E-3;
		light[0].intensity = 0;
		
		light[1].angle = (game->game_time/10000.0) * PI2;
		light[1].intensity = 200;
		//	message.print(1500, 15, "angle[%i]", int(light[1].angle * 180 / PI));
		
		double ambient = 0.0;
		
		BITMAP *tmp = add_shades(b[mip][0], ambient, light, 2, index * PI2 / 64.0);
		bmp = bmp_rot(tmp, index*PI2/64.0 );//b[mip][index];
		destroy_bitmap(tmp);
	} else
		bmp = b[mip][index];
	if (!bmp) return;

	aa_set_mode(find_aa_mode(general_attributes));
	if (tw_aa_mode & AA_NO_ALIGN) {
		ix = iround_down(pos.x);
		iy = iround_down(pos.y);
		iw = iround_up(pos.x+size.x) + 1 - ix;
		ih = iround_up(pos.y+size.y) + 1 - iy;
		aa_stretch_blit(bmp, frame->surface, 0,0,bmp->w,bmp->h, 
			pos.x, pos.y, size.x, size.y);
	}
	else {
		ix = iround(pos.x);
		iy = iround(pos.y);
		iw = iround(size.x);
		ih = iround(size.y);
		aa_stretch_blit(bmp, frame->surface, 0,0,bmp->w,bmp->h, 
			ix, iy, iw, ih);
	}
	frame->add_box(ix, iy, iw, ih);

	if (use_shademaps)
		destroy_bitmap(bmp);

	return;
}

void SpaceSprite::draw(int x, int y, int index, BITMAP *surface) {
	STACKTRACE
	draw(Vector2(x,y), Vector2(b[0][index]->w, b[0][index]->h), index, surface);
	return;
}

void SpaceSprite::animate(Vector2 pos, int index, Frame *space, double scale) 
{STACKTRACE
	if (index >= count) {
		tw_error("SpaceSprite::animate - index %d >= count %d", index, count);
		return;
		}
	if (index < 0) {
		tw_error("SpaceSprite::animate - index %d < 0 (count %d)", index, count);
		return;
	}
	Vector2 s = size(index) * scale;
	draw(corner(pos, s ), s * space_zoom, index, space);
	return;
}

void SpaceSprite::draw_character(int x, int y, int index, int color, BITMAP *bmp) 
{STACKTRACE
	if (index >= count) {tw_error("SpaceSprite::draw_character - index %d >= count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	draw_allegro_pmask ( m[index], bmp, x, y, color );
	return;
}

void SpaceSprite::draw_character(int x, int y, int index, int color, Frame *space) 
{STACKTRACE
	draw_character(x, y, index, color, space->surface);
	space->add_box(x, y, w, h);
}

void SpaceSprite::draw_character(int x, int y, int w, int h, int index, int color, Frame *space) 
{STACKTRACE
	draw_character(x, y, w, h, index, color, space->surface);
	space->add_box(x, y, w, h);
}

void SpaceSprite::draw_character(int x, int y, int w, int h, int index, int color, BITMAP *bmp) 
{STACKTRACE
	if (index >= count) {tw_error("SpaceSprite::draw_character_stretch - index %d >= count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	draw_allegro_pmask_stretch(m[index], bmp, x, y, w, h, color) ;
	return;
}

int SpaceSprite::collide(int x, int y, int i, int ox, int oy, int oi,
		SpaceSprite *other)
{
	STACKTRACE;
	if (i >= count) { tw_error("SpaceSprite::collide - index1 %d >= count1 %d", i, count); }
	if (oi >= other->count) { tw_error("SpaceSprite::collide - index2 %d >= count2 %d", oi, other->count); }
	if (i < 0) { tw_error("SpaceSprite::collide - index1 %d < count1 %d", i, count); }
	if (oi < 0) { tw_error("SpaceSprite::collide - index2 %d < count2 %d", oi, other->count); }
	return (check_pmask_collision(m[i], other->m[oi], x, y, ox, oy));
}

int line_collide;
int rect_x;
int rect_y;
int rect_w;
int rect_h;
int collide_x;
int collide_y;

PMASK *rect_mask;

void check_line_collision(BITMAP *bmp, int x, int y, int d)
{
	STACKTRACE;
	int row, col;

	if(line_collide)
		return;

	col = x - rect_x;
	row = y - rect_y;

	if((col >= 0) && (col < rect_w) &&
		(row >= 0) && (row < rect_h)) {
		if(get_pmask_pixel(rect_mask, col, row)) {
			line_collide = TRUE;
			collide_x = x;
			collide_y = y;
		}
	}
}

int SpaceSprite::collide_ray(int lx1, int ly1, int *lx2, int *ly2,
  int sx, int sy, int sindex)
{STACKTRACE
	line_collide = FALSE;
	rect_x = sx - (w / 2);
	rect_y = sy - (h / 2);
	rect_w = w;
	rect_h = h;
	if ((rect_x   > lx1) && (rect_x   > *lx2)) return FALSE;
	if ((rect_x+w < lx1) && (rect_x+w < *lx2)) return FALSE;
	if ((rect_y   > ly1) && (rect_y   > *ly2)) return FALSE;
	if ((rect_y+h < ly1) && (rect_y+h < *ly2)) return FALSE;
	rect_mask = m[sindex];
	do_line(NULL, lx1, ly1, *lx2, *ly2, 0, check_line_collision);
	if(line_collide) {
		*lx2 = collide_x;
		*ly2 = collide_y;
		return(TRUE);
	}

	return(FALSE);
}





BITMAP *create_video_bmp(int w, int h)
{
	BITMAP *bmp;
	bmp = create_video_bitmap(w, h);

	if (!bmp)
	{
		bmp = create_bitmap(w, h);
	}

	return bmp;
}

void destroy_video_bmp(BITMAP *bmp)
{
	if (!bmp)
		return;

	// allegro function
	destroy_bitmap(bmp);
}

void destroy_bmp(BITMAP **bmp)
{
	if (!*bmp)
		return;

	if (is_video_bitmap(*bmp))
		destroy_video_bmp(*bmp);
	else
		// allegro function
		destroy_bitmap(*bmp);
	*bmp = 0;
}


void destroy_rle(RLE_SPRITE **bmp)
{
	if (!*bmp)
		return;

	// allegro function
	destroy_rle_sprite(*bmp);
	*bmp = 0;
}


void destroy_sprite(SpaceSprite **sprite)
{
	if (!*sprite)
		return;

	delete *sprite;

	*sprite = 0;
}
