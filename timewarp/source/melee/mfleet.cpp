/*
 * Star Control - TimeWarp
 *
 * melee/mfleet.cpp - fleet module
 *
 * 19-Jun-2002
 *
 * - _fleetsort_alphabetical() and _fleetsort_numerical() functions modified to load
 *   ship ini files according to shp*.* file naming convention.
 * - Cosmetic code changes.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "mfleet.h"
#include "../util/net_tcp.h"

ShipType es = { "_???_", "[Empty slot]", "\255\255\255", 0, NULL, NULL };
ShipType* empty_slot = &es;

Fleet* reference_fleet = NULL;

void init_fleet() {
	if(reference_fleet)
		return;
	reference_fleet = new Fleet();
	int i;
	for(i = 0; i < num_shiptypes; i++)
		reference_fleet->select_slot(i, &shiptypes[i] );
	reference_fleet->sort(fleetsort_by_name);
	reference_fleet->sort_alphabetical("Origin");
}

Fleet *Fleet::sorting = NULL;
Fleet::Fleet() {
	reset();
}

void Fleet::reset() {
	size = 0;
	cost = 0;
	for( int i = 0; i < MAX_FLEET_SIZE; i++ ) ship[i] = empty_slot;
	sprintf(title, "Untitled");
}

void *Fleet::serialize(int *psize) {
	unsigned char buffy[65536];
	int s = 0;
	int j;
	j = intel_ordering(size);
	memcpy(&buffy[s], &j, sizeof(j)); s += sizeof(j);
	for (j = 0; j < size; j += 1) {
		char k = strlen(ship[j]->id);
		if (k > 64)
			tw_error("serialize_fleet - that's a hell of a long ship id");
		memcpy(&buffy[s], &k, sizeof(k)); s += sizeof(k);
	}
	for (j = 0; j < size; j += 1) {
		memcpy(&buffy[s], ship[j]->id, strlen(ship[j]->id)); s += strlen(ship[j]->id);
	}
	unsigned char *holder = (unsigned char *) malloc(s);
	memcpy(holder, buffy, s);
	*psize = s;
	return holder;
}

void Fleet::deserialize(void *data, int psize) {

#define READ(a) if (int(s+sizeof(a))>psize) {delete k;tw_error ("deserialize_fleet - bad!");}memcpy(&a, &buffy[s], sizeof(a)); s += sizeof(a);
#define READ2(a,b) if (b+s>psize) {delete k;tw_error ("deserialize_fleet - bad!");}memcpy(&a, &buffy[s], b); s += b;

	unsigned char *buffy = (unsigned char *) data;
	int s = 0;
	int j;
	unsigned char *k = NULL;

	READ(j);

	size = 0;
	cost = 0;
	for(int i = 0; i < MAX_FLEET_SIZE; i++ )
		ship[i] = empty_slot;
	// I think fleet title should also be serialized

	int _fleet_size = intel_ordering(j);
	if (_fleet_size > MAX_FLEET_SIZE)
		tw_error("fleet too large! (%d ships)", _fleet_size);
	k = new unsigned char[_fleet_size];
	for (j = 0; j < _fleet_size; j += 1) {
		READ(k[j]);
		if (k[j] > MAX_SHIP_ID) {
			tw_error ("deserialize_fleet - that's a long shipid! (%d)", k[j]);
		}
	}
	char sname[64];
	for (j = 0; j < _fleet_size; j += 1) {
		READ2(sname, k[j]);
		sname[k[j]] = '\0';
		ShipType *t = shiptype(sname);
		if (!t) {
			tw_error("deserialize fleet - bad shiptype (%s)", sname);
		}
		else select_slot(j, t);
	}
	if (s != psize)
		tw_error("deserialize_fleet - didn't use all the data...");
	delete k;

#undef READ
#undef READ2

	return;
}

void Fleet::select_slot(int slot, ShipType *type) {
	if (slot < 0)
		return;
	if (!type)
		tw_error("select_fleet_slot - bad type");

	if (ship[slot] == empty_slot) size += 1;
	else cost -= ship[slot]->cost;

	if (type == empty_slot) size -= 1;
	else cost += type->cost;

	ship[slot] = type;
	return;
}

void Fleet::clear_slot(int slot) {
	if (slot < 0) return;
  if (ship[slot] == empty_slot)
    return;

  cost -= ship[slot]->cost;
  ship[slot] = empty_slot;
  size--;
}

void Fleet::save(const char *filename, const char *section) {
	int i, count = 0;
	char slot_str[8];

	sort();

	if (filename) set_config_file(filename);

	for(i = 0; i < MAX_FLEET_SIZE; i++) {
		if(ship[i] != empty_slot) {
		  sprintf(slot_str, "Slot%d", count);
		  set_config_string(section, slot_str, ship[i]->id);
		  count++;
		}
	}

	for(i = count; i < MAX_FLEET_SIZE; i++) {
		sprintf(slot_str, "Slot%d", i);
		set_config_string(section, slot_str, NULL);
	}

	set_config_int(section, "Size", count);
	set_config_string(section, "Title", title);

	return;
}

/*
int Fleet::ship_idtoindex(const char *id_str)
{
  int i = 0, found = FALSE;

  if(id_str == NULL) return -1;

  while((i < num_shiptypes) && (!found)) {
    if(strcmp(id_str, shiptypes[i].id) == 0)
      found = TRUE;
    else
      i++;
  }

  if(found)
     return(i);
  else
    return(-1);
}
*/

