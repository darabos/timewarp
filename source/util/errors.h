#ifndef _ERRORS_H
#define _ERRORS_H

#define DO_STACKTRACE
#define USE_ALLEGRO

#define COMPILE_TIME_ASSERT(condition) typedef char _compile_time_assert_type_ ## __LINE__ [(condition) ? 1 : -1];

/*
 * int tw_alert(char *message, char *b1, char *b2, char *b3)
 *
 * Replaces allegro's alert and alert3. It handles carriage returns.
 */

#ifdef __cplusplus

int tw_alert(const char *message, const char *b1 = 0, const char *b2 = 0, const char *b3 = 0, const char *b4 = 0) ;

// for errors caught in "catch" clauses
void caught_error(const char *format, ...);

//quits TW with an error message
//used for catastrophic errors
void tw_error_exit(const char* message) ;

#ifdef DO_STACKTRACE
	class UserStackTraceHelper { public:
		UserStackTraceHelper(const char *file, int line);
		~UserStackTraceHelper();
	};
#	define STACKTRACE UserStackTraceHelper _stacktrace_ ## __LINE__ (__FILE__,__LINE__);
#else
#	define STACKTRACE
#endif

extern "C" {
#endif


void init_error() ;   //initialize error handling routines
void deinit_error();  //de-initialize error handling routines


/*
 * void tw_error(const char *format, ...)
 *
 * WARNING: the macro implementation of this function can cause compile 
 *     errors.  If your code complains about a line that calls 
 *     tw_error, wrap the call to tw_error with curly braces like 
 *     this: {tw_error("My error message");}
 *
 * This function is used to query the user what action to take when an error 
 * occurs.  It will pop up a box displaying the error string and prompting 
 * the user to abort, retry, or debug.  It may also display some diagnostic 
 * data, such as a call stack.  To avoid showing the call stack, call error 
 * instead of tw_error.  
 *
 * If the user clicks on "retry" then error() will return.
 *
 * If the user clicks on "abort" then error() will throw 0. This will cause 
 * one level of the program to be aborted. (i.e. from game to main menu)
 *
 * If the user clicks on "debug" then error() will attempt to abort the 
 * entire program in a way that lets the debugger work on things. On MSVC 
 * this is currently implemented with the assembly instruction "int 3".  On 
 * other platforms this is currently implemented by intentionally 
 * derefencing a NULL pointer.  
 * 
 */

#	define tw_error _prep_error(__FILE__, __LINE__); _error
	void error_handler ( const char *message);
	extern void (*_error_handler) ( const char *src_file, int line, const char *message );
	void _prep_error(const char *file, int line);
	void _error(const char *format, ...);

/*
 * void error(const char *format, ...)
 *
 * This function is more or less the same as tw_error(const char *format, ...)
 *
 * The primary difference is, this function will not display extra debugging 
 * information.  Specifically, it won't attempt to display the call stack.  
 * 
 */

	void error(const char *format, ...);


/*
 * void log_debug(const char *format, ...)
 *
 * Records a log of certain information to disk.
 * Typical log message: "Sound Initialized (10 voices sfx, 22 for music)"
 */
	void log_debug(const char *format, ...);

/*
 * void tw_error_exit(char* message)
 *
 * Informs a user about an error and exits. This function does not return.
 * This is for errors so critical that there is no hope of recovery
 */


/*
 * void tw_exit(int errorcode)
 *
 * Quits timewarp.  Calls the standard libc function exit() 
 * after cleaning up a few things.  
 *
 */
void tw_exit(int errorcode) ;

// not for user-code
//this flag is set to 0 normally, or 1 to bypass error catching
extern int __error_flag;


#if defined __cplusplus
}
#endif

#endif
