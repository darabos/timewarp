/* $Id$ */ 
#include <string.h>
#include <stdio.h>
#include <allegro.h>
#include <stdarg.h>

#include "../melee.h"
REGISTER_FILE

#include "../scp.h"

void dump_physics(const char *message)
{
	char fname[512];
	sprintf(fname, "tw_error.txt");

	FILE *f;
	f = fopen(fname, "wt");
	if (!f)
		return;

	fprintf(f, "timewarp error message is:\n%s\n", message);

	if (!physics)
		return;

	int i;
	for ( i = 0; i < physics->num_items; ++i )
	{
		SpaceLocation *o = physics->item[i];
		// write: address, parent, target pointer, identity, pos, angle, and vel
		fprintf(f, "0x%08p p:0x%08p t:0x%08p id:%s   x:%9.3f y:%9.3f a:%6.3f  vx:%6.4f vy:%6.4f\n",
			o, o->parent, o->target,
			o->get_identity(),
			o->pos.x, o->pos.y,
			o->angle,
			o->vel.x, o->vel.y);
	}
}

#if defined DO_STACKTRACE

#	define MAX_STACK_LEVELS 64
#	define MAX_STACK_LEVELS_MASK (MAX_STACK_LEVELS - 1)
#	define MAX_TRACE_LENGTH 64
#	define MAX_TRACE_LENGTH_MASK (MAX_TRACE_LENGTH - 1)


	static int _stack_level = 0;
	static int _trace_calls = 0;

	static SOURCE_LINE *_stack_data[MAX_STACK_LEVELS];
	static TraceData    _trace_data[MAX_TRACE_LENGTH];

	static void _usth_error() {
		tw_error("Exceeded maximum stacktrace level");
	}

	int get_stacktrace_data ( SOURCE_LINE **stack, int max ) {
		int n = max;
		n = n < _stack_level ? n : _stack_level;
		for (int i = 0; i < n; i++) stack[i] = _stack_data[i];
		return n;
	}

	int get_trace_data ( TraceData *trace, int max ) {
		int n = max;
		n = n < _trace_calls ? n : _trace_calls;
		n = n < MAX_TRACE_LENGTH ? n : MAX_TRACE_LENGTH;
		for (int i = 0; i < n; i++) {
			trace[i] = _trace_data[
				(_trace_calls - n + i) & MAX_TRACE_LENGTH_MASK
			];
		}
		return n;
	}

	UserStackTraceHelper::UserStackTraceHelper ( SOURCE_LINE *srcline ) {
		int i = (_trace_calls++) & MAX_TRACE_LENGTH_MASK;
		_trace_data[i].srcline = srcline;
		_trace_data[i].level = _stack_level;
		_stack_data[_stack_level++] = srcline;
		if (_stack_level >= MAX_STACK_LEVELS) _usth_error();
	}
	UserStackTraceHelper::~UserStackTraceHelper() {
		_stack_level --;
	}
	// why use a class: because it places a count-down automatically at the end
	// of the subroutine where you place the STACKTRACE.
	char *get_stack_trace_string(int max) {
		int l = 30;
		int i = 0;
		if (_stack_level < 0) return "\nError in stack trace\n";
//		if (_stacktrace_level > 256) return "\n
		if (max > MAX_STACK_LEVELS) max = MAX_STACK_LEVELS;
		if (max > _stack_level) max = _stack_level;
		for (i = 0; i < max; i += 1) {
			l += strlen(_stack_data[(_stack_level-i-1) & MAX_STACK_LEVELS_MASK]->file);
			l += 25;
		}
		char *buf = (char*)malloc(l);
		char *tmp = buf;
		tmp += sprintf(tmp, "\nStack Trace:\n");
		for (i = 0; i < max; i += 1) {
			if (i != 0) tmp += sprintf(tmp, "called from");
			const char *str = _stack_data[(_stack_level-i-1) & MAX_STACK_LEVELS_MASK]->file;
			const char *tmp2 = strstr(str, "source");
			if (tmp2 && tmp2[6]) str = tmp2 + 7;
			int line = _stack_data[(_stack_level-i-1) & MAX_STACK_LEVELS_MASK]->line;
			tmp += sprintf(tmp, "  %s : %d\n", str, line);
		}
		return buf;
	}
