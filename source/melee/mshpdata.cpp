/* $Id$ */ 
/*
 * Star Control - TimeWarp
 *
 * melee/mshpdata.cpp - Melee ship data module
 *
 * 19-Jun-2002
 *
 * - Cosmetic code changes.
 * - Comments added.
 */

#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "../util/aastr.h"

#include "mship.h"
#include "mtarget.h"


int auto_unload = false;


// for debugging ...
void test_pointers()
{
#ifdef _DEBUG
	if (!physics)
		return;

	// also check the target list
	int i;
	for ( i = 0; i < targets->N; ++i)
	{
		targets->item[i]->exists();
	}

	// check if it's still in the physics list... which shouldn't be the case
	for ( i = 0; i < physics->num_items; ++i)
	{
		if (physics->item[i]->exists())
		{
			if (physics->item[i]->target)
				physics->item[i]->target->exists();

			if (physics->item[i]->ship)
				physics->item[i]->ship->exists();
		}
	}
#endif
}


/*------------------------------*
 *		Ship Data Registration  *
 *------------------------------*/


int num_shipdatas = 0;
ShipData **shipdatas = NULL;

int shipdatas_loaded = 0;

ShipData *shipdata ( const char *file ) {
	int i;
	if (!file) return NULL;
	for ( i = 0; i < num_shipdatas; i += 1 ) {
		if (!strcmp(file, shipdatas[i]->file) ) return shipdatas[i];
	}
	if (!exists(file)) return NULL;
	ShipData *data = new ShipData(file);
	num_shipdatas += 1;
	shipdatas = (ShipData**)realloc(shipdatas, num_shipdatas * sizeof(ShipData*));
	shipdatas[num_shipdatas-1] = data;
	return data;
}
/*
void load_ship_data(shiptype_type *type) {
	if(type->dat != NULL)
		return;
	ships_loaded += 1;
	char fn[256];
	sprintf(fn, "ships/shp%s.dat", type->id);
	type->dat = new ShipData(fn);
	type->dat->type = type;
	return;
}

void unload_ship_data(shiptype_type *type) {
	if(type->dat == NULL)
		return;
	ships_loaded -= 1;
	delete type->dat;
	type->dat = NULL;
	return;
}

void load_all_ship_data() {
	int i;
	for (i = 0; i < num_shiptypes; i += 1) {
		load_ship_data(&shiptypes[i]);
	}
	return;
}

void unload_all_ship_data() {
	int i;
	for (i = 0; i < num_shiptypes; i += 1) {
		unload_ship_data(&shiptypes[i]);
	}
	return;
}

void unload_unused_ship_data() {
	int i;
	for (i = 0; i < num_shiptypes; i += 1) {
		if (shiptypes[i].dat && (shiptypes[i].dat->references == 0)) {
			unload_ship_data(&shiptypes[i]);
		}
	}
	return;
}
*/


/*
 static BITMAP *copy_bitmap(BITMAP *old) {
	BITMAP *r;
	r = create_bitmap(old->w, old->h);
	blit (old, r, 0, 0, 0, 0, old->w, old->h);
	return r;
}
*/

void save_spacesprite2(SpaceSprite *ss, const char *spritename, const char *destination, const char *extension) {
	int i;
	char buf[512];

	if (ss->frames() != 64)
		tw_error("save_spacesprite2 - error");

	BITMAP *tmp = create_bitmap(int(ss->width() * 8), int(ss->height() * 8));
	for (i = 0; i < ss->frames(); i += 1) {
		blit(ss->get_bitmap(i), tmp, 0, 0, (i&7) * (int)ss->width(), int((i/8) * ss->height()), (int)ss->width(), (int)ss->height());
		sprintf(buf, "%s%i.%s", spritename, i, extension);
		save_bitmap(buf, tmp, NULL);
	}
	return;
}

