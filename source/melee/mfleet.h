#ifndef _MFLEET_H
#define _MFLEET_H

#include "../melee.h"

class Fleet;
extern Fleet *reference_fleet;

void init_fleet();	// inits reference_fleet [former shiptype array]

class Fleet{

public:
	ShipType* ship[MAX_FLEET_SIZE];
	char title[80];
	int  size;
	int  cost;

	Fleet();
	void reset();

	void *serialize (int *psize);
	void deserialize(void *data, int psize);

	void select_slot(int slot, ShipType *type);
	void clear_slot (int slot);

//	int ship_idtoindex(const char *id_str);

	void save(const char *filename, const char *section);
	void load(const char *filename, const char *section);

	void sort();

	typedef unsigned int Index;

	void sort ( int (*compare_function)(const Index *, const Index *)) ;
	void sort_alphabetical(const char *item, const char *section = "Info") ;
	void sort_numerical(const char *item, const char *section = "Info") ;
	static Fleet *sorting;
};


int fleetsort_clean ( const Fleet::Index *_i1, const Fleet::Index *_i2 ) ;
int fleetsort_by_name ( const Fleet::Index *_i1, const Fleet::Index *_i2 ) ;
int fleetsort_by_cost ( const Fleet::Index *_i1, const Fleet::Index *_i2 ) ;

#endif

