
#include <allegro.h>
#include <string.h>
#include <stdio.h>

#include "configrw.h"


bool config_read = true;

const bool CONFIG_READ = true;

const bool CONFIG_WRITE = false;

char *section = 0;


void conf(char *id, int &x, int def)
{
	if (config_read)
		x = get_config_int(section, id, def);
	else
		set_config_int(section, id, x);
}

void conf(char *id, double &x, double def)
{
	if (config_read)
		x = get_config_float(section, id, def);
	else
		set_config_float(section, id, x);
}

void conf(char *id, char *x, char *def)
{
	if (config_read)
		strcpy(x, get_config_string(section, id, def));
	else
		set_config_string(section, id, x);
}


void confnum(char *id0, int i, int &x)
{
	char id[128];
	sprintf(id, "%s%i", id0, i);
	conf(id, x);
}

void confnum(char *id0, int i, double &x)
{
	char id[128];
	sprintf(id, "%s%i", id0, i);
	conf(id, x);
}

void confnum(char *id0, int i, char *x)
{
	char id[128];
	sprintf(id, "%s%i", id0, i);
	conf(id, x);
}



char *init_dir = "gamedata";
char *source_dir = "save/save01";
char *target_dir = "save/save01";

void set_conf(char *f)
{
	char s[512];
	
	if (config_read)
		strcpy(s, source_dir);
	else
		strcpy(s, target_dir);

	// to-do:
	// check if the target-directory exists, if not, create it.
//	if (!file_exists(s, FA_DIREC, 0))		// this is too limited, "f" can also contain directory info...
//		mkdir(s);

	strcat(s, "/");
	strcat(s, f);

	set_config_file(s);
}


