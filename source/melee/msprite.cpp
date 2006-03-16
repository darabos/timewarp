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

	if (strstr(s, "+masked")) {
		a |= SpaceSprite::MASKED;
	}
	if (strstr(s, "-masked")) {
		a &=~SpaceSprite::MASKED;
	}

	if (strstr(s, "+alpha")) {
		a |= SpaceSprite::ALPHA;
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
			// better: alpha is NOT supported for 16-bit color depth.
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
	//if ((obpp == bpp)) || !(aa_mode & AA_MASKED)) {	//xxx Geo: they are the same size, hence, color-depth conversion is best done by a BLIT.
	{
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

	int bpp = bitmap_color_depth(bmp);
	if (bpp != 16 && bpp != 32 && bpp != 24)
	{
		tw_error("invalid target bitmap depth in color-correct");
	}

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
			switch (bpp) {
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


void SpaceSprite::change_color_depth(int newbpp) {STACKTRACE
	int i, l;
	for (l = 0; l <= highest_mip; l += 1) {
		for (i = 0; i < count; i += 1) {
			if (sbitmap[l][i])
			{
				BITMAP *tmp = create_bitmap_ex(newbpp, width(i), height(i));

				convert_bitmap(sbitmap[l][i], tmp, (general_attributes & MASKED) ? AA_MASKED : 0);
				
//				if (attributes[i] & DEALLOCATE_IMAGE)
//					destroy_bitmap(sbitmap[l][i]);

//				attributes[i] |= DEALLOCATE_IMAGE;
				sbitmap[l][i] = tmp;
			}
		}
	}
	return;
}


/*
void SpaceSprite::permanent_phase_shift ( int phase ) {STACKTRACE
	int i, mip;
	Surface **tmp = new Surface*[count];
	while (phase < 0) phase += count;
	for (mip = 0; mip <= highest_mip; mip += 1) {
		for (i = 0; i < count; i += 1) {
			tmp[i] = sbitmap[mip][(i + phase) % count];
		}
		for (i = 0; i < count; i += 1) {
			sbitmap[mip][i] = tmp[i];
		}
	}
	delete[] tmp;

	PMASK **tmp2 = new PMASK*[count];
	while (phase < 0) phase += count;
	for (mip = 0; mip <= highest_mip; mip += 1) {
		for (i = 0; i < count; i += 1) {
			tmp2[i] = smask[(i + phase) % count];	// you don't have to generate, only shift
		}
		for (i = 0; i < count; i += 1) {
			smask[i] = tmp2[i];
		}
	}
	delete[] tmp2;
	return;
}
*/


Vector2 SpaceSprite::size(int i)
{

	BITMAP *b;
	b = get_bitmap(i);
	return Vector2(b->w, b->h);
}


int	SpaceSprite::width(int i)
{
	BITMAP *b;
	b = get_bitmap(i);
	return b->w;
}

int	SpaceSprite::height(int i)
{
	BITMAP *b;
	b = get_bitmap(i);
	return b->h;
}

void SpaceSprite::size(int *w, int *h, int i)
{
	BITMAP *b;
	b = get_bitmap(i);
	*w = b->w;
	*h = b->h;
}


PMASK *SpaceSprite::get_pmask(int index)
{
	if (!smask[index])
		smask[index] = create_allegro_pmask(get_bitmap(index));

	return smask[index];
}




// note. This creates a sprite from a data file. Make sure you copy all information, since the data-
// file will be deleted.
SpaceSprite::SpaceSprite(const DATAFILE *images, int sprite_count, int _attributes, int rotations)
{
	STACKTRACE
	int i, j;
	BITMAP *bmp;//, *tmp = 0;

	if (_attributes == -1) _attributes = string_to_sprite_attributes(NULL);

	count_base = sprite_count;		// real different images
	count_rotations = rotations;	// derived rotations from each image.
	count = sprite_count * rotations;
	if ((rotations < 1) || (count < 1)) {tw_error("SpaceSprite::SpaceSprite - bad parameters");}

	references = 0;
	highest_mip = 0;
	for (i = 1; i < MAX_MIP_LEVELS; i += 1) {
		sbitmap[i] = 0;
	}

	general_attributes = _attributes;
	bpp = videosystem.bpp;

	general_attributes |= SpaceSprite::NO_AA;
	general_attributes |= SpaceSprite::ALPHA;
//	general_attributes |= SpaceSprite::DITHER;

	// this is moved lower...
//	if (obpp != bpp)
//		tmp = create_bitmap_ex(obpp, bw, bh);

	if (general_attributes & ALPHA) {
		if (bpp <= 16) bpp = 16;
		else bpp = 32;
	}


	smask = new PMASK*[count];
	sbitmap[0] = new BITMAP*    [count];
//	attributes  = new char [count];

	for(i = 0; i < sprite_count; i += 1)
	{
		// error intercept? perhaps if memory is overwritten or so?
		if (bpp != 16 && bpp != 32 && bpp != 24)
		{
			tw_error("invalid target bitmap depth");
		}

		// determine the size of this bitmap
		int bw = 0, bh = 0;
		int obpp = 0;
		switch (images[i].type)
		{
			case DAT_RLE_SPRITE:
				bw = ((RLE_SPRITE *)(images[i].dat))->w;
				bh = ((RLE_SPRITE *)(images[i].dat))->h;
				obpp = ((RLE_SPRITE *)images[i].dat)->color_depth;
			break;
			case DAT_BITMAP:
				bw = ((BITMAP *)(images[i].dat))->w;
				bh = ((BITMAP *)(images[i].dat))->h;
				obpp = bitmap_color_depth((BITMAP *)images[i].dat);
			break;
		}

		// allocate space for storage of a copy of the bitmap
		// use the new color depth
		bmp = create_bitmap_ex(bpp, bw, bh);

		
		if (general_attributes & MASKED)
			clear_to_color(bmp, bitmap_mask_color(bmp));


		switch (images[i].type)
		{
			case DAT_RLE_SPRITE:
				{
				// use the old color depth
				BITMAP *tmp = create_bitmap_ex(obpp, bw, bh);

				int col = bitmap_mask_color(tmp);
				clear_to_color(tmp, col);

				draw_rle_sprite(tmp, (RLE_SPRITE *)(images[i].dat), 0, 0);
				

				// use the new color depth.
				blit(tmp, bmp, 0, 0, 0, 0, bw, bh);

				destroy_bitmap(tmp);
				}
			break;
			case DAT_BITMAP: {

				/*
				if (obpp != bpp)
				{
					convert_bitmap((BITMAP *)(images[i].dat), bmp, (general_attributes & MASKED) ? AA_MASKED : 0);
				} else {
					if (general_attributes & MASKED)
					{
						int col = bitmap_mask_color(bmp);
						clear_to_color(bmp, col);
						draw_sprite(bmp, (BITMAP*)images[i].dat, 0, 0);
					
					} else
						blit((BITMAP*)images[i].dat, bmp, 0, 0, 0, 0, bw, bh);
					}
					}
					*/

				// this copies, and also converts color depth.
				blit((BITMAP*)images[i].dat, bmp, 0, 0, 0, 0, bw, bh);
				}
			break;
		}

		if (general_attributes & ALPHA)
			handle_alpha_load(bmp);

		//xxx test
		//color_correct_bitmap(bmp, general_attributes & MASKED);

//		if (tmp == bmp)
//			tmp = NULL;

		for (j = 1; j < rotations; j += 1)
		{
			/* moved to the get_bitmap routine...
			BITMAP *tmp = create_bitmap_ex(bpp, w, h);
			clear_to_color(tmp, bitmap_mask_color(tmp));
			rotate_sprite(tmp, bmp, 0, 0, j * ((1<<24)/rotations));
			*/

			int index = j + (i * rotations);
			smask[index] = 0;
//			m[j + (i * rotations)] = create_allegro_pmask(tmp);
			
			sbitmap[0][index] = 0;//tmp;
//			attributes[j + (i * rotations)] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
			}
		int index = i * rotations;
		smask[index] = 0;
//		m[(i * rotations)] = create_allegro_pmask(bmp);
		sbitmap[0][index] = bmp;
//		attributes[index] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
		}

//	if (tmp)
//	{
//		destroy_bitmap(tmp);
//		tmp = 0;
//	}




	if (!sbitmap[0][0])
	{
		tw_error("Basic sprite shape expected, but doesn't exist");
	}

}

SpaceSprite::SpaceSprite(SpaceSprite &old) {
	STACKTRACE
	int i, l;
	BITMAP *bmp;
	count = old.count;
	bpp = old.bpp;
	highest_mip = old.highest_mip;
	originaltype = -1;
//	w = old.w;
//	h = old.h;
	smask = new PMASK*[count];
	sbitmap[0] = new BITMAP*    [count];

	references = 0;
//	attributes  = new char [count];
	general_attributes = old.general_attributes;

	count_base = old.count_base;
	count_rotations = old.count_rotations;

	for(i = 0; i < count; i++)
	{
		if (old.sbitmap[0][i])
		{
			bmp = create_bitmap(old.sbitmap[0][i]->w, old.sbitmap[0][i]->h);
			blit(old.sbitmap[0][i], bmp, 0, 0, 0, 0, old.sbitmap[0][i]->w, old.sbitmap[0][i]->h);
			sbitmap[0][i] = bmp;
		} else {
			sbitmap[0][i] = 0;
		}

		smask[i] = 0;//create_allegro_pmask(bmp);
//		attributes[i] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
		}
	for (l = 1; l < MAX_MIP_LEVELS; l += 1)
	{
		if (old.sbitmap[l])
		{
			sbitmap[l] = new BITMAP* [count];
			
			for(i = 0; i < count; i++)
			{
				if (old.sbitmap[l][i])
				{
					bmp = create_bitmap(old.sbitmap[l][i]->w, old.sbitmap[l][i]->h);
					blit(old.sbitmap[l][i], bmp, 0, 0, 0, 0, old.sbitmap[l][i]->w, old.sbitmap[l][i]->h);
					sbitmap[l][i] = bmp;
				} else {
					sbitmap[l][i] = 0;
				}
			}
		} else {
			sbitmap[l] = 0;
		}
	}

	if (!sbitmap[0][0])
	{
		tw_error("Basic sprite shape expected, but doesn't exist");
	}
}

void SpaceSprite::lock() {
	STACKTRACE
	highest_mip = 0;
	return;
}

void SpaceSprite::unlock()
{
	STACKTRACE
	int i, j;
	for (i = 0; i < MAX_MIP_LEVELS; i += 1) {
		if (sbitmap[i])
			j = i;
	}
	return;
}


SpaceSprite::~SpaceSprite() {
	STACKTRACE
	int i, l;

	for(i = 0; i < count; i++) {
		//xxx why is this conditional ?? It never borrows, that's too messy...
		//if (attributes[i] & DEALLOCATE_MASK)
		if (smask[i])
			destroy_pmask(smask[i]);
		}

	if (smask)
		delete[] smask;
	smask = 0;

	for (l = 0; l <= highest_mip; l += 1)
	{
		if (sbitmap[l])
		{
			for(i = 0; i < count; i++) {
				//xxx why is this conditional ?? It never borrows, that's too messy...
				//if (attributes[i] & DEALLOCATE_IMAGE)
				if (sbitmap[l][i])
					destroy_bitmap(sbitmap[l][i]);
			}

			delete[] sbitmap[l];
			sbitmap[l] = 0;
		}
	}

//	delete[] attributes;
//	attributes = 0;
	return;
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
	int spr_w = iround(width(index) * space_zoom * scale);
	int spr_h = iround(height(index) * space_zoom * scale);
	if(spr_w < 1) spr_w = 1;
	if(spr_h < 1) spr_h = 1;
	draw_character(ix, iy, spr_w, spr_h, index, color, space);
	return;
}

void SpaceSprite::overlay (int index1, int index2, BITMAP *dest) {
	STACKTRACE
	int x, y;
	if (index1 > count) {tw_error("SpaceSprite::overlay - index1 %d > count %d", index1, count);}
	if (index2 > count) {tw_error("SpaceSprite::overlay - index2 %d > count %d", index2, count);}

	BITMAP *bmp1, *bmp2;
	bmp1 = get_bitmap(index1);
	bmp2 = get_bitmap(index2);
	if ((bmp1->w != bmp2->w) || (bmp1->h != bmp2->h))
	{
		tw_error("Overlay: trying to match bitmaps of different sizes.");
	}
	for(y = 0; y < bmp1->h; y += 1) {
		for(x = 0; x < bmp1->w; x += 1) {
			if (getpixel(bmp1, x, y) != getpixel(bmp2, x, y)) {
				putpixel(dest, x, y, getpixel(bmp2, x, y));
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
	int mip = find_mip_level(size.x / this->width(index), highest_mip);
	BITMAP *bmp = get_bitmap(index, mip);//b[mip][index];
	aa_set_mode(find_aa_mode(general_attributes));
	if (tw_aa_mode & AA_NO_ALIGN) {
		aa_stretch_blit(bmp, surface, 0,0,bmp->w,bmp->h, pos.x, pos.y, size.x, size.y);
	}
	else {
		ix = iround(pos.x);
		iy = iround(pos.y);
		iw = iround_up(size.x);
		ih = iround_up(size.y);
		if (tw_aa_mode & AA_NO_AA)
		{
			masked_stretch_blit(bmp, surface, 0, 0, bmp->w, bmp->h, 
						ix, iy, iw, ih);
		} else {
			aa_stretch_blit(bmp, surface, 0, 0, bmp->w, bmp->h, 
						ix, iy, iw, ih);
		}
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
	
	int mip = find_mip_level(size.x / this->width(index), highest_mip);

	BITMAP *bmp;

	bmp = get_bitmap(index, mip);//b[mip][index];
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
		if (tw_aa_mode & AA_NO_AA)
		{
			masked_stretch_blit(bmp, frame->surface, 0,0,bmp->w,bmp->h, ix, iy, iw, ih);

		} else {
			aa_stretch_blit(bmp, frame->surface, 0,0,bmp->w,bmp->h, 
				ix, iy, iw, ih);
		}
	}
	frame->add_box(ix, iy, iw, ih);

	return;
}

void SpaceSprite::draw(int x, int y, int index, BITMAP *surface) {
	STACKTRACE;
	BITMAP *bmp = get_bitmap(index);
	draw(Vector2(x,y), Vector2(bmp->w, bmp->h), index, surface);
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
	draw_allegro_pmask ( get_pmask(index), bmp, x, y, color );
	return;
}

void SpaceSprite::draw_character(int x, int y, int index, int color, Frame *space) 
{STACKTRACE
	draw_character(x, y, index, color, space->surface);
	space->add_box(x, y, width(index), height(index));
}

void SpaceSprite::draw_character(int x, int y, int w, int h, int index, int color, Frame *space) 
{
	STACKTRACE;
	draw_character(x, y, w, h, index, color, space->surface);
	space->add_box(x, y, w, h);
}

void SpaceSprite::draw_character(int x, int y, int w, int h, int index, int color, BITMAP *bmp) 
{STACKTRACE
	if (index >= count) {tw_error("SpaceSprite::draw_character_stretch - index %d >= count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	draw_allegro_pmask_stretch(get_pmask(index), bmp, x, y, w, h, color) ;
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
	return (check_pmask_collision(get_pmask(i), other->get_pmask(oi), x, y, ox, oy));
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
{
	STACKTRACE;

	BITMAP *bmp;
	bmp = get_bitmap(sindex);
	if (!bmp)
		return (FALSE);

	int w, h;
	w = bmp->w;
	h = bmp->h;

	line_collide = FALSE;
	rect_x = sx - (w / 2);
	rect_y = sy - (h / 2);
	rect_w = w;
	rect_h = h;
	if ((rect_x   > lx1) && (rect_x   > *lx2)) return FALSE;
	if ((rect_x+w < lx1) && (rect_x+w < *lx2)) return FALSE;
	if ((rect_y   > ly1) && (rect_y   > *ly2)) return FALSE;
	if ((rect_y+h < ly1) && (rect_y+h < *ly2)) return FALSE;
	rect_mask = get_pmask(sindex);
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





BITMAP *SpaceSprite::get_bitmap(int index, int miplevel)
{STACKTRACE
	if (miplevel != 0) 
	{
		tw_error ("get_bitmap on undefined mipmap level");
	}

	if (index >= count)
	{
		tw_error("SpaceSprite::get_bitmap - index %d > count %d", index, count);
	}

	if (index < 0)
	{
		tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count);
	}
	// changed ROB
	//highest_mip = 0;
	// changed ROB

	if (!sbitmap[miplevel][index])
	{
		if (!sbitmap[miplevel])
		{
			tw_error("The pointer array for the mipmapped bitmaps hasn't been defined for this level!");
		}

		// generate a derived image...
		if (!sbitmap[0][index])
		{
			// generate the unscaled image
			int irot = index % count_rotations;	// which rotation
			int ipic = index - irot;			// basic pic

			if (ipic < 0 || ipic >= count || irot < 0 || irot >= count_rotations)
			{
				tw_error("Accessing invalid base picture");
			}

			BITMAP *bmp = sbitmap[0][ipic];
			if (!bmp)
			{
				tw_error("Basic sprite shape doesn't exist, cannot rotate");
			}
			BITMAP *tmp = create_bitmap_ex(bpp, bmp->w, bmp->h);
			clear_to_color(tmp, bitmap_mask_color(tmp));
			rotate_sprite(tmp, bmp, 0, 0, irot * ((1<<24)/count_rotations));

			sbitmap[0][index] = tmp;

		}

	}

	return sbitmap[miplevel][index];
}






// all the sprite-relevant information (and nothing else).
SpaceSprite::SpaceSprite(BITMAP **bmplist, int sprite_count, int _attributes, int rotations)
{
	count_base = sprite_count;		// real different images
	count_rotations = rotations;	// derived rotations from each image.
	count = sprite_count * rotations;
	if ((rotations < 1) || (count < 1)) {tw_error("SpaceSprite::SpaceSprite - bad parameters");}

	int i, j, obpp;

	if (_attributes == -1) _attributes = string_to_sprite_attributes(NULL);

	references = 0;
	highest_mip = 0;
	for (i = 1; i < MAX_MIP_LEVELS; i += 1) {
		sbitmap[i] = 0;
	}

	general_attributes = _attributes;
	bpp = videosystem.bpp;


	if (general_attributes & ALPHA) {
		if (bpp <= 16) bpp = 16;
		else bpp = 32;
	}


	smask = new PMASK*[count];
	sbitmap[0] = new BITMAP*    [count];
//	attributes  = new char [count];

	for ( i = 0; i < sprite_count; ++i )
	{

		BITMAP *bmp = NULL;
		bmp = create_bitmap_ex(bpp, bmplist[i]->w, bmplist[i]->h);

		if (general_attributes & MASKED)
			clear_to_color(bmp, bitmap_mask_color(bmp));

		obpp = bitmap_color_depth(bmplist[i]);

		/*
		if (obpp != bpp)
		{
			convert_bitmap(bmplist[i], bmp, (general_attributes & MASKED) ? AA_MASKED : 0);

		} else {

			if (general_attributes & MASKED)
				draw_sprite(bmp, bmplist[i], 0, 0);
			else
				blit(bmplist[i], bmp, 0, 0, 0, 0, bmp->w, bmp->h);
		}*/
		// this will copy, and convert if necessary.
		blit(bmplist[i], bmp, 0, 0, 0, 0, bmp->w, bmp->h);

		//color_correct_bitmap(bmp, general_attributes & MASKED);

		if (general_attributes & ALPHA)
			handle_alpha_load(bmp);


		for (j = 1; j < rotations; j += 1)
		{

			int index = j + (i * rotations);
			smask[index] = 0;
			
			sbitmap[0][index] = 0;//tmp;
//			attributes[index] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
		}

		int index = i * rotations;
		smask[index] = 0;

		sbitmap[0][index] = bmp;
//		attributes[index] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;

	}


	if (!sbitmap[0][0])
	{
		tw_error("Basic sprite shape expected, but doesn't exist");
	}
}






BITMAP* copybmp( BITMAP* src )
{
	STACKTRACE;

	BITMAP *dest;

	// copied from vanguard
	dest = create_bitmap_ex(bitmap_color_depth(screen), src->w, src->h);	//Create a new bitmap
	if(!dest) return NULL;							//If failed, return NULL

	blit(src, dest, 0, 0, 0, 0, src->w, src->h);	//Copy bitmap from datafile

	return dest;
}