void Fleet::load(const char *filename, const char *section) {
  int i, count;
  ShipType *type;
  char slot_str[8];
  const char *slot_id, *_fleet_title;

  reset();

  if (filename) set_config_file(filename);
  int _fleet_size = get_config_int(section, "Size", 0);
  _fleet_title = get_config_string(section, "Title", "");
  sprintf(title, _fleet_title);

  count = 0;
  for(i = 0; i < _fleet_size; i++) {
		sprintf(slot_str, "Slot%d", i);
		slot_id = get_config_string(section, slot_str, "?????");
		type = shiptype(slot_id);
		if (type) {
			select_slot(count, type );
			count++;
		}
	}
	return;
}

int fleetsort_clean(const Fleet::Index *_i1, const Fleet::Index *_i2) {
	ShipType **s = Fleet::sorting->ship;
	Fleet::Index i1 = *_i1, i2 = *_i2;
	if (s[i1] == empty_slot) return 1;
	if (s[i2] == empty_slot) return -1;
	return i1 - i2;
}

int fleetsort_by_name(const Fleet::Index *_i1, const Fleet::Index *_i2) {
	ShipType **s = Fleet::sorting->ship;
	Fleet::Index i1 = *_i1, i2 = *_i2;
	int i = strcmp(s[i1]->name, s[i2]->name);
	if (i == 0)	return i1 - i2;
	else return i;
}

int fleetsort_by_cost(const Fleet::Index *_i1, const Fleet::Index *_i2) {
	ShipType **s = Fleet::sorting->ship;
	Fleet::Index i1 = *_i1, i2 = *_i2;
	int i = s[i1]->cost - s[i2]->cost;
	if (i == 0)	return i1 - i2;
	else return i;
}

void Fleet::sort(int (*compare_function)(const Index *p1, const Index *p2)) {
	Index indexes[MAX_FLEET_SIZE];
	int i;
	for (i = 0; i < MAX_FLEET_SIZE; i += 1) {
		indexes[i] = i;
	}
	/* not thread-safe, just a half-assed effort*/
	while (sorting) idle(5);
	sorting = this;
	qsort(indexes, MAX_FLEET_SIZE, sizeof(Index), (int (*)(const void*, const void *))compare_function);
	sorting = NULL;

	ShipType *copy[MAX_FLEET_SIZE];
	memcpy(copy, ship, sizeof(ShipType*) * MAX_FLEET_SIZE);
	for (i = 0; i < MAX_FLEET_SIZE; i += 1) {
		ship[i] = copy[indexes[i]];
	}

	return;
}

static const char *_fleetsort_ini_section = NULL;
static const char *_fleetsort_ini_item = NULL;
static const char *_fleetsort_ini_alphabetical_default = "\255\255\255";
static int _fleetsort_ini_numerical_default = 0x0fffffff;

static int _fleetsort_alphabetical(const Fleet::Index *_i1, const Fleet::Index *_i2) {
	ShipType **s = Fleet::sorting->ship;
	Fleet::Index i1 = *_i1, i2 = *_i2;
	set_config_file(s[i1]->file);
	char *tmp1 = strdup(get_config_string(
		_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_alphabetical_default
	));
	set_config_file(s[i2]->file);
	char *tmp2 = strdup(get_config_string(
		_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_alphabetical_default
	));
	if (!tmp1 && !tmp2) return i1 - i2;
	if (!tmp1) return 1;
	if (!tmp2) return -1;
	int i = strcmp(tmp1, tmp2);
	if (i == 0) return i1 - i2;
	else return i;
}

static int _fleetsort_numerical(const Fleet::Index *_i1, const Fleet::Index *_i2) {
	ShipType **s = Fleet::sorting->ship;
	Fleet::Index i1 = *_i1, i2 = *_i2;
	set_config_file(s[i1]->file);
	int tmp1 = get_config_int(
		_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_numerical_default
	);
	set_config_file(s[i2]->file);
	int tmp2 = get_config_int(
		_fleetsort_ini_section,
		_fleetsort_ini_item,
		_fleetsort_ini_numerical_default
	);
	int i = tmp1 - tmp2;
	if (i == 0) return i1 - i2;
	else return i;
}

