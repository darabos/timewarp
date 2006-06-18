/*
This file is part of project TWX, homepage http://twx.sourceforge.net/

This program is distributed under the GNU General Public License as
published by the Free Software Foundation.
*/
#include <allegro.h>
#include "allegro/internal/aintern.h"

#include <ft2build.h> //required by freetype2 headers
//#include FT_FREETYPE_H //required by freetype2 headers
#include "freetype/freetype.h"

#include "ttf.h"

//#include "../math/round.h"

#include "../scp.h"

// copy ttf bitmap onto an allegro bitmap
static void my_draw_bitmap( FT_Bitmap *src, BITMAP *dest, int dx, int dy )
{
	if (src->rows == 0 || src->width == 0)
		return;

	int ix, iy;

	if (src->pixel_mode == ft_pixel_mode_mono)
	{

		// dunno if this works - untested
		int ix, iy;
		for ( iy = 0; iy < src->rows; ++iy )
		{
			unsigned char *p = src->buffer + iy * src->pitch;
			
			for ( ix = 0; ix < src->width; ++ix )
			{
				unsigned char v;
				unsigned char mask;

				//v = p[ix >> 8];
				//mask = 1 << (ix & 0x0F);

				v = p[ix/8];
				mask = 1 << (7 + 8*(ix/8) - ix);
				
				if (v & mask)
					putpixel(dest, ix+dx, iy+dy, 255);
			}
		}
		
	} else if ( src->pixel_mode == ft_pixel_mode_grays )
	{
		for ( iy = 0; iy < src->rows; ++iy )
		{
			unsigned char *p = src->buffer + iy * src->pitch;
			
			for ( ix = 0; ix < src->width; ++ix )
			{
				unsigned char v;
				v = p[ix];
				
				//if (v != 0)
				//if (v > 64)	// discard too transparent pixels (disregard anti-aliasing...)
				putpixel(dest, ix+dx, iy+dy, v);
			}
		}
		
	} else {
		return;//tw_error("unsupported bitmap format");
	}


}


static void cleanup (FT_Library engine, FT_Face face)
{
	if (engine)
		FT_Done_FreeType(engine);
}






// for debugging (copied from allegro)
void color_destroy_font(FONT* f)
{
    FONT_COLOR_DATA* cf;

    if(!f) return;

    cf = (FONT_COLOR_DATA*)(f->data);

    while(cf) {
        FONT_COLOR_DATA* next = cf->next;
        int i = 0;

        for(i = cf->begin; i < cf->end; i++)
			destroy_bmp(&cf->bitmaps[i - cf->begin]);

        free(cf->bitmaps);
        free(cf);

        cf = next;
    }

	delete f->vtable;

    free(f);
}





