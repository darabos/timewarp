
#include <string.h>
#include <stdio.h>

#include "../../melee.h"

REGISTER_FILE

#include "../../util/aastr.h"
#include "../../util/pmask.h"

#include "sprites.h"


void scale_bitmap32(BITMAP **bmp, double a)
{
	int w, h;

	w = int((*bmp)->w * a + 0.5);
	h = int((*bmp)->h * a + 0.5);

	if (w == 0) w = 1;
	if (h == 0) h = 1;

	BITMAP *tmp;
	tmp = create_bitmap_ex(32, w, h);

	stretch_blit(*bmp, tmp, 0, 0, (*bmp)->w, (*bmp)->h, 0, 0, w, h);

	destroy_bitmap(*bmp);
	*bmp = tmp;
}



void load_bitmap32(BITMAP **bmp, char *fname)
{
	*bmp = load_bitmap(fname, 0);

	// map_bmp should have 32 bit depth, because colors are assumed to come from
	// a 32 bpp depth image. Loading from disk is sometimes (or often) 24 bit.
	BITMAP *tmp;
	tmp = create_bitmap_ex(32, (*bmp)->w, (*bmp)->h);
	clear_to_color(tmp, makecol32(255,0,255));
	blit(*bmp, tmp, 0, 0, 0, 0, tmp->w, tmp->h);
	destroy_bitmap(*bmp);
	*bmp = tmp;
}





void balance(double *r, double *g, double *b)
{
	double max;
	max = *r;
	if (*g > max)
		max = *g;
	if (*b > max)
		max = *b;

	*r /= max;
	*g /= max;
	*b /= max;
}


void avcolor(BITMAP *bmp, double *r, double *g, double *b)
{
	*r = 0;
	*g = 0;
	*b = 0;
	int N = 0;

	int ix, iy;
	
	for ( iy = 0; iy < bmp->h; ++iy)
	{
		for ( ix = 0; ix < bmp->w; ++ix)
		{

			int col, rr, gg, bb;

			col = getpixel(bmp, ix, iy);
			rr = getr32(col);
			gg = getg32(col);
			bb = getb32(col);

			if (rr != 255 || gg != 0 || bb != 255)
			{
				*r += rr;
				*g += gg;
				*b += bb;
				++N;
			}

		}
	}

	*r /= (255 * N);
	*g /= (255 * N);
	*b /= (255 * N);
}



void colorize(SpaceSprite *spr, double mr, double mg, double mb)
{
	colorize(spr->get_bitmap(0), mr, mg, mb);
}

void colorize(BITMAP *bmp, double mr, double mg, double mb)
{

	int ix, iy;
	
	for ( iy = 0; iy < bmp->h; ++iy)
	{
		for ( ix = 0; ix < bmp->w; ++ix)
		{

			int col, r, g, b;

			col = getpixel(bmp, ix, iy);
			r = getr32(col);
			g = getg32(col);
			b = getb32(col);

			if (!(r == 255 && g == 0 && b == 255))
			{
				r *= mr;
				g *= mg;
				b *= mb;

				if (r > 255)	r = 255;
				if (g > 255)	g = 255;
				if (b > 255)	b = 255;
				
				col = makecol32(r,g,b);
				putpixel(bmp, ix, iy, col);
			}
		}
	}

}


void brighten(SpaceSprite *spr)
{
	brighten(spr->get_bitmap(0));
}

void brighten(BITMAP *bmp)
{

	int ix, iy;

	int max = 0;
	
	for ( iy = 0; iy < bmp->h; ++iy)
	{
		for ( ix = 0; ix < bmp->w; ++ix)
		{

			int col, r, g, b;

			col = getpixel(bmp, ix, iy);
			r = getr32(col);
			g = getg32(col);
			b = getb32(col);

			if (r > max)	max = r;
			if (g > max)	max = g;
			if (b > max)	max = b;
		}
	}

	double scale;
	scale = double(255) / double(max);

	colorize(bmp, scale, scale, scale);
}