void save_spacesprite(SpaceSprite *ss, const char *spritename, const char *destination, const char *extension) {
	int i;
	char buf[512];

	if (!ss)
		return;

	if (ss->frames()) {
		for (i = 0; i < ss->frames(); i += 1) {
			if (strchr(extension, '.')) {
				sprintf(buf, "tmp/%s%03d%s", spritename, i, extension);
			}
			else {
				sprintf(buf, "tmp/%s%03d.bmp", spritename, i);
			}

//			if ((i != ss->frames() - 1) && (ss->frames() == 64)) compress_bitmap(ss->get_bitmap(i), ss->get_bitmap(i+1));

			save_bitmap(buf, ss->get_bitmap(i), NULL);
		}

/*		chdir("tmp");
//		if (strchr(extension, '.')) sprintf(buf, "dat ../ships/%s.dat -k -t data -a *", destination);
//		else sprintf(buf, "dat ../ships/%s.dat -t %s -a *", destination, extension);
//		system(buf);
		sprintf(buf, "md ..\\ships\\%s", destination);
		system(buf);
		sprintf(buf, "move * ..\\ships\\%s", destination);
		system(buf);
		delete_file("*");
		chdir("..");
*/	

	}

	sprintf(buf, "tmp/%s.ini", spritename);
	set_config_file(buf);
	set_config_string("Main", "Type", "SpaceSprite");
	set_config_int("SpaceSprite", "Number", (int)ss->frames());
	set_config_int("SpaceSprite", "Width", (int)ss->width());
	set_config_int("SpaceSprite", "Height", (int)ss->height());
	set_config_string("SpaceSprite", "SubType", "Normal");
	set_config_string("SpaceSprite", "Extension", extension);
	chdir("tmp");
	sprintf(buf, "dat ../ships/%s.dat -k -a *", destination);
//	system(buf);
	sprintf(buf, "move * ..\\ships\\%s", destination);
	system(buf);
//	delete_file("*");
	chdir("..");
	return;
}

void save_samples(SpaceSprite *ss, const char *spritename, const char *destination, const char *extension) 
{
}

void ShipData::lock() {
	if (!islocked()) {
		push_config_state();//can screw up badly if an error occurs while loading...
		load();
		pop_config_state();
	}
	references += 1;
}

void ShipData::unlock() {
	references -= 1;
	if (!islocked() && auto_unload) {
		unload();
	}
}

bool ShipData::islocked()
{
	return references > 0;
}

void unload_all_ship_data() {
}
void unload_unused_ship_data() {
}

void ShipData::unload()
{
	//test_pointers();

	if (status != LOADED_FULL) return;

	if (spritePanel) {
		delete spritePanel;
		spritePanel = 0;
	}

	if (spriteShip) {
		delete spriteShip;
		spriteShip = 0;
	}
	if (spriteWeapon) {
		delete spriteWeapon;
		spriteWeapon = 0;
	}
	if (spriteWeaponExplosion) {
		delete spriteWeaponExplosion;
		spriteWeaponExplosion = 0;
	}
	if (spriteSpecial) {
		delete spriteSpecial;
		spriteSpecial = 0;
	}
	if (spriteSpecialExplosion) {
		delete spriteSpecialExplosion;
		spriteSpecialExplosion = 0;
	}
	if (spriteExtra) {
		delete spriteExtra;
		spriteExtra = 0;
	}
	if (spriteExtraExplosion) {
		delete spriteExtraExplosion;
		spriteExtraExplosion = 0;
	}

	if (num_more_sprites) {
		int i;

		for (i = 0; i < num_more_sprites; i += 1)
			delete more_sprites[i];

		//delete[] more_sprites;
		// this was created with the "realloc" command --> use free
		free( more_sprites );

		more_sprites = NULL;
		num_more_sprites = 0;
	}

	// clean up these pointer arrays ...
	// these were created with the "new" command --> use delete
	// also, the wave data are kept in memory... you've to remove those data
	if (data)
	{
		unload_datafile(data);
		data = 0;
	} else {
		int i;
		if (sampleWeapon)
		{
			for( i = 0; i < num_weapon_samples; ++i )
			{
				if (sampleWeapon[i])
					destroy_sample(sampleWeapon[i]);
			}
		}
		
		if (sampleSpecial)
		{
			for( i = 0; i < num_special_samples; ++i )
			{
				if (sampleSpecial[i])
					destroy_sample(sampleSpecial[i]);
			}
		}
		
		if (sampleExtra)
		{
			for( i = 0; i < num_extra_samples; ++i )
			{
				if (sampleExtra[i])
					destroy_sample(sampleExtra[i]);
			}
		}


		if (moduleVictory)
		{
			destroy_mod(moduleVictory);
			//moduleVictory = 0;
		}
	}

	if (sampleWeapon)
		delete [] sampleWeapon;
	if (sampleSpecial)
		delete [] sampleSpecial;
	if (sampleExtra)
		delete [] sampleExtra;

	sampleWeapon = 0;
	sampleSpecial = 0;
	sampleExtra = 0;
	moduleVictory = 0;


	shipdatas_loaded -= 1;
	status = LOADED_NONE;

	//test_pointers();

/*	num_weapon_samples0),
	sampleWeapon(NULL),
	num_special_samples(0),
	sampleSpecial(NULL),
	num_extra_samples(0),
	sampleExtra(NULL),
	moduleVictory(NULL)*/
}

