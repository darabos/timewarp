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
 *
 * 2003.10.16 youbastrd Rewrote most of this class to use a list instead of an array.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "mfleet.h"
#include "../util/net_tcp.h"

#include <algorithm>
#include <functional>
using namespace std;


//
static const char *_fleetsort_ini_alphabetical_default = "\255\255\255";
static const int _fleetsort_ini_numerical_default = 0x0fffffff;

static char _fleetsort_ini_section[80] = "Info";
static char _fleetsort_ini_item[80] = "";



//global variable used in at least 42 places. 
//TODO remove this global variable and use proper Object-oriented techinques.
Fleet* reference_fleet = NULL;

//global function to initialize reference_fleet
//TODO remove this global function and use proper Object-oriented techinques.

void init_fleet() {STACKTRACE
	if(reference_fleet)
		return;
	reference_fleet = new Fleet();
	int i;
	for(i = 0; i < num_shiptypes; i++)
        reference_fleet->addShipType( &shiptypes[i] );
		//reference_fleet->select_slot(i, &shiptypes[i] );//replaced by preceding line

	reference_fleet->Sort();
    //TODO figure out what next line was supposed to do; was replaced by preceding line
    //reference_fleet->sort(fleetsort_by_name);
	//reference_fleet->sort_alphabetical("Origin");
    
}


/*
ShipType es = { "_???_", "", "[Empty slot]", NULL, 0, NULL, NULL };
ShipType* empty_slot = &es;

typedef unsigned int Index;

Fleet *Fleet::sorting = NULL;

void Fleet::select_slot(int slot, ShipType *type) {STACKTRACE
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

void Fleet::clear_slot(int slot) {STACKTRACE
	if (slot < 0) return;
  if (ship[slot] == empty_slot)
    return;

  cost -= ship[slot]->cost;
  ship[slot] = empty_slot;
  size--;
}

void Fleet::save(const char *filename, const char *section) {STACKTRACE
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


//int Fleet::ship_idtoindex(const char *id_str)
//{
//  int i = 0, found = FALSE;
//
//  if(id_str == NULL) return -1;
//
//  while((i < num_shiptypes) && (!found)) {
//    if(strcmp(id_str, shiptypes[i].id) == 0)
//      found = TRUE;
//    else
//      i++;
//  }
//
//  if(found)
//     return(i);
//  else
//    return(-1);
//}


void Fleet::load(const char *filename, const char *section) {STACKTRACE
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

void Fleet::sort(int (*compare_function)(const Index *p1, const Index *p2)) {STACKTRACE
	Index indexes[MAX_FLEET_SIZE];
	int i;
	for (i = 0; i < MAX_FLEET_SIZE; i += 1) {
		indexes[i] = i;
	}
	// not thread-safe, just a half-assed effort
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

void Fleet::sort_alphabetical(const char *item, const char *section) {STACKTRACE
	_fleetsort_ini_section = section;
	_fleetsort_ini_item = item;
	sort(_fleetsort_alphabetical);
	return;
}

void Fleet::sort_numerical(const char *item, const char *section) {STACKTRACE
	_fleetsort_ini_section = section;
	_fleetsort_ini_item = item;
	sort(_fleetsort_numerical);
	return;
}

void Fleet::sort() {STACKTRACE
	sort(fleetsort_clean);
	return;
}


/**/


    Fleet::Fleet() {STACKTRACE
        cost = 0;
        memset(title, '\0', MAX_TITLE_LENGTH);
    };

    void Fleet::reset() {STACKTRACE
        ships.clear();
        this->cost = 0;
        memset(title, '\0', MAX_TITLE_LENGTH);
    }

    void * Fleet::serialize (int *psize) {STACKTRACE
        unsigned char buffy[65536];
        int s = 0;
        int j;
        j = intel_ordering(getSize());
        memcpy(&buffy[s], &j, sizeof(j)); s += sizeof(j);

        MyFleetListType::iterator iter = NULL;

        for (iter = ships.begin(); iter != ships.end(); iter++) {
            char k = strlen((*iter)->id);
            if (k > 64)
                tw_error("serialize_fleet - that's a hell of a long ship id");
            memcpy(&buffy[s], &k, sizeof(k)); s += sizeof(k);
        }

        for (iter = ships.begin(); iter != ships.end(); iter++) {
            memcpy(&buffy[s], (*iter)->id, strlen((*iter)->id)); s += strlen((*iter)->id); 
        }

        unsigned char *holder = (unsigned char *) malloc(s);
        memcpy(holder, buffy, s);
        *psize = s;
        return holder;    
    }



