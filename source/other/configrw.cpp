
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
char *temp_dir = "save/temp";

void set_conf(char *f, bool option)
{
	config_read = option;

	char *s;

	if (config_read)
		s = construct_loadname(f);
	else
		s = construct_savename(f);

	set_config_file(s);
}




void makedir(char *name)
{
	#ifdef LINUX
	mkdir(name, 0755); 
	#else
	mkdir(name);
	#endif
}






char constr_name_return[512];
char *construct_name(char *refdir, char *savename, bool strip)
{
	char *fname = constr_name_return;

	strcpy(fname, refdir);
	strcat(fname, "/");
	int k = strlen(fname);
	strcat(fname, savename);

	// let the savename become a "single" file, no subdirs --> replace / by _
	if (strip)
	{
		char *s = &fname[k];
		while (*s)
		{
			if (*s == '/')
				*s = '_';
			++s;
		}
	}

	return fname;
}


char *construct_savename(char *savename)
{
	char *s;

	// uses an collapsed file-path (/ --> _)
	s = construct_name(temp_dir, savename, true);

	return s;
}


char *construct_loadname(char *loadname)
{
	char *s = "hi";

	// uses an collapsed file-path (/ --> _)
//	s = construct_name(temp_dir, loadname, true);
	// FOR NOW, DISABLE, TO TEST INIT BEHAVIOUR

	// uses an collapsed file-path (/ --> _)
	if (!exists(s))
		s = construct_name(source_dir, loadname, true);

	// uses the normal file-path
	if (!exists(s))
		s = construct_name(init_dir, loadname, false);

	return s;
}


void validate_directory(char *fname)
{
	char *s = fname;

	while (s = strchr(s, '/'))	// yes, this must be =, not ==
	{
		*s = 0;

		if (!file_exists(fname, FA_DIREC, 0))
		{
			// create the directory ?!
			makedir(fname);
		}

		*s = '/';
		s += 1;	// skip the '/' that you've already found.
	}

}


FILE *getsavefile(char *savename)
{
	// only write to the temp-directory !
	char *s = construct_savename(savename);
	
	validate_directory(s);

	return fopen(s, "wb");
}


FILE *getloadfile(char *loadname)
{
	char *s = 0;
	
	s = construct_loadname(loadname);

	return fopen(s, "rb");
}



static char replext_string[512];

char *replext(char *oldname, char *newext)
{
	strcpy(replext_string, oldname);
	char *s;
	s = strrchr(replext_string, '.');
	if (s)
		strcpy(s+1, newext);

	return replext_string;	// it's now the new name.
}
