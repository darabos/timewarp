/* $Id$ */ 
#include <allegro.h>
#include "allegro/internal/aintern.h"

#include <ft2build.h> //required by freetype2 headers
#include FT_FREETYPE_H //required by freetype2 headers
#include "freetype/freetype.h"

#include "ttf.h"

#include "util/round.h"

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
				v = p[ix >> 8];
				
				unsigned char mask;
				mask = 1 << (ix & 0x0F);
				
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
				if (v > 128)	// discard too transparent pixels (disregard anti-aliasing...)
					putpixel(dest, ix+dx, iy+dy, 255);
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



FONT* load_ttf_font (const char* filename, const int points, const int smooth)
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
	
	AL_CONST int num = end - begin + 1;
	
	struct FONT *f;
	struct FONT_COLOR_DATA *fcd;
	
	// Allocate and setup the Allegro font
	// (copied from allegttf)
	f = (struct FONT*)calloc(1,sizeof(struct FONT));
	fcd = (struct FONT_COLOR_DATA*)calloc(1,sizeof(struct FONT_COLOR_DATA));
	fcd->begin = begin;
	fcd->end = end;
	fcd->bitmaps = (BITMAP**)calloc(num,sizeof(BITMAP*));
	fcd->next = NULL;
	f->data = (void*)fcd;
	f->vtable = font_vtable_color;
	


	FT_GlyphSlot  slot = face->glyph;  // a small shortcut
	int           pen_x, pen_y;
	
	pen_x = 0;
	pen_y = 0;

	int ymin, ymax, yorigin;
	
	ymin = face->bbox.yMin;		// the bounding box for all chars in this font.
	ymax = face->bbox.yMax;		// this actually defines the origin position...
	yorigin = iround(points_h * double(ymax) / double(ymax - ymin));
	
	//char c;
	for ( c = begin; c <= end; ++c )
	{
		// load glyph image into the slot (erase previous one)
		
		error = FT_Load_Char( face, c, FT_LOAD_RENDER );
		if (error)
			continue;
		
		// now, draw to our target surface
		BITMAP *bmp = 0;
		int w, h;

		w = slot->metrics.horiAdvance / 64;
		if (!w)
			w = 1;

		h = points_h;
		
		int dx, dy;

		dx = slot->bitmap_left;
		dy = yorigin - slot->bitmap_top;

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
   
   



