#ifndef _awconfig_h
#define _awconfig_h

//allegro based implementation:

class INI_File {
	char *_fname;
	int _network_channel;
	int _id;
	static int _last_accessed;
	static int _last_issued;
	void _prepare() const;//not really very const
public:
	const char *get_name() const { return _fname; }
	bool is_networked() const { return _network_channel == -1; }
	int get_channel() const { return _network_channel; }
	int get_int (const char *section, const char *value, int default_value) const;
	void set_int (const char *section, const char *value, int new_value);
	double get_float (const char *section, const char *value, double default_value) const;
	void set_float (const char *section, const char *value, double new_value);
	const char *get_string (const char *section, const char *value, const char *default_value) const;
	void set_string (const char *section, const char *value, const char *new_value);

	INI_File ( const char *fname, int network_channel = -1 );
	~INI_File();

	void flush();

//	void change_file ( const char *fname );
};

#endif//_awconfig_h