void load_bitmaps(BITMAP **bmp, char *dirname, char *filename, int N)
{
	int i;
	for ( i = 0; i < N; ++i )
	{
		char txt[512];
		strcpy(txt, dirname);
		strcat(txt, "/");
		strcat(txt, filename);

		char num[128];
		sprintf(num, "_%02i.bmp", i+1);

		strcat(txt, num);
		
		bmp[i] = load_bitmap(txt, 0);
		if (!bmp[i])
		{
			tw_error("No such bitmap");
		}
		replace_color(bmp[i], 0, makecol_depth(bitmap_color_depth(bmp[i]), 255, 0, 255));
	}
}



void destroy_bitmaps(BITMAP **bmp, int N)
{
	int i;
	for ( i = 0; i < N; ++i )
	{
		destroy_bitmap(bmp[i]);
	}
}





void replace_color(BITMAP *bmp, int col1, int col2)
{
	int ix, iy;

	for ( iy = 0; iy < bmp->h; ++iy )
	{
		for ( ix = 0; ix < bmp->w; ++ix )
		{
			int col;
			col = getpixel(bmp, ix, iy);

			if (col == col1)	// replace this color
				putpixel(bmp, ix, iy, col2);
		}
	}
}



// all the sprite-relevant information (and nothing else).
SpaceSprite::SpaceSprite(BITMAP **bmplist, int sprite_count, int rotations, int _attributes)
{
	count = sprite_count * rotations;

	int i, j, obpp;
	BITMAP *bmp, *tmp = NULL;

	if (_attributes == -1) _attributes = string_to_sprite_attributes(NULL);

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
			bpp = 32;
		}
	}
	else bpp = 0;

	w = bmplist[0]->w;
	h = bmplist[0]->h;
	obpp = bitmap_color_depth(bmplist[0]);

	if (bpp == 0) bpp = obpp;
	if (obpp != bpp) tmp = create_bitmap_ex(obpp, w, h);
	if (general_attributes & ALPHA) {
		if (bpp <= 16) bpp = 16;
		else bpp = 32;
	}


	m = new PMASK*[count];
	b[0] = new BITMAP*    [count];
	attributes  = new char [count];

	for(i = 0; i < sprite_count; i += 1)
	{

		bmp = create_bitmap_ex(bpp, bmplist[i]->w, bmplist[i]->h);
		if (!tmp) tmp = bmp;
		if (general_attributes & MASKED)
			clear_to_color(bmp, bitmap_mask_color(bmp));

		if (tmp != bmp) clear_to_color(tmp, bitmap_mask_color(tmp));

//		if (general_attributes & ALPHA)
//			handle_alpha_load((BITMAP *)(images[i].dat));

		if (obpp != bpp)
		{
			convert_bitmap(bmplist[i], bmp, (general_attributes & MASKED) ? AA_MASKED : 0);
		} else {
			if (general_attributes & MASKED)
				draw_sprite(bmp, bmplist[i], 0, 0);

			else blit(bmplist[i], bmp, 0, 0, 0, 0, bmp->w, bmp->h);
		}

		color_correct_bitmap(bmp, general_attributes & MASKED);

//		make_alpha(bmp);

		if (tmp == bmp) tmp = NULL;

		for (j = 1; j < rotations; j += 1)
		{
			BITMAP *tmp = create_bitmap_ex(bpp, bmp->w, bmp->h);
			clear_to_color(tmp, bitmap_mask_color(tmp));
			rotate_sprite(tmp, bmp, 0, 0, j * ((1<<24)/rotations));

			// also replace the color ZERO with a transparancy color (why: I dislike bright pink)
			replace_color(tmp, 0, makecol_depth(bitmap_color_depth(tmp), 255, 0, 255));

			m[j + (i * rotations)] = create_allegro_pmask(tmp);
			b[0][j + (i * rotations)] = tmp;
			attributes[j + (i * rotations)] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;

		}

		// also replace the color ZERO with a transparancy color (why: I dislike bright pink)
		replace_color(bmp, 0, makecol_depth(bitmap_color_depth(bmp), 255, 0, 255));

		m[(i * rotations)] = create_allegro_pmask(bmp);
		b[0][(i * rotations)] = bmp;
		attributes[(i * rotations)] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;

	}

	if (general_attributes & MIPMAPED) {
		generate_mipmaps();
	}
}