FONT* load_ttf_font (const char* filename, const int points, const int smooth,
					 double linespacing)
{

	int points_w, points_h, begin, end;
	begin = 32;	// space character
	end = 128;	// ?
	points_w = points;
	points_h = points;


	FT_Library           engine = 0;
	FT_Face              face = 0;
	
	
	FT_Error error;

	// initialize the library
	error = FT_Init_FreeType(&engine);
	if(error)
		return 0;
		
	// load a font
	error = FT_New_Face(engine, filename, 0, &face);
	if(error)
	{
		cleanup(engine, face);
		return 0;
	}

	// set font size in pixels
	error = FT_Set_Pixel_Sizes(face, points_w, points_h);
	if(error)
	{
		cleanup(engine, face);
		return 0;
	}


	// in case there's no unicode (ascii) charmap loaded by default
	if (!face->charmap)
	{
		if (!face->num_charmaps)	// there are no charmaps !
		{
			cleanup(engine, face);
			return 0;
		}

		error = FT_Set_Charmap(face, face->charmaps[0]);	// just pick the first one .
		
		if (error)	// shouldn't occur
		{
			cleanup(engine, face);
			return 0;
		}
	}

	
	
	int c;
	
	// NOTE: static void color_destroy(FONT* f)  destroys bitmaps >=begin, <end
	
	
	// Allocate and setup the Allegro font
	// (copied from allegttf)
	struct FONT_COLOR_DATA *fcd;
	fcd = (struct FONT_COLOR_DATA*) calloc(1,sizeof(struct FONT_COLOR_DATA));
	fcd->begin = begin;
	fcd->end = end;
	int num = end - begin;
	fcd->bitmaps = (BITMAP**) calloc(num,sizeof(BITMAP*));
	fcd->next = NULL;

	struct FONT *f;
	f = (struct FONT*) calloc(1,sizeof(struct FONT));
	f->data = (void*)fcd;

	f->vtable = new FONT_VTABLE;
	*f->vtable = *font_vtable_color;
	f->vtable->render = color_render_props;
	
	//f->vtable->destroy = &color_destroy_font;
	// STRANGE, this is needed to avoid a crash ?!?!
	// WHY ???
	


	FT_GlyphSlot  slot = face->glyph;  // a small shortcut
	int           pen_x, pen_y;
	
	pen_x = 0;
	pen_y = 0;

	int ymin, ymax, yorigin;
	
	ymin = face->descender;		// the bounding box for all chars in this font.
	ymax = face->ascender;		// this actually defines the origin position...
	//yorigin = iround(points_h * double(ymax) / double(ymax - ymin));
	// measured from the top.

	// the "real" height is this ... each char would fit in a box of the pixel
	// size, but there's also the line spacing, and some vertical offset to take
	// into account. This creates a higher vertical value...
	double hmax = ymax - ymin;
	yorigin = points_h;			// so that a capital aligns at the top?
	
	//points_h = iround(points_h * double(hmax + face->height) / hmax);
	// well ... the default line spacing in the ttf files isn't that great. Better
	// define you own spacing.

	// best thing is ... include some spacing:
	points_h = iround(points_h * linespacing);

	// override, test this:
	yorigin = points_h * ymax/hmax;			// so that a capital aligns at the top?
	
	//char c;
	for ( c = begin; c < end; ++c )		// not <= end !!
	{
		// load glyph image into the slot (erase previous one)
		
		error = FT_Load_Char( face, c, FT_LOAD_RENDER);// | FT_LOAD_TARGET_MONO | FT_LOAD_NO_AUTOHINT );
		if (error)
			continue;
		
		// now, draw to our target surface
		int w, h;

		w = slot->metrics.horiAdvance / 64;
		if (!w)
			w = 1;

		h = points_h;
		
		int dx, dy;

		/* Note that bitmap_left is the horizontal distance from the
		current pen position to the leftmost border of the glyph bitmap,
		while bitmap_top is the vertical distance from the pen position
		(on the baseline) to the topmost border of the glyph bitmap.
		It is positive to indicate an upwards distance. */
		dx = slot->bitmap_left;
		dy = yorigin - slot->bitmap_top;
		//dy = 0;
		//dy = points_h - slot->bitmap_top;

		BITMAP *bmp = 0;
		if (w && h)
		{
			bmp = create_bitmap_ex(8, w, h);
			clear_to_color(bmp, 0);
			
			my_draw_bitmap( &slot->bitmap, bmp, dx, dy );
		}

		fcd->bitmaps[c-begin] = bmp;
	}

	
	// set the font height
	f->height = points_h;
	
	// clean up the font stuff
	cleanup(engine, face);
	
	return f;
}



static int target_red   = 0;
static int target_green = 0;
static int target_blue  = 0;

void set_text_color(int red, int green, int blue)
{
	target_red = red;
	target_green = green;
	target_blue = blue;
}


