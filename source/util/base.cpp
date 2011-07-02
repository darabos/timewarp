/* $Id$ */ 

#include <stdlib.h>
#include <string.h>
#include "base.h"

__call_before_main::__call_before_main ( void (*func)() ) {
	func();
}

char *tw_strdup ( const char *str ) {
	int l = strlen(str);
	char *r = (char*) malloc(l+1);
	strcpy(r, str);
	return r;
}

#ifdef TW_MALLOC_CHECK
#	undef malloc
#	undef realloc
#	undef free
	void *tw_malloc(int size) {
		void *r = malloc(size);
		if (!r) error_oom();
		return r;
	}
	void *tw_realloc(void *old, int size) {
		void *r = realloc(old, size);
		if (!r && size) error_oom();
		return r;
	}
	void tw_free(void *mem) {
		free(mem);
	}
/*	void *operator new(unsigned int size) {
		return tw_malloc(size);
	}
	void operator delete(void *mem) {
		tw_free(mem);
	}*/
#endif

/*------------------------------
		Base Class
------------------------------*/

void BaseClass::preinit() {
	return;
}
BaseClass::~BaseClass() {
}
void **BaseClass::get_vtable_pointer() const {
	if (sizeof(this) != sizeof(void*)) 
		throw "get_vtable_pointer failed (ptr* size weird)";
		//error("get_vtable_pointer failed (size == %d != !%d)", sizeof(this), sizeof(void*));
	return ((void**)this);
}
int BaseClass::serialize(void *stream) {
	return 0;
}
int BaseClass::_get_size() const {
	return 0;
}
void BaseClass::_event( Event *e) {
	return;
}
void BaseClass::issue_event ( int num, BaseClass **recipients, Event *e) {
	int i;
	for ( i = 0; i < num; i += 1) {
		recipients[i]->_event(e);
	}
}


/** \brief You can use this to generate buffered data across the network. These are shared
on the network, and taken from it, in a neat, orderly way so that information doesn't get
mixed up. Make sure that you can also handle the null-case, where fake data are produced so
that there's an initial networking buffer (yeah that's needed, otherwise each has to
wait for the other).
*/
/*
void BaseClass::gen_buffered_data()
{
	// do nothing by default
}
*/