#endif



static DIALOG tw_alert_dialog1[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
  { d_button_proc,     250,  280,  160,  20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};
static DIALOG tw_alert_dialog2[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
  { d_button_proc,     190,  275,  125,  30,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_button_proc,     325,  275,  125,  30,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
};
static DIALOG tw_alert_dialog3[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
  { d_button_proc,     230,  280,  50,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_button_proc,     290,  280,  50,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_button_proc,     350,  280,  50,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};
static DIALOG tw_alert_dialog4[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        180,  170,  280,  140,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_textbox_proc,    185,  175,  270,  95,   255,  0,    0,    0,       80,   0,    NULL, NULL, NULL },
  { d_button_proc,     190,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_button_proc,     255,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_button_proc,     320,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_button_proc,     385,  280,  55,   20,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};
static DIALOG *tw_alert_dialogs[4] = { 
	tw_alert_dialog1, 
	tw_alert_dialog2, 
	tw_alert_dialog3, 
	tw_alert_dialog4
};
char find_shortcut_key(const char *s) {
	while (true) {
		s = strchr(s, '&');
		if (s && (s[1] != '&')) {
			s++;
			break;
		}
		if (!s) return 0;
	}
	return *s;
}

int _tw_alert ( 
	bool popup, 
	char *message, 
	const char *b1, 
	const char *b2 = NULL, 
	const char *b3 = NULL, 
	const char *b4 = NULL
) 
{
	char *s1 = message;
	char *s2 = NULL, *s3 = NULL;

	s2 = strchr(s1, '\n');
	if (s2) {
		s2[0] = 0;
		s2 += 1;
		s3 = strchr(s2, '\n');
		if (s3) {
			s3[0]= 0;
			s3 += 1;
		}
	}

	int l = 1;
	if (b2) l = 2;
	if (b3) l = 3;
	if (b4) l = 4;
	DIALOG *dialog = tw_alert_dialogs[l-1];
	dialog[1].dp = (void*)message;
	if (b1) {
		dialog[2].dp = (void*)b1;
		dialog[2].key = find_shortcut_key(b1);
	}
	if (b2) {
		dialog[3].dp = (void*)b2;
		dialog[3].key = find_shortcut_key(b2);
	}
	if (b3) {
		dialog[4].dp = (void*)b3;
		dialog[4].key = find_shortcut_key(b3);
	}
	if (b4) {
		dialog[5].dp = (void*)b4;
		dialog[5].key = find_shortcut_key(b4);
	}

	int i;

	if (popup) i = tw_popup_dialog(&videosystem.window, dialog, 2) - 2;
	else       i = tw_do_dialog   (&videosystem.window, dialog, 2) - 2;
	return i + 1;
	//return alert3(s1, s2, s3, b1, b2, b3, 0, 0, 0);
}


int tw_alert(const char *message, const char *b1, const char *b2, const char *b3, const char *b4) {
	char *s1 = strdup(message);
	int r = _tw_alert( true, s1, b1, b2, b3, b4 );
	free(s1);
	return r;
}


static void tw_error_handler (
	int in_catch_statement, 
	const char *file, 
	int line, 
	const char *message
) 
{
	char error_string[4096];
	int i;

	if (__error_flag & 2) return;

	char *cp = &error_string[0];
	if (file) {
		char *_file = strstr(file, "source");
		if (_file) file = _file;
	}

	log_debug("tw_error_handler invoked: ");
	int len = strlen(message);
	memcpy(error_string, message, len);
	cp += len;
	if (line >= 0) {//display line # and file name
		log_debug(        "(from %s, Line %d)\n", file, line);
		cp += sprintf(cp, "\n(from %s, line %d)\n", file, line);
	}
	else {
		log_debug("(from unspecified file & linenumber)\n");
		cp += sprintf(cp, "\n");
	}
	log_debug("begin error message:\n%s\nend error message\n", message);


#ifdef DO_STACKTRACE
	//there needs to be enough space for the whole trace...
	//if it gets printed into error_string
	enum {TRACE_LENGTH = 16};
	enum {MAX_STACK = 24};

	if (1) {//always displaying trace
		log_debug(        "Trace:\n");
		cp += sprintf(cp, "Trace:\n");
		TraceData trace[TRACE_LENGTH];
		int n = get_trace_data(&trace[0], TRACE_LENGTH);
		char blanks[MAX_STACK];
		int i;
		for (i = 0; i < 24; i++) blanks[i] = ' ';
		for (i = 0; i < n; i++) {

			int level = trace[i].level;
			if (level > MAX_STACK) {
				log_debug("maximum stack level exceeded? (stack level = %d\n", level);
				continue;
			}
			blanks[level] = 0;
			log_debug(        "%2d %2d %s", i, level, blanks);
			cp += sprintf(cp, "%2d %2d %s", i, level, blanks);
			blanks[level] = ' ';

			SOURCE_LINE *srcline = trace[i].srcline;
			if (srcline->name) {
				log_debug(        "%s, line %d (\"%s\")\n", srcline->file, srcline->line, srcline->name);
				cp += sprintf(cp, "%s, line %d (\"%s\")\n", srcline->file, srcline->line, srcline->name);
			}
			else {
				log_debug(        "%s, line %d\n", srcline->file, srcline->line);
				cp += sprintf(cp, "%s, line %d\n", srcline->file, srcline->line);
			}
		}
	}

	if (!in_catch_statement) {//display stack only if it's meaningfull
		log_debug(        "Stacktrace:\n");
		cp += sprintf(cp, "Stacktrace:\n");
		SOURCE_LINE *stack[MAX_STACK];
		int n = get_stacktrace_data(&stack[0], MAX_STACK);
		char blanks[MAX_STACK];
		int i;
		for (i = 0; i < 24; i++) blanks[i] = ' ';
		i = n - 8;
		i = i > 0 ? i : 0;
		for (; i < n; i++) {
			
			int level = i;
			blanks[level] = 0;
			log_debug(        "%2d %s", level, blanks);
			cp += sprintf(cp, "%2d %s", level, blanks);
			blanks[level] = ' ';

			if (stack[i]->name) {
				log_debug(        "%s, line %d (\"%s\")\n", stack[i]->file, stack[i]->line, stack[i]->name);
				cp += sprintf(cp, "%s, line %d (\"%s\")\n", stack[i]->file, stack[i]->line, stack[i]->name);
			}
			else {
				log_debug(        "%s, line %d\n", stack[i]->file, stack[i]->line);
				cp += sprintf(cp, "%s, line %d\n", stack[i]->file, stack[i]->line);
			}
		}
		//char *STstring = get_stack_trace_string(8);
		//len += sprintf(error_string + len, "\n\n%s", STstring);
		//free(STstring);
	}
#endif


	// dump the physics content to a file, called... uhh... something?
	dump_physics(error_string);

	if (videosystem.width <= 0) {
		allegro_message("Critical Error$: %s\n", error_string);
		log_debug("\nUnable to display messge, shutting down\n");
		throw -1;
	}

	log_debug("Pressenting graphical error prompt\n");

	const char *es[] = {"&Abort", "&Retry", "&Debug", "&Ignore"};
	enum {
		ES_ABORT = 0,
		ES_RETRY,
		ES_DEBUG,
		ES_IGNORE
	};
	int selection = -1;

	if ( in_catch_statement ) {
		int les[] = { ES_ABORT, ES_DEBUG };
		i = _tw_alert( false, error_string, es[les[0]], es[les[1]] );
		if (i > 0) selection = les[i - 1];
	}
	else {
		int les[] = { ES_ABORT, ES_RETRY, ES_DEBUG, ES_IGNORE };
		i = _tw_alert( false, error_string, es[les[0]], es[les[1]], es[les[2]], es[les[3]] );
		if (i > 0) selection = les[i - 1];
	}
	if (selection < 0) selection = ES_ABORT;
	log_debug("Option \"%s\" selected\n", es[selection]);

	videosystem.screen_corrupted = true;

	//xxx TWERROR

	if (selection == ES_DEBUG) {//"Debug"
		__error_flag |= 1;
		if (in_catch_statement) {
			return;
		}
#		if defined ALLEGRO_MSVC
			__asm int 3;
#		elif defined __GNUC__ && defined __i386__
			asm("int $0x03");
#		else
			if (1) (*((int*)NULL)) = 0;
#		endif
		return;
	}

	if (selection == ES_IGNORE) {//"Ignore"
		__error_flag |= 2;
		return;
	}

	if (selection == ES_RETRY) {//"Retry"
		return;
	}

	//"Abort"
	if (in_catch_statement) return;
	else throw 0;
}
static void _register_tw_error_hanlde() {_error_handler = &tw_error_handler;}
CALL_BEFORE_MAIN(_register_tw_error_hanlde);






void tw_error_exit(const char* message) {
	log_debug("\nCritical Error!: %s\n\n", message);
	if ((videosystem.width > 0) && (strlen(message) < 1000)) {
		char buf[1024];
		sprintf(buf, "Critical Error!: %s", message);
		tw_alert (buf, "Quit");
	}
	else
		allegro_message("Critical Error!: %s\n", message);

	tw_exit(1);
}

//const int tw_error_str_len = 2048;	// should be pleny of room.
//char tw_error_str[tw_error_str_len];

void caught_error(const char *format, ...) {
	char error_string[4096];
	if (format) {
		va_list those_dots;
		va_start(those_dots, format);
		vsprintf(error_string, format, those_dots);
		va_end(those_dots);
	}

#ifdef DO_STACKTRACE
	int l = strlen(error_string);
	char *STstring = get_stack_trace_string(8);
	if (l + strlen(STstring) < 4000) 
		sprintf(error_string + l, "\n\n%s", STstring);

	// added GEO:
	//strncpy(tw_error_str, STstring, tw_error_str_len-1);
	//tw_error_str[tw_error_str_len-1] = 0;
	// for later use. This needs to be stored, cause this is the only place where
	// the stacks lead to the bug location ?? Or not ??

	free(STstring);
#endif

	_error_handler( 1, NULL, -1, error_string);

	return;
}


#if defined(USE_ALLEGRO) && defined(DO_STACKTRACE)

volatile int _crash_detected = 0;
static void _crash_detector() {
	int i = get_time();
	if (_crash_detected) {
		if (videosystem.last_poll == -1) return;
		if (videosystem.last_poll > i - 200) _crash_detected -= 1;
		return;
	}
	if (videosystem.last_poll < i - 5000) {
		if (videosystem.last_poll == -1) return;
		_crash_detected = 30;
		char *STstring = get_stack_trace_string(16);
		log_debug("\nPossible Infinite Loop:\n%s", STstring);
		free(STstring);
	}
}
END_OF_STATIC_FUNCTION(_crash_detector)

void init_error() {
	int i;
	LOCK_VARIABLE(_crash_detected);
	i = install_int(_crash_detector, 1000);
	if (i) {
		log_debug("init_error() - failed to install crash detector");
		tw_error("init_error() - failed to install crash detector");
	}
}

void deinit_error() {
	remove_int(_crash_detector);
}

#else

void init_error() {
}
void deinit_error() {
}

#endif