ShipData::ShipData(const char *filename) :
	data(NULL),
	spriteShip(NULL),
	spriteWeapon(NULL),
	spriteWeaponExplosion(NULL),
	spriteSpecial(NULL),
	spriteSpecialExplosion(NULL),
	spriteExtra(NULL),
	spriteExtraExplosion(NULL),
	num_weapon_samples(0),
	sampleWeapon(NULL),
	num_special_samples(0),
	sampleSpecial(NULL),
	num_extra_samples(0),
	sampleExtra(NULL),
	moduleVictory(NULL)
{
	file = strdup(filename);
	references = 0;
	status = LOADED_NONE;
}

#include "mview.h"
SpaceSprite *load_sprite(const char *string, DATAFILE *data, int *index) 
{
	char buffy[512]; buffy[0] = 0;
	char *cp = buffy;
	char *tp;
	int argc, i;
	int rotations = 1;
	char **argv = get_config_argv("Objects", string, &argc);
	int count = 0;
	if (!argc) return NULL;
	count = atoi(argv[0]);
	if (!count) return NULL;
	tp = strchr(argv[0], 'r');
	for (i = 1; i < argc; i += 1) {
		if ((argv[i][0] == '-') || (argv[i][0] == '+')) {
			cp += sprintf(cp, "%s ", argv[i]);
		}
		else if (argv[i][0] == 'r') tp = argv[i];
		else {tw_error("load_sprite - unrecognized modifiers '%s'", argv[i]);}
	}
	if (tp) {
		rotations = atoi(tp+1);
		if (rotations == 0) rotations = 64;
	}
	SpaceSprite *sprite = NULL;
	int attrib = string_to_sprite_attributes(buffy);
	sprite = new SpaceSprite(&data[*index], count, attrib, rotations);

	/*
	// a sprite load/destroy-test (can be used to test memory-leaks:
	i = 0;
	for (;;)
	{
		++i;
		sprite = new SpaceSprite(&data[*index], count, attrib, rotations);
		delete sprite;
		if (i % 100 == 0)
		{
			message.print(100, 15, "load/destroy test: %i", i);
			message.animate(0);
			readkey();
		}
	}
	// end of the test.
	//*/

	
	for (i = 0; i < count; i += 1) {

		//xxx Geo - I don't think there is much to gain by doing this.
		// and why is it by default an RLE_SPRITE ?????
//		destroy_rle_sprite((RLE_SPRITE*)data[(*index)+i].dat);
//		data[(*index)+i].dat = NULL;
		// brutal hack to free up the memory
	}
	*index += count;




	return sprite;
}


void *copy_data(void *data, int N)
{
	if (!data)
	{
		tw_error("Unable to copy data.");
	}

	if (N < 0 || N > 1E7)
	{
		tw_error("Invalid data size");
	}

	void *d;
	if (N > 0)
	{
		//d = new unsigned char [N];
		d = malloc(N);
		memcpy(d, data, N);
	} else {
		d = 0;
	}

	return d;
}


SAMPLE *copy_sample(SAMPLE *source)
{
	SAMPLE *dest = (SAMPLE*) malloc(sizeof(SAMPLE));

	// copy the sample info
	memcpy(dest, source, sizeof(SAMPLE));

	// copy the sample data (and set the pointer to the sample data)
	dest->data = copy_data(source->data, (source->len * source->bits) / 8);

	return dest;
}

//void *jgmod_calloc (int size);

#define INIT_MEM(TYPE, NUM, DEST, SRC) \
	DEST = (TYPE*) malloc(NUM*sizeof(TYPE)); \
	memcpy(DEST, SRC, NUM*sizeof(TYPE));

