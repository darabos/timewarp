
#ifndef __CONFIG_RW__
#define __CONFIG_RW__


/** \brief sets the direction for all subsequent calls to conf: read from the config file, or write to the config file

*/
extern bool config_read;

/** \brief the read-setting for the config_read variable
*/
extern const bool CONFIG_READ;

/** \brief the write-setting for the config_read variable
*/
extern const bool CONFIG_WRITE;

/** \brief sets the section string for all subsequent calls to conf.

*/
extern char *section;

/** \brief reads or writes an integer value to a allegro config file. Required: config_read
and section must be propertly defined. This works for values of int, double, and string. With
this, you can write a single "config" subroutine, which reads or writes all variables based
on the config_read setting.

*/

void conf(char *id, int &x, int def = 0);

void conf(char *id, double &x, double def = 0);

void conf(char *id, char *x, char *def = "none");

/** \brief front-end for conf for writing or reading an indexed value to a config file. Indexes
are appended to a general id, starting with "1". This works for values of int, double or string.

\param id0 is the general id for accessing value x
\param i is the index for accessing value x
*/


void confnum(char *id0, int i, int &x);

void confnum(char *id0, int i, double &x);

void confnum(char *id0, int i, char *x);




#endif

