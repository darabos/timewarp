
#ifndef __FONT_MORPH__
#define __FONT_MORPH__

#include <allegro.h>
#include "allegro/internal/aintern.h"


FONT_COLOR_DATA* font_upgrade_to_color_data(FONT_MONO_DATA* mf);

void font_upgrade_to_color(FONT* f);

void morph_font(FONT *f);

#endif

   



