#ifndef __GAMEX_SPRITES__
#define __GAMEX_SPRITES__

void colorize(SpaceSprite *spr, double mr, double mg, double mb);

void load_bitmaps(BITMAP **bmp, char *dirname, char *filename, int N);

void destroy_bitmaps(BITMAP **bmp, int N);

void replace_color(BITMAP *bmp, int col1, int col2);


// all the sprite-relevant information (and nothing else).
//SpaceSprite::SpaceSprite(BITMAP **bmplist, int sprite_count, int rotations, int _attributes);

#endif