#define READ(a) if (int(s+sizeof(a))>psize) {delete k;tw_error ("deserialize_fleet - bad!");}memcpy(&a, &buffy[s], sizeof(a)); s += sizeof(a);
#define READ2(a,b) if (b+s>psize) {delete k;tw_error ("deserialize_fleet - bad!");}memcpy(&a, &buffy[s], b); s += b;

    void Fleet::deserialize(void *data, int psize) {STACKTRACE        
        unsigned char *buffy = (unsigned char *) data;
        int s = 0;
        int j;
        unsigned char *k = NULL;

        reset();
        
        READ(j);
        
        
        //handed by reset()
        //size = 0;
        //cost = 0;
        //for(int i = 0; i < MAX_FLEET_SIZE; i++ )
          //  ship[i] = empty_slot;

        int _fleet_size = intel_ordering(j);
        if (_fleet_size > MAX_FLEET_SIZE) 
        {tw_error("fleet too large! (%d ships)", _fleet_size);}
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
            else addShipType(t);
            //else select_slot(j, t);
        }
        if (s != psize)
            tw_error("deserialize_fleet - didn't use all the data...");
        if (k)
            delete k;
        return;
    }
#undef READ
#undef READ2
       
    


    //void Fleet::select_slot(int slot, ShipType *type) {STACKTRACE
    //}

    int Fleet::addShipType(ShipType * type) {
        if ( (getSize() >= MAX_FLEET_SIZE) || (type == NULL))
            return -1;

        cost += type->cost;
        ships.push_back(type);

        return ships.size()-1;
    }

    void Fleet::clear_slot (int slot) {STACKTRACE
        ships.erase( &ships.at(slot) );
    }

    ShipType * Fleet::getShipType(int slot) {STACKTRACE
        if ( (slot<0) || (slot>=ships.size()))
            return NULL;

        return ships.at(slot);
    }

    void Fleet::save(const char *filename, const char *section) {STACKTRACE
        int i=0, count = 0;
        char slot_str[8];
    
        sort(ships.begin(), ships.end());
        
        if (filename) 
            set_config_file(filename);

        for (MyFleetListType::iterator iter = ships.begin(); iter != ships.end(); iter++) {
            sprintf(slot_str, "Slot%d", count);
            set_config_string(section, slot_str, (*iter)->id);
            count ++;
        }

        set_config_int(section, "Size", count);
        set_config_string(section, "Title", title);
    }

    void Fleet::load(const char *filename, const char *section) {STACKTRACE
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
                addShipType(type);
                //replaced with preceding line
                //select_slot(count, type );
                count++;
            }
        }
        return;
    }


    struct _NumericConfigDescending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 

            if (!x || !y)
                return false;

            set_config_file(x->file);
            int tmp1 = get_config_int(_fleetsort_ini_section,
                                      _fleetsort_ini_item, 
                                      _fleetsort_ini_numerical_default);
            set_config_file(y->file);
            int tmp2 = get_config_int(_fleetsort_ini_section,
                                      _fleetsort_ini_item, 
                                      _fleetsort_ini_numerical_default);
            if (!tmp1 || !tmp2)
                return false;

            return (tmp1 < tmp2);
       }
    };/**/

    struct _NumericConfigAscending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 

            if (!x || !y)
                return false;

            set_config_file(x->file);
            int tmp1 = get_config_int(_fleetsort_ini_section,
                                      _fleetsort_ini_item, 
                                      _fleetsort_ini_numerical_default);
            set_config_file(y->file);
            int tmp2 = get_config_int(_fleetsort_ini_section,
                                      _fleetsort_ini_item, 
                                      _fleetsort_ini_numerical_default);
            if (!tmp1 || !tmp2)
                return false;

            return (tmp1 > tmp2);
       }
    };/**/

    struct _AlphabeticConfigAscending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 

            if (!x || !y)
                return false;

            set_config_file(x->file);
            char * tmp1 = strdup(get_config_string(_fleetsort_ini_section,
                                                   _fleetsort_ini_item, 
                                                   _fleetsort_ini_alphabetical_default));
            set_config_file(y->file);
            char * tmp2 = strdup(get_config_string(_fleetsort_ini_section,
                                                   _fleetsort_ini_item, 
                                                   _fleetsort_ini_alphabetical_default));
            if (!tmp1 || !tmp2)
                return false;

            return (strncmp(tmp1,tmp2,80) > 0);
       }
    };/**/

    struct _AlphabeticConfigDescending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 

            if (!x || !y)
                return false;

            set_config_file(x->file);
            char * tmp1 = strdup(get_config_string(_fleetsort_ini_section,
                                                   _fleetsort_ini_item, 
                                                   _fleetsort_ini_alphabetical_default));
            set_config_file(y->file);
            char * tmp2 = strdup(get_config_string(_fleetsort_ini_section,
                                                   _fleetsort_ini_item, 
                                                   _fleetsort_ini_alphabetical_default));
            if (!tmp1 || !tmp2)
                return false;

            return (strncmp(tmp1,tmp2,80) < 0);
       }
    };/**/



    struct _nameDecending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 
            if ((x) && (y))
                return strcmp(x->name, y->name) < 0; 
            else 
                return false; 
        }
    };

    struct _nameAscending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 
            if (x && y)
                return strcmp(x->name, y->name) > 0; 
            else 
                return false; 
        }
    };

    struct _costDecending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 
            if (x && y)
               return (x->cost > y->cost);  
            else
                return false;
        }
    };
    struct _costAscending : public binary_function<ShipType *, ShipType *, bool> {
        bool operator()(ShipType * x, ShipType * y) { 
            if (x && y)
                return (x->cost < y->cost); 
            else 
                return false; 
        }
    };
    
    
    void Fleet::Sort(SortingMethod sortMethod, int startIndex, int endIndex) {
        MyFleetListType::iterator _begin, _end;
        int _size = ships.size();

        if (startIndex < 0) startIndex = 0;
        if (startIndex >= _size) startIndex = _size;
        if ( (endIndex < 0) && (endIndex != -1) ) endIndex = 0;
        if (endIndex > _size) endIndex = _size;

        if  ( (startIndex > endIndex) && (endIndex != -1) )
        {
            int temp = endIndex;
            endIndex = startIndex;
            startIndex = temp;
        }

        if (startIndex == endIndex)
            return;


        if (startIndex == 0)
            _begin = ships.begin();
        else
            _begin = &ships.at(startIndex);

        if (endIndex == -1)
            _end = ships.end();
        else
            _end = &ships.at(endIndex);

        switch (sortMethod) {
            case SORTING_METHOD_COST_DESCENDING:
                sort(_begin, _end, _costDecending());
                break;

            case SORTING_METHOD_COST_ASCENDING:
                sort(_begin, _end, _costAscending());
                break;

            case SORTING_METHOD_NAME_ASCENDING:    
                sort(_begin, _end, _nameAscending());
                break;

            case SORTING_METHOD_TWCOST_ASCENDING:
                strcpy(_fleetsort_ini_item, "TWCost");
                sort(_begin, _end, _NumericConfigAscending());
                break;

            case SORTING_METHOD_TWCOST_DESCENDING:    
                strcpy(_fleetsort_ini_item, "TWCost");
                sort(_begin, _end, _NumericConfigDescending());
                break;

            case SORTING_METHOD_NAME1_ASCENDING:
                strcpy(_fleetsort_ini_item, "Name1");
                sort(_begin, _end, _AlphabeticConfigAscending());
                break;

            case SORTING_METHOD_NAME1_DESCENDING:    
                strcpy(_fleetsort_ini_item, "Name1");
                sort(_begin, _end, _AlphabeticConfigDescending());
                break;

            case SORTING_METHOD_NAME2_ASCENDING:
                strcpy(_fleetsort_ini_item, "Name1");
                sort(_begin, _end, _AlphabeticConfigAscending());
                break;

            case SORTING_METHOD_NAME2_DESCENDING:    
                strcpy(_fleetsort_ini_item, "Name1");
                sort(_begin, _end, _AlphabeticConfigDescending());
                break;

            case SORTING_METHOD_CODERS_ASCENDING:
                strcpy(_fleetsort_ini_item, "Coders");
                sort(_begin, _end, _AlphabeticConfigAscending());
                break;

            case SORTING_METHOD_CODERS_DESCENDING:    
                strcpy(_fleetsort_ini_item, "Coders");
                sort(_begin, _end, _AlphabeticConfigDescending());
                break;

            case SORTING_METHOD_ORIGIN_ASCENDING:
                strcpy(_fleetsort_ini_item, "Origin");
                sort(_begin, _end, _AlphabeticConfigAscending());
                break;

            case SORTING_METHOD_ORIGIN_DESCENDING:    
                strcpy(_fleetsort_ini_item, "Origin");
                sort(_begin, _end, _AlphabeticConfigDescending());
                break;

            case SORTING_METHOD_NAME_DESCENDING:    
            case SORTING_METHOD_DEFAULT:
            default:
                sort(ships.begin(), ships.end(), _nameDecending());
        }
    }
