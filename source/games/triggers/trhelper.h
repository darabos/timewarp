
#ifndef __TRHELPER__
#define __TRHELPER__



char *skipspaces(char *s);
char *strsub(char **newstr, char *s, char separator);



void file_write(char *s, FILE *f);
void file_read(char *s, FILE *f);
void file_init(char **s, FILE *f);

void file_write(int i, FILE *f);
void file_read(int *i, FILE *f);

void file_write(double d, FILE *f);
void file_read(double *d, FILE *f);

void file_write(int N, int *iarray, FILE *f);
void file_read(int *N, int *iarray, FILE *f);

#endif
