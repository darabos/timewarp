#ifndef _BASE_H
#define _BASE_H

#include "types.h"


#ifndef __i386__
#	if defined(__I386__) || defined(__IA32__) || defined(__ia32__)
#		define __i386__
#	endif
#endif

#ifdef __cplusplus

class __call_before_main { 
public:__call_before_main ( void (*func)());
};
#define CALL_BEFORE_MAIN(a) static __call_before_main __call_ ## a ( a ) ;
#define CALL_BEFORE_MAIN2(id, code) static void __call_before_main2 ## id(){code;} CALL_BEFORE_MAIN(__call_before_main2 ## id)

//#define REGISTER_UDT(a,b) CALL_BEFORE_MAIN ( __udt_register_ ## a );
//#define DECLARE_UDT virtual int getsize(); virtual const char *get_class_name(); virtual const char *get_class_parent_name();

struct _Ignore_Me { };//dummy type

class Event;
class BaseClass {
public:
	virtual void preinit();
	virtual void _event( Event * e);
	virtual ~BaseClass ();                 //does nothing, but is necessary
	void **get_vtable_pointer () const;    //returns the address of the virtual table pointer inside of an instance
	void issue_event ( int num, BaseClass **list, Event *event);
	virtual int serialize (void *stream);  //returns 0 on failure
	virtual int _get_size() const;          //returns 0 on failure
};


class Event : public BaseClass {
public:
	short type;
	short subtype;


	enum { 
		VIDEO = 1, 
		KEYBOARD,
		MOUSE, 
		NETWORK,
		LAST_GENERIC_EVENT = 15
	}; //generic events

	enum {
		TW_NET1 = LAST_GENERIC_EVENT + 1, 
		TW_CONFIG
	}; //TimeWarp events

	virtual int _get_size() const {return sizeof(*this);}
};

#endif
#endif
