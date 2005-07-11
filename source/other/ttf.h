/*
This file is part of project TWX, homepage http://twx.sourceforge.net/

This program is distributed under the GNU General Public License as
published by the Free Software Foundation.
*/

/** @file fonts */
#ifndef __FONT_TTF__
#define __FONT_TTF__

/** set the drawing color */
void set_text_color(int red, int green, int blue);

/** load a true type font */
FONT* load_ttf_font (const char* filename, const int points, const int smooth, double linespacing);
FONT* load_font_test (const int points);

void color_destroy_font(FONT* f);


void color_render_props(AL_CONST FONT* f, AL_CONST char* text, int fg, int bg, BITMAP* bmp, int x, int y);
void reset_color_props();

/** properties for drawing: bit 1, unset=transparent, set=direct against background square */
void set_color_props(char *newprops);

#endif

