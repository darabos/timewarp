
#include "base.h"


COMPILE_TIME_ASSERT(sizeof(char)==1);
COMPILE_TIME_ASSERT(sizeof(short)==2);
COMPILE_TIME_ASSERT(sizeof(int)==4);
COMPILE_TIME_ASSERT(sizeof(long)==4);

__call_before_main::__call_before_main ( void (*func)() ) {
	func();
}

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