// geo- I had to look into the JGMOD *load_jgm (JGMOD_FILE *f) function to understand
// how the data structures are organized and created... in the file load_jgm.c
JGMOD *copy_jgmod(JGMOD *source)
{

	JGMOD *dest;
	INIT_MEM(JGMOD, 1, dest, source);



	// certain number of instruments

	INIT_MEM(INSTRUMENT_INFO, dest->no_instrument, dest->ii, source->ii);

	// certain number of samples

	INIT_MEM(SAMPLE_INFO, dest->no_sample, dest->si, source->si);

	// allocate memory for the SAMPLE headers
	INIT_MEM(SAMPLE, dest->no_sample, dest->s, source->s);

	// initialize the SAMPLE data pointers (and the sample data).
	int i;
	for ( i = 0; i < dest->no_sample; ++i )
	{
		SAMPLE *s, *s_src;
		s = dest->s + i;	// the i-th sample.
		s_src = source->s + i;

		int L = s->len * s->bits / 8;
		s->data = malloc (L);
		memcpy(s->data, s_src->data, L);

	}


	INIT_MEM(PATTERN_INFO, dest->no_pat, dest->pi, source->pi);

	for ( i = 0; i < dest->no_pat; ++i )
	{
		PATTERN_INFO *pi, *pi_src;
		pi = dest->pi + i;
		pi_src = source->pi + i;

		int L = sizeof(NOTE_INFO) * dest->no_chn * pi->no_pos;
		pi->ni = (NOTE_INFO*) malloc(L);
		memcpy(pi->ni, pi_src->ni, L);
	}


	return dest;
}

