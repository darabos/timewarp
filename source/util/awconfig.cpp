#include <string.h>
#include <allegro.h>

#include "base.h"
#include "awconfig.h"

int INI_File::_last_accessed = 0;
int INI_File::_last_issued = 0;

INI_File::INI_File ( const char *fname, int network_channel ) {
	_fname = strdup(fname);
	_network_channel = network_channel;
	_id = ++_last_issued;
}
INI_File::~INI_File() {
	free (_fname);
}
void INI_File::_prepare() const {
	_last_accessed = _id;
	return;
}
int INI_File::get_int (const char *section, const char *value, int default_value) const {
	_prepare(); return get_config_int(section, value, default_value);
}
void INI_File::set_int (const char *section, const char *value, int new_value) {
	_prepare(); set_config_int(section, value, new_value);
}
double INI_File::get_float (const char *section, const char *value, double default_value) const {
	_prepare(); return get_config_float(section, value, default_value);
}
void INI_File::set_float (const char *section, const char *value, double new_value) {
	_prepare(); set_config_float(section, value, new_value);
}
const char *INI_File::get_string (const char *section, const char *value, const char *default_value) const {
	_prepare(); return get_config_string(section, value, default_value);
}
void INI_File::set_string (const char *section, const char *value, const char *new_value) {
	_prepare(); set_config_string(section, value, new_value);
}
void INI_File::flush() {
	if (_last_accessed == _id) flush_config_file();
}
//	void INI_File::change_file ( const char *fname );

