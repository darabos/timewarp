


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
REGISTER_FILE


#include "trhelper.h"




char *skipspaces(char *s)
{
	int i;

	i = 0;
	while (s[i] == ' ' && s[i] != 0)
		++i;

	if (s[i] != 0)
		s += i;
	else
		s = 0;

	return s;
}

char *strsub(char **newstr, char *s, char separator)
{
	int i;

	i = 0;
	while (s[i] != separator && s[i] != 0)
		++i;

	char *stemp = new char [i+1];
	(*newstr) = stemp;

	strncpy((*newstr), s, i);
	(*newstr)[i] = 0;

	if (s[i] != 0)
		s += i;
	else
		s = 0;

	return s;
}




void file_write(char *s, FILE *f)
{
	int N;

	if (s)
		N = strlen(s) + 1;
	else
		N = 0;

	fwrite(&N, sizeof(int), 1, f);
	fwrite(s, sizeof(char), N, f);
}

void file_read(char *s, FILE *f)
{
	int N;
	fread(&N, sizeof(int), 1, f);
	if (N > 0)
		fread(s, sizeof(char), N, f);
	else
		s[0] = 0;	// yes s[0]=0 cause the space was already allocated.
}

// init, because it reads, and initializes, a string.
void file_init(char **s, FILE *f)
{
	int N;
	fread(&N, sizeof(int), 1, f);
	if (N > 0)
	{
		*s = new char [N];
		fread(*s, sizeof(char), N, f);
	}
	else
		*s = 0;		// nothing allocated; pointer points to nothing.
}



void file_write(int i, FILE *f)
{
	fwrite(&i, sizeof(int), 1, f);
}

void file_read(int *i, FILE *f)
{
	fread(i, sizeof(int), 1, f);
}


void file_write(double d, FILE *f)
{
	fwrite(&d, sizeof(double), 1, f);
}

void file_read(double *d, FILE *f)
{
	fread(d, sizeof(double), 1, f);
}



void file_write(int N, int *iarray, FILE *f)
{
	fwrite(&N, sizeof(int), 1, f);
	fwrite(iarray, sizeof(int), N, f);
}

void file_read(int *N, int *iarray, FILE *f)
{
	fread(N, sizeof(int), 1, f);
	fread(iarray, sizeof(int), *N, f);
}



