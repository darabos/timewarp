
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

int string_to_sprite_attributes ( const char *s, int recommended ) {
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
	}
	if (strstr(s, "-masked")) {
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
int SpaceSprite::mip_max = 4;
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


/*class GraphicsTable {
	int num_spacesprites, max_spacesprites;
	SpaceSprite **spacesprites;
	BITMAP *register_bitmap      (const char *path);
	SpaceSprite *register_sprite (const char *path, int attributes);
} graphics_table;*/

void convert_bitmap(BITMAP *src, BITMAP *dest, int aa_mode) {
	//requires that both be memory bitmaps
	int obpp = bitmap_color_depth(src);
	int bpp = bitmap_color_depth(dest);
	if ((src->w != dest->w) || (src->h != dest->h)) tw_error("convert_bitmap - wrong size");
	//if (obpp == bpp) tw_error("convert_bitmap - color depths match");
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

void color_correct_bitmap(BITMAP *bmp, int masked) {
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

void SpaceSprite::generate_mipmaps() {
	int bpp, level, i;
	if (general_attributes & MIPMAPED) {
		bpp = bitmap_color_depth(b[0][0]);
		for (level = 1; level < MAX_MIP_LEVELS; level += 1) {
			int lw, lh;
			lw = (int)ceil(w * pow(0.5, level));
			lh = (int)ceil(h * pow(0.5, level));
			if ((lw < 4) || (lh < 4)) break;
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
				a |= AA_NOTRANS;
				a &=~AA_BLEND;
				aa_set_mode( a );
				aa_stretch_blit(src, dest, 0,0,src->w,src->h, 0,0,dest->w, dest->h );
				//aa_stretch_blit(b[0][i], b[level][i], 0, 0, w, h, 0, 0, lw, lh);
			}
		}
	}
}

void SpaceSprite::change_color_depth(int newbpp) {
	int i, l;
	for (l = 0; l <= highest_mip; l += 1) {
		for (i = 0; i < count; i += 1) {
			BITMAP *tmp = create_bitmap_ex(newbpp, w, h);
			convert_bitmap(b[l][i], tmp, general_attributes & MASKED);
			if (attributes[i] & DEALLOCATE_IMAGE) destroy_bitmap(b[l][i]);
			attributes[i] |= DEALLOCATE_IMAGE;
			b[l][i] = tmp;
		}
	}
	return;
}

void SpaceSprite::permanent_phase_shift ( int phase ) {
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
	delete tmp;

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
	delete tmp;
	return;
}

SpaceSprite::SpaceSprite(const DATAFILE *images, int sprite_count, int _attributes, int rotations) {
	int i, j, obpp;
	BITMAP *bmp, *tmp = NULL;

	if (_attributes == -1) _attributes = string_to_sprite_attributes(NULL);

	count = sprite_count * rotations;
	if ((rotations < 1) || (count < 1)) tw_error("SpaceSprite::SpaceSprite - bad parameters");

	references = 0;
	highest_mip = 0;
	for (i = 1; i < MAX_MIP_LEVELS; i += 1) {
		b[i] = NULL;
	}

	general_attributes = _attributes;
	if (general_attributes &  MATCH_SCREEN_FORMAT)
		bpp = videosystem.bpp;
	else bpp = 0;
	if (general_attributes & ALPHA) {
		if (bpp <= 16) bpp = 16;
		else bpp = 32;
	}

	originaltype = images[0].type;
	switch (originaltype) {
		case DAT_RLE_SPRITE: {
			w = ((RLE_SPRITE *)images[0].dat)->w;
			h = ((RLE_SPRITE *)images[0].dat)->h;
			obpp = ((RLE_SPRITE *)images[0].dat)->color_depth;
			if (bpp == 0) bpp = obpp;
			if (obpp != bpp) tmp = create_bitmap_ex(obpp, w, h);
			}
		break;
		case DAT_BITMAP: {
			w = ((BITMAP *)images[0].dat)->w;
			h = ((BITMAP *)images[0].dat)->h;
			obpp = bitmap_color_depth((BITMAP *)images[0].dat);
			if (bpp == 0) bpp = obpp;
			}
		break;
		default: {
			tw_error("SpaceSprite::SpaceSprite - bad data file");
			}
		break;
		}
	if (general_attributes & IRREGULAR) goto irregular;
	for(i = 1; i < sprite_count; i++) {
		if (images[i].type != originaltype) tw_error ("SpaceSprite - bad data file");
		switch (originaltype) {
			case DAT_RLE_SPRITE: {
				if ((w != ((RLE_SPRITE *)images[i].dat)->w) || 
						(h != ((RLE_SPRITE *)images[i].dat)->h) ) tw_error("SpaceSprite - size changed");
				if (((RLE_SPRITE *)images[i].dat)->color_depth != obpp) tw_error("SpaceSprite - changing source color depth not yet allowed");
				}
			break;
			case DAT_BITMAP: {
				if ((w != ((BITMAP *)images[i].dat)->w) || 
						(h != ((BITMAP *)images[i].dat)->h) ) tw_error("SpaceSprite - size changed");
				if (bitmap_color_depth((BITMAP *)images[i].dat) != obpp) tw_error("SpaceSprite - changing source color depth not yet allowed");
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
		if (tmp != bmp) clear_to_color(tmp, bitmap_mask_color(tmp));
		switch (originaltype) {
			case DAT_RLE_SPRITE: {
				draw_rle_sprite(tmp, (RLE_SPRITE *)(images[i].dat), 0, 0);
				if ((general_attributes & ALPHA) && (obpp == 16)) rgba4444_as_rgb16 (tmp);
				if (tmp != bmp) convert_bitmap(tmp, bmp, general_attributes & MASKED);
				}
			break;
			case DAT_BITMAP: {
				if ((general_attributes & ALPHA) && (obpp == 16)) rgba4444_as_rgb16 ((BITMAP *)(images[i].dat));
				if (obpp != bpp) {
					convert_bitmap((BITMAP *)(images[i].dat), bmp, general_attributes & MASKED);
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

	return;//end of normal/masked/autorotated

	irregular:

	if (rotations != 1) tw_error (" irregular SpaceSprites are not permitted to be autorotated");

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
				if (bpp == obpp) {
					bmp = tmp;
					tmp = NULL;
				}
				else {
					bmp = create_bitmap_ex ( bpp, w, h);
					convert_bitmap(tmp, bmp, general_attributes & MASKED);
					destroy_bitmap(tmp);
					tmp = NULL;
					}
				}
			break;
			case DAT_BITMAP: {
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
	highest_mip = 0;
	return;
}

void SpaceSprite::unlock() {
	int i, j;
	for (i = 0; i < MAX_MIP_LEVELS; i += 1) {
		if (b[i]) j = i;
	}
	/*
	We aught to rebuild mipmaps	but too much work, so for now we do nothing
	*/
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
	if (strcmp(tmpstr, "SpaceSprite")) tw_error("SpaceSprite(%s # %s) : %s != SpaceSprite", sourcename, spritename, tmpstr);
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
/*
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
		tw_error("Error loading SpaceSprite: that's not a SpaceSprite!");
	ext = get_config_string(name, "Extension", "");

	m = new PPMASK*[count * rotations];
	b = new BITMAP*    [count * rotations];
	for (int i = 0; i < count; i += 1) {
		sprintf(buf, "%s/%s%03d%s", package, name, i, ext);
		BITMAP *bmp = load_bitmap(buf, NULL);
		b[i * rotations] = bmp;
		m[i * rotations] = create_ppmask(bmp);
		if (!m[i * rotations]) tw_error ("Error loading SpaceSprite: image %s not found", buf);
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

/*
SpaceSprite::SpaceSprite(BITMAP **sprites, int sprite_count) {
	int i, j;
	BITMAP *bmp;
	rotations = 1;
	count = sprite_count;
	if (count < 1) tw_error("SpaceSprite::SpaceSprite - bad parameters (2)");
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
	int i, l;

	for(i = 0; i < count; i++) {
		if (attributes[i] & DEALLOCATE_MASK) destroy_pmask(m[i]);
		}
	delete m;
	m = NULL;

	for (l = 0; l <= highest_mip; l += 1) {
		for(i = 0; i < count; i++) {
			if (attributes[i] & DEALLOCATE_IMAGE) destroy_bitmap(b[l][i]);
		}
		delete b[l];
		b[l] = NULL;
	}

	delete attributes;
	attributes = NULL;
	return;
}

BITMAP *SpaceSprite::get_bitmap(int index)
{
	if (general_attributes & MIPMAPED) if (highest_mip > 0) 
		tw_error ("get_bitmap on a mipmaped sprite!\n(retry likely to work)");
	if (index >= count) tw_error("SpaceSprite::get_bitmap - index %d > count %d", index, count);
	if (index < 0) tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count);
	highest_mip = 0;
	return(b[0][index]);
}
BITMAP *SpaceSprite::get_bitmap_readonly(int index)
{
	if (index >= count) {tw_error("SpaceSprite::get_bitmap_readonly - index %d > count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap_readonly - index %d < 0 (count %d)", index, count); index = 0;}
	return(b[0][index]);
}

void SpaceSprite::animate_character(Vector2 pos, int index, int color, Frame *space) {
	if (index >= count) {tw_error("SpaceSprite::animate_character - index %d > count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	pos = corner(pos, size());
	int ix = iround(pos.x);
	int iy = iround(pos.y);
	int spr_w = iround(w * space_zoom);
	int spr_h = iround(h * space_zoom);
	if(spr_w < 1) spr_w = 1;
	if(spr_h < 1) spr_h = 1;
	draw_character(ix, iy, spr_w, spr_h, index, color, space);
	return;
	}

void SpaceSprite::overlay (int index1, int index2, BITMAP *dest) {
	int x, y;
	if (general_attributes & MIPMAPED) tw_error ("overlay on a mipmaped sprite!  oh no!");
	if (index1 > count) tw_error("SpaceSprite::overlay - index1 %d > count %d", index1, count);
	if (index2 > count) tw_error("SpaceSprite::overlay - index2 %d > count %d", index2, count);

	for(y = 0; y < h; y += 1) {
		for(x = 0; x < w; x += 1) {
			if (getpixel(b[0][index1], x, y) != getpixel(b[0][index2], x, y)) {
				putpixel(dest, x, y, getpixel(b[0][index2], x, y));
				}
			}
		}
	return;
	}

void SpaceSprite::draw(Vector2 pos, Vector2 size, int index, BITMAP *surface) {
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
		iw = round_up(size.x);
		ih = round_up(size.y);
		aa_stretch_blit(bmp, surface, 0, 0, bmp->w, bmp->h, 
					ix, iy, iw, ih);
	}
	return;
}

void SpaceSprite::draw(Vector2 pos, Vector2 size, int index, Frame *frame) {
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
	BITMAP *bmp = b[mip][index];
	aa_set_mode(find_aa_mode(general_attributes));
	if (tw_aa_mode & AA_NO_ALIGN) {
		ix = round_down(pos.x);
		iy = round_down(pos.y);
		iw = round_up(pos.x+size.x) + 1 - ix;
		ih = round_up(pos.y+size.y) + 1 - iy;
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
	return;
}

void SpaceSprite::draw(int x, int y, int index, BITMAP *surface) {
	if (index >= count) {tw_error("SpaceSprite::draw - index %d >= count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	BITMAP *bmp = b[0][index];
	if (general_attributes & MASKED) 
		masked_blit (bmp, surface, 0, 0, x, y, bmp->w, bmp->h);
	else 
		blit        (bmp, surface, 0, 0, x, y, bmp->w, bmp->h);
	return;
	}

void SpaceSprite::animate(Vector2 pos, int index, Frame *space, double scale) {
	if (index >= count) {
		tw_error("SpaceSprite::animate - index %d >= count %d", index, count);
		return;
		}
	if (index < 0) {
		tw_error("SpaceSprite::animate - index %d < 0 (count %d)", index, count);
		return;
		}
	Vector2 s = size() * scale;
	draw(corner(pos, s ), s * space_zoom, index, space);
	return;
	}

void SpaceSprite::draw_character(int x, int y, int index, int color, BITMAP *bmp) {
	if (index >= count) {tw_error("SpaceSprite::draw_character - index %d >= count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	draw_allegro_pmask ( m[index], bmp, x, y, color );
	return;
}

void SpaceSprite::draw_character(int x, int y, int index, int color, Frame *space) {
	draw_character(x, y, index, color, space->surface);
	space->add_box(x, y, w, h);
	}

void SpaceSprite::draw_character(int x, int y, int w, int h, int index, int color, Frame *space) {
	draw_character(x, y, w, h, index, color, space->surface);
	space->add_box(x, y, w, h);
	}

void SpaceSprite::draw_character(int x, int y, int w, int h, int index, int color, BITMAP *bmp) {
	if (index >= count) {tw_error("SpaceSprite::draw_character_stretch - index %d >= count %d", index, count); index = 0;}
	if (index < 0) {tw_error("SpaceSprite::get_bitmap - index %d < 0 (count %d)", index, count); index = 0;}
	draw_allegro_pmask_stretch(m[index], bmp, x, y, w, h, color) ;
	return;
	}

int SpaceSprite::collide(int x, int y, int i, int ox, int oy, int oi,
		SpaceSprite *other)
{
	if (i >= count) tw_error("SpaceSprite::collide - index1 %d >= count1 %d", i, count);
	if (oi >= other->count) tw_error("SpaceSprite::collide - index2 %d >= count2 %d", oi, other->count);
	if (i < 0) tw_error("SpaceSprite::collide - index1 %d < count1 %d", i, count);
	if (oi < 0) tw_error("SpaceSprite::collide - index2 %d < count2 %d", oi, other->count);
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