void Fleet::sort_alphabetical(const char *item, const char *section) {
	_fleetsort_ini_section = section;
	_fleetsort_ini_item = item;
	sort(_fleetsort_alphabetical);
	return;
}

void Fleet::sort_numerical(const char *item, const char *section) {
	_fleetsort_ini_section = section;
	_fleetsort_ini_item = item;
	sort(_fleetsort_numerical);
	return;
}

void Fleet::sort() {
	sort(fleetsort_clean);
	return;
}

/*
void Fleet::sort_by_name(){
	qsort_by_name( 0, MAX_FLEET_SIZE - 1 );
}

void Fleet::qsort_by_name( int lo0, int hi0 ){
  if( hi0 > lo0 ){
    int lo = lo0;
    int hi = hi0;
    const char* mid = ship[(int)((lo0 + hi0) / 2)]->name;
    while( lo <= hi ){
      while( lo < hi0 && strcmp( mid, ship[lo]->name ) > 0 )
        lo++;
      while( hi > lo0 && strcmp( mid, ship[hi]->name ) < 0 )
        hi--;
      if( lo <= hi ){
        if( strcmp( ship[lo]->name, ship[hi]->name )){
          shiptype_type* tmp = ship[lo];
          ship[lo] = ship[hi];
          ship[hi] = tmp;
        }
        lo++;
        hi--;
      }
    }
    if( lo0 < hi ) qsort_by_name( lo0, hi );
    if( lo < hi0 ) qsort_by_name( lo, hi0 );
  }
}


void Fleet::sort_by_cost(){
	empty_slot->cost = 999999;
	qsort_by_cost( 0, MAX_FLEET_SIZE - 1 );
	empty_slot->cost = 0;
}

void Fleet::qsort_by_cost( int lo0, int hi0 ){
  if( hi0 > lo0 ){
    int lo = lo0;
    int hi = hi0;
    int mid = ship[(int)((lo0 + hi0) / 2)]->cost;
    while( lo <= hi ){
      while( lo < hi0 && mid > ship[lo]->cost )
        lo++;
      while( hi > lo0 && mid < ship[hi]->cost )
        hi--;
      if( lo <= hi ){
        if( ship[lo]->cost != ship[hi]->cost ){
          shiptype_type* tmp = ship[lo];
          ship[lo] = ship[hi];
          ship[hi] = tmp;
        }
        lo++;
        hi--;
      }
    }
    if( lo0 < hi ) qsort_by_cost( lo0, hi );
    if( lo < hi0 ) qsort_by_cost( lo, hi0 );
  }
}

void Fleet::sort_by( const char* property ){
	int i;
	char *properties[MAX_FLEET_SIZE];
	for( i = 0; i < MAX_FLEET_SIZE; i++ ){
		char fn[256];
		sprintf( fn, "ships/%s.ini", ship[i]->id );
		set_config_file( fn );
		properties[i] = (char*)malloc( 256 );
		sprintf( properties[i], get_config_string( "Info", property, "\255\255\255" ));
	}
	qsort_by( properties, 0, MAX_FLEET_SIZE - 1 );
	for( i = 0; i < MAX_FLEET_SIZE; i++ ) free( properties[i] );
	sort();		// just to be sure
}

void Fleet::qsort_by( char** properties, int lo0, int hi0 ){
  if( hi0 > lo0 ){
    int lo = lo0;
    int hi = hi0;
    char* mid = properties[(int)((lo0 + hi0) / 2)];
    while( lo <= hi ){
      while( lo < hi0 && strcmp( mid, properties[lo] ) > 0 )
        lo++;
      while( hi > lo0 && strcmp( mid, properties[hi] ) < 0 )
        hi--;
      if( lo <= hi ){
        if( strcmp( properties[lo], properties[hi] )){
          shiptype_type* tmp = ship[lo];
          ship[lo] = ship[hi];
          ship[hi] = tmp;
          char *tmpc = properties[lo];
          properties[lo] = properties[hi];
          properties[hi] = tmpc;
        }
        lo++;
        hi--;
      }
    }
    if( lo0 < hi ) qsort_by( properties, lo0, hi );
    if( lo < hi0 ) qsort_by( properties, lo, hi0 );
  }
}

void Fleet::sort(){
  int i = 0;
  int j; 
  while( true ){
    while( ship[i] != empty_slot && i < MAX_FLEET_SIZE - 1 ) i++;
    j = i;
    while( ship[j] == empty_slot && j < MAX_FLEET_SIZE - 1 ) j++;
    if( i == j ) break;
    if( ship[j] == empty_slot ) break;
    ship[i] = ship[j];
    ship[j] = empty_slot;
  }
}
*/

