
#include <vector>

void init_profiling();
void deinit_profiling();

struct PROFILE_DATUM {
	const SOURCE_LINE * srcline;

	int runs;
	PROFILE_TIME_TYPE2 time;
};

class Profile {
public:
	char active;
	char sorted;
	enum { SORTED_ALPHA, SORTED_TIME };
	std::vector<PROFILE_DATUM> data;
public:
	PROFILE_TIME_TYPE2 total_time;

	Profile();
	~Profile();
//	Profile &operator= (const Profile &p);
	Profile &operator+=(const Profile &p);
	Profile &operator-=(const Profile &p);
	void clear();
	int length() const {return data.size();}
	//int length() const {return data.num();}
	const PROFILE_DATUM &operator[] ( int i ) const {return data[i];}

	void _sort_time();
	void _sort_alpha();
	void sort_time();  //call before reading
	void sort_alpha(); //call after reading
	int  print ( int index, char *dest, int max ) ;
//	void print ( int index, BITMAP *surface, int x, int y, int c );
};


Profile *get_master_profile();

class SelectProfile {
	Profile &data;
	Profile *temp;
	PROFILE_TIME_TYPE2 time;
public:
	SelectProfile ( Profile &_data );
	~SelectProfile ();
};