void ShipData::load()
{
	//test_pointers();

	int i, index = 0, count;

//	if (status != LOADED_NONE) return;

	data = load_datafile(file);

	if(!data)
		tw_error("Error loading '%s'", file);

	push_config_state();
	set_config_data((char *)(data[index].dat), data[index].size);

	int num_panel_bitmaps = get_config_int("Objects", "PanelBitmaps", 0);

	index++;

	// load ship panel
	if (num_panel_bitmaps < 2)
		tw_error("Too few ship panel bitmaps");
	spritePanel = new SpaceSprite(&data[index], num_panel_bitmaps, SpaceSprite::IRREGULAR);
	index += num_panel_bitmaps;

	// load ship sprites
	spriteShip = load_sprite("ShipSprites", data, &index);

	// load weapon sprites
	spriteWeapon = load_sprite("WeaponSprites", data, &index);

	// load weapon explosion sprites
	spriteWeaponExplosion = load_sprite("WeaponExplosion", data, &index);

	// load special ability sprites
	spriteSpecial = load_sprite("SpecialSprites", data, &index);

	// load special ability explosion sprites
	spriteSpecialExplosion = load_sprite("SpecialExplosion", data, &index);

	// load extra sprites
	spriteExtra = load_sprite("ExtraSprites", data, &index);

	// load extra explosion sprites
	spriteExtraExplosion = load_sprite("ExtraExplosion", data, &index);

	//load optional super-extra sprites
	i = 0;
	more_sprites = NULL;
	while (true) {
		char buffy[512];
		sprintf(buffy, "ExtraExtraSprites%d", i);
		if (get_config_int("Objects", buffy, -1) == -1) break;
		more_sprites = (SpaceSprite**) realloc(more_sprites, (i+1) * sizeof(SpaceSprite*));
		more_sprites[i] = load_sprite(buffy, data, &index);
		i += 1;
	}
	num_more_sprites = i;

	// initialize ship victory ditty
	
	//moduleVictory = (JGMOD*)data[index].dat;
	moduleVictory = copy_jgmod((JGMOD*)data[index].dat);//(Music *) copy_data(data[index].dat, data[index].size);
	index++;

	// load weapon samples
	count = get_config_int("Objects", "WeaponSamples", 0);
	num_weapon_samples = count;
	if(count > 0) {
		sampleWeapon = new SAMPLE*[count];
		for(i = 0; i < count; i++) {
			//sampleWeapon[i] = (SAMPLE *)copy_data(data[index].dat, data[index].size + min_sample_amount);//(data[index].dat);
			//sampleWeapon[i] = (SAMPLE *)(data[index].dat);
			sampleWeapon[i] = copy_sample( (SAMPLE *)data[index].dat );
			index++;
		}
	}
	else sampleWeapon = NULL;

	
	// load special ability samples
	count = get_config_int("Objects", "SpecialSamples", 0);
	num_special_samples = count;
	if(count > 0) {
		sampleSpecial = new SAMPLE*[count];
		for(i = 0; i < count; i++) {
			//sampleSpecial[i] = (SAMPLE *)copy_data(data[index].dat, data[index].size + min_sample_amount);//(data[index].dat);
			//sampleSpecial[i] = (SAMPLE *)data[index].dat;
			sampleSpecial[i] = copy_sample( (SAMPLE *)data[index].dat );
			index++;
		}
	}
	else sampleSpecial = NULL;

	// load extra samples
	count = get_config_int("Objects", "ExtraSamples", 0);
	num_extra_samples = count;
	if(count > 0) {
		sampleExtra = new SAMPLE*[count];
		for(i = 0; i < count; i++) {
			//sampleExtra[i] = (SAMPLE *)copy_data(data[index].dat, data[index].size + min_sample_amount);
			//sampleExtra[i] = (SAMPLE *)data[index].dat;
			sampleExtra[i] = copy_sample( (SAMPLE *)data[index].dat );
			index++;
		}
	}
	else sampleExtra = NULL;

	shipdatas_loaded += 1;
	status = LOADED_FULL;

	// all data is copied, so now, you can discard the original data file !
	pop_config_state();
	unload_datafile(data);
	data = 0;

	// hmmm... in SAMPLE, there's a pointer to "data"
	//sound.play(sampleSpecial[0], 32, 128, 1024);
	
	//sound.play_music(moduleVictory);

	//test_pointers();

	return;

/*	if (tw_alert("transform this datafile?", "No", "Yes") != 2) 
		return;

	char dname[512], fname[512], header[8192];
	sprintf(dname, strstr(file, "shp")+3);
	dname[strlen(dname)-4] = 0;
	sprintf (header, "[Main]\n");
	sprintf (header, "type = Ship\n");

//panel bitmaps
	sprintf (header, "num_panel_bitmaps = %d\n", num_panel_bitmaps);
	for (i = 0; i < num_panel_bitmaps; i += 1) {
		sprintf(fname, "%s/panel%03d.pcx", dname, i);
		save_bitmap(fname, bitmapPanel[i], NULL);
		}*/

//sound effects
/*	fprintf (f, "num_weapon_samples = %d\n", num_weapon_samples);
	for (i = 0; i < num_weapon_samples; i += 1) {
		sprintf(fname, "%s/WeaponSample%03d.wav", dname, i);
		//save_sample(fname, bitmapPanel[i]);
		}
	fprintf (f, "num_special_samples = %d\n", num_special_samples);
	for (i = 0; i < num_weapon_samples; i += 1) {
		sprintf(fname, "%s/SpecialSample%03d.wav", dname, i);
		//save_sample(fname, bitmapPanel[i]);
		}
	fprintf (f, "num_extra_samples = %d\n", num_extra_samples);
	for (i = 0; i < num_extra_samples; i += 1) {
		sprintf(fname, "%s/ExtraSample%03d.wav", dname, i);
		//save_sample(fname, bitmapPanel[i]);
		}
	fclose(f);*/

//SpaceSprites
/*	sprintf(fname, "%s/000index.txt", dname);
	set_config_file(fname);
	
//	moduleVictory;
	save_spacesprite2(spriteShip,             "ShipSprite",             dname, ".bmp");
//	save_spacesprite(spriteWeapon,           "WeaponSprite",           dname, ".bmp");
//	save_spacesprite(spriteWeaponExplosion,  "WeaponExplosionSprite",  dname, ".bmp");
//	save_spacesprite(spriteSpecial,          "SpecialSprite",          dname, ".bmp");
//	save_spacesprite(spriteSpecialExplosion, "SpecialExplosionSprite", dname, ".bmp");
//	save_spacesprite(spriteExtra,            "ExtraSprite",            dname, ".bmp");
//	save_spacesprite(spriteExtraExplosion,   "ExtraExplosionSprite",   dname, ".bmp");
	return;*/
}

ShipData::~ShipData()
{
	unload();
/*	if(spriteShip)
    delete spriteShip;
	if(spriteWeapon)
		delete spriteWeapon;
	if(spriteWeaponExplosion)
		delete spriteWeaponExplosion;
	if(spriteSpecial)
		delete spriteSpecial;
	if(spriteSpecialExplosion)
		delete spriteSpecialExplosion;
	if(spriteExtra)
		delete spriteExtra;

	if (spritePanel)
    delete spritePanel;

	unload_datafile(data);*/

	free(file);
}
