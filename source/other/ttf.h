/* $Id$ */ 

#ifndef __FONT_TTF__
#define __FONT_TTF__

FONT* load_ttf_font (const char* filename, const int points, const int smooth);
FONT* load_font_test (const int points);

void color_destroy_font(FONT* f);

#endif

