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
#include "mview.h"


int auto_unload = false;


char *create_filename(const char *base_directory, const char *file_name)
{
	static char result[512];

	if (file_name[0] == '/')
	{
		// skip the '/' character
		++file_name;

		// use "absolute" path, wrt the game root
		// i.e., ignore the current custom base_directory.
		strcpy(result, file_name);

	} else if (file_name[0] == '.' && file_name[1] == '.' && file_name[2] == '/')
	{
		// relative path
		char tmp_dir[512];
		strcpy(tmp_dir, base_directory);

		while (file_name[0] == '.' && file_name[1] == '.' && file_name[2] == '/')
		{
			// skip the ../ part
			file_name += 3;

			// also reduce the base-dir:
			char *tmp = strrchr(tmp_dir, '/');
			if (tmp)
				*tmp = 0;
		}

		// finally, combine the two:
		strcpy(result, tmp_dir);
		strcat(result, "/");
		strcat(result, file_name);

	} else {
		// normal path
		strcpy(result, base_directory);
		strcat(result, "/");
		strcat(result, file_name);
	}

	return result;
}


// for debugging ...
void test_pointers()
{
	//xxx test
	return;

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

ShipData *shipdata ( const char *file )
{
	int i;

	if (!file)
		return NULL;

	for ( i = 0; i < num_shipdatas; i += 1 )
	{
		// if these data already exist
		if (strcmp(file, shipdatas[i]->file) == 0 )
			return shipdatas[i];
	}

	// well... there's now also the option of a directory, so ... don't perform this check anymore
	//if (!exists(file))
	//	return NULL;

	num_shipdatas += 1;
	shipdatas = (ShipData**)realloc(shipdatas, num_shipdatas * sizeof(ShipData*));

	ShipData *data = new ShipData(file);
	shipdatas[num_shipdatas-1] = data;

	return data;
}


void save_spacesprite2(SpaceSprite *ss, const char *spritename, const char *destination, const char *extension) {
	int i;
	char buf[512];

	if (ss->frames() != 64)
		tw_error("save_spacesprite2 - error");

	BITMAP *tmp = create_bitmap(int(ss->width(0) * 8), int(ss->height(0) * 8));
	for (i = 0; i < ss->frames(); i += 1) {
		blit(ss->get_bitmap(i), tmp, 0, 0, (i&7) * (int)ss->width(i), int((i/8) * ss->height(i)), (int)ss->width(i), (int)ss->height(i));
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
	set_config_int("SpaceSprite", "Width", (int)ss->width(0));
	set_config_int("SpaceSprite", "Height", (int)ss->height(0));
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

void ShipData::unlock()
{
	references -= 1;

	if (references < 0)
	{
		tw_error("Too few references.");
	}

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



/** load (copy) a sprite from a data file in memory */
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

	
	*index += count;

	return sprite;
}





/** load a sprite from a set of .bmp files in format *_01.bmp from disk.
A sample line can read: ShipSprites = ship 1  +r +alpha.
The first item is the base of the file name, the second is the number of files to read into
the sprite, and then there are options. */
SpaceSprite *load_sprite(const char *ini_string, char *dirname, int default_attrib) 
{
	// first, read the arguments from the .ini string.
	int argc;
	char **argv = get_config_argv("Objects", ini_string, &argc);
	if (!argc) return NULL;

	// the first argument is the base of the filename.
	char base_filename[512];
	strcpy(base_filename, create_filename(dirname, argv[0]));

	// the second is the number of files that you should read.
	int count = 0;
	count = atoi(argv[1]);
	if (!count)
		return 0;

	// and then, there are some options.

	char buffy[512]; buffy[0] = 0;
	char *cp = buffy;
	int rotations = 1;
	
	int i;
	// arg 0 = base name
	// arg 1 = count
	// start checking for options at 2.
	for (i = 2; i < argc; i += 1)
	{
		char *command = argv[i];
		if (strcmp(command, "+r")==0 || strcmp(command, "+rotate")==0)
		{
			// use default rotations (64)
			rotations = 64;

		} else if ((argv[i][0] == '-') || (argv[i][0] == '+'))
		{
			// read spacesprite attributes.
			cp += sprintf(cp, "%s ", argv[i]);
		} else
		{
			tw_error("Unrecognized command %s in %s", argv[i], base_filename);
		}
	}

	// read the bitmaps into memory (in the colordepth of the file)
	// these are temporary - they usually require a color-depth conversion anyway.

	// allow count start at 0 or at 1...
	int count_base = 0;

	BITMAP **bmplist = new BITMAP* [count];
	for ( i = 0; i < count; ++i )
	{
		char filename[512];
		sprintf(filename, "%s%02i.bmp", base_filename, i+count_base);	// combine the filename-base, the _number, and the .bmp extension
		bmplist[i] = load_bitmap(filename, 0);
		if (!bmplist[i])
		{
			if (i == 0)
			{
				// perhaps you should start counting at 1 instead of 0.
				--i;	// start checking at 0 again
				++count_base;	// but with a higher base count.

				if (count_base > 1)
				{
					tw_error("Count base too high for sprite image %s", filename);
				}
			} else {
				tw_error("Failed to load sprite image file %s", filename);
			}
		}
	}

	int attrib = default_attrib | string_to_sprite_attributes(buffy, 0);

	SpaceSprite *sprite = new SpaceSprite(bmplist, count, attrib, rotations);

	// remove the temporary image data
	for ( i = 0; i < count; ++i )
	{
		destroy_bitmap(bmplist[i]);
	}
	delete [] bmplist;

	
	return sprite;
}



/** load .wav samples */
void load_sample(char *ini_string, char *dirname, int *num_weapon_samples, Sound*** sampleWeapon)
{
	int argc;
	char **argv = get_config_argv("Objects", ini_string, &argc);
	if (!argc)
		return;

	// base name
	char base_filename[512];
	strcpy(base_filename, create_filename(dirname, argv[0]));

	// load weapon samples
	int count = atoi(argv[1]);
	if (!count)
		return;

	Sound **sample;

	
	sample = new SAMPLE*[count];

	int i;
	for(i = 0; i < count; i++)
	{
		char filename[512];
		sprintf(filename, "%s%02i.wav", base_filename, i+1);

		sample[i] = load_sample(filename);//copy_sample( (SAMPLE *)data[index].dat );

		if (!sample[i])
		{
			tw_error("Failed to initialize sample %s", filename);
		}

	}


	*num_weapon_samples = count;
	*sampleWeapon = sample;
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
	int mult;
	if (source->stereo != 0)
		mult = 2;
	else
		mult = 1;

	dest->data = copy_data(source->data, (source->len * (source->bits/8) * mult));

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

		int mult;
		if (s->stereo != 0)
			mult = 2;
		else
			mult = 1;

		int L = mult * s->len * s->bits / 8;
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


void ShipData::load_datafile(DATAFILE *data)
{
	int i, index = 0, count;

	push_config_state();
	set_config_data((char *)(data[index].dat), data[index].size);

	int num_panel_bitmaps = get_config_int("Objects", "PanelBitmaps", 0);

	index++;

	// load ship panel
	if (num_panel_bitmaps < 2)
		tw_error("Too few ship panel bitmaps");
	spritePanel = new SpaceSprite(&data[index], num_panel_bitmaps);
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

	// all data is copied, so now, you can discard the original data file !
	pop_config_state();
	unload_datafile(data);

	// hmmm... in SAMPLE, there's a pointer to "data"
	//sound.play(sampleSpecial[0], 32, 128, 1024);
	
	//sound.play_music(moduleVictory);

	//test_pointers();
}





void ShipData::load_directory(char *dirname)
{
	// load the ship configuration data.
	char info_filename[512];
	strcpy(info_filename, dirname);
	strcat(info_filename, "/content.ini");

	push_config_state();
	set_config_file(info_filename);

	int basic_attrib = SpaceSprite::MASKED;

	// load the ship panel sprites.
	spritePanel = load_sprite("PanelBitmaps", dirname, basic_attrib);

	// load ship sprites
	spriteShip = load_sprite("ShipSprites", dirname, basic_attrib);

	// load weapon sprites
	spriteWeapon = load_sprite("WeaponSprites", dirname, basic_attrib);

	// load weapon explosion sprites
	spriteWeaponExplosion = load_sprite("WeaponExplosion", dirname, basic_attrib);

	// load special ability sprites
	spriteSpecial = load_sprite("SpecialSprites", dirname, basic_attrib);

	// load special ability explosion sprites
	spriteSpecialExplosion = load_sprite("SpecialExplosion", dirname, basic_attrib);

	// load extra sprites
	spriteExtra = load_sprite("ExtraSprites", dirname, basic_attrib);

	// load extra explosion sprites
	spriteExtraExplosion = load_sprite("ExtraExplosion", dirname, basic_attrib);

	//load optional super-extra sprites
	int i = 0;
	more_sprites = NULL;
	for (;;)
	{
		char buffy[512];
		sprintf(buffy, "ExtraExtraSprites%d", i);

		if (get_config_int("Objects", buffy, -1) == -1)
			break;

		more_sprites = (SpaceSprite**) realloc(more_sprites, (i+1) * sizeof(SpaceSprite*));
		more_sprites[i] = load_sprite(buffy, dirname, 0);
		i += 1;
	}
	num_more_sprites = i;



	// initialize ship victory ditty
	
	char *modfilename;
	modfilename = create_filename(dirname, get_config_string("Objects", "Ditty", ""));
	moduleVictory = load_mod(modfilename);

	load_sample("WeaponSamples", dirname, &num_weapon_samples, &sampleWeapon);

	load_sample("SpecialSamples", dirname, &num_special_samples, &sampleSpecial);

	load_sample("ExtraSamples", dirname, &num_extra_samples, &sampleExtra);
	

	// all data is copied, so now, you can discard the original data file !
	pop_config_state();

}





void ShipData::load()
{
	//test_pointers();

	DATAFILE *data;
	data = ::load_datafile(file);

	if(data)
	{
		load_datafile(data);
		//tw_error("Error loading '%s'", file);
	} else {
		char tmp[512];
		strcpy(tmp, file);
		char *dot = strrchr(tmp, '.');	//Find last occurrence of character in string
		*dot = 0;	// now, the dot becomes the end of the string.

		load_directory(tmp);	// without the .dat extension.
	}

	shipdatas_loaded += 1;
	status = LOADED_FULL;
}



ShipData::~ShipData()
{
	unload();

	free(file);
}