void draw_transparent(BITMAP *dest, BITMAP *src, int x, int y)
{
	if (bitmap_color_depth(dest) != 32)
		return;

	if (bitmap_color_depth(src) != 8)
		return;

	// if the char. is offscreen, don't draw it.
	if (x >= dest->w || y >= dest->h || x+src->w < 0 || y+src->h < 0)
		return;

	int k = 0;
	int ix, iy;
	for ( iy = 0; iy < src->h; ++iy )
	{
		for ( ix = 0; ix < src->w; ++ix )
		{
			int v;
			v = getpixel(src, ix, iy);

			if (v)
			{
				// this is the transparency value
				int v_inv;
				v_inv = 256 - v;

				// target color:
//				int target_red, target_green, target_blue;
//				target_red = 200;
//				target_green = 100;
//				target_blue = 50;

				int w, destx, desty;
				destx = x + ix;
				desty = y + iy;

				w = getpixel(dest, destx, desty);

				int wr, wg, wb;
				wr = getr32(w);
				wg = getg32(w);
				wb = getb32(w);

				int cr, cg, cb;
				cr = ((v_inv * wr) + (v * target_red  )) >> 8;
				cg = ((v_inv * wg) + (v * target_green)) >> 8;
				cb = ((v_inv * wb) + (v * target_blue )) >> 8;

				putpixel(dest, x+ix, y+iy, makecol32(cr, cg, cb));
			}

			++k;
		}
	}
}

/* color_render_char:
 *  (color vtable entry)
 *  Renders a color character onto a bitmap, at the specified location, using
 *  the specified colors. If fg == -1, render as color, else render as
 *  mono; if bg == -1, render as transparent, else render as opaque.
 *  Returns the character width, in pixels.
 */

static int color_render_char_props(AL_CONST FONT* f, int ch, int fg, int bg, BITMAP* bmp, int x, int y, int property)
{
    int w = 0;
    BITMAP *g = 0;

    int old_textmode = _textmode;
    _textmode = bg;

    acquire_bitmap(bmp);

    if (property & 1)
	{
		rectfill(bmp, x, y, x + f->vtable->char_length(f, ch) - 1, y + f->vtable->font_height(f) - 1, bg);
    }

    g = _color_find_glyph(f, ch);
    if(g) {

	if (!property & 1)
	{
	    //bmp->vtable->draw_256_sprite(bmp, g, x, y);
		draw_transparent(bmp, g, x, y);
	} else {
		int bg = -1;
	    bmp->vtable->draw_character(bmp, g, x, y, fg, bg);
	}

	w = g->w;
    }

    release_bitmap(bmp);
    _textmode = old_textmode;

    return w;
}




static char *color_props = 0;
// properties (one for each character):
// 0x01: bit unset: transparent, bit set:with black background
char color_props_default[256];

void reset_color_props()
{
	int i;
	for ( i = 0; i < 256; ++i )
		color_props_default[i] = 0;
	color_props = color_props_default;
}

void set_color_props(char *newprops)
{
	color_props = newprops;
}


/* color_render:
 *  (color vtable entry)
 *  Renders a color font onto a bitmap, at the specified location, using
 *  the specified colors. If fg == -1, render as color, else render as
 *  mono; if bg == -1, render as transparent, else render as opaque.
 */
void color_render_props(AL_CONST FONT* f, AL_CONST char* text, int fg, int bg, BITMAP* bmp, int x, int y)
{
	if (!color_props)
	{
		font_vtable_color->render(f, text, fg, bg, bmp, x, y);
		return;
	}

	int i;

    AL_CONST char* p = text;
    int ch = 0;

    int old_textmode = _textmode;
    _textmode = bg;

    acquire_bitmap(bmp);

    if(fg < 0 && bg >= 0) {
	rectfill(bmp, x, y, x + text_length(f, text) - 1, y + text_height(f) - 1, bg);
	bg = -1; /* to avoid filling rectangles for each character */
    }

	i = 0;
    while( (ch = ugetxc(&p)) ) {
        x += color_render_char_props(f, ch, fg, bg, bmp, x, y, color_props[i]);
		++i;
    }

    release_bitmap(bmp);
    _textmode = old_textmode;
}
