#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE

#if defined DO_STACKTRACE
	struct StackTraceData {
		const char *file;
		int line;
	};
	#define MAX_STACKTRACE_LEVELS 64
	#define MAX_STACKTRACE_LEVELS_MASK (MAX_STACKTRACE_LEVELS-1)
	int _stacktrace_level = 0;
	static StackTraceData _stacktrace_data[MAX_STACKTRACE_LEVELS];
	UserStackTraceHelper::UserStackTraceHelper(const char *file, int line) {
		_stacktrace_data[_stacktrace_level & MAX_STACKTRACE_LEVELS_MASK].file = file;
		_stacktrace_data[_stacktrace_level & MAX_STACKTRACE_LEVELS_MASK].line = line;
		_stacktrace_level += 1;
	}
	UserStackTraceHelper::~UserStackTraceHelper() {
		_stacktrace_level -= 1;
	}
	char *get_stack_trace_string(int max) {
		int l = 30;
		int i = 0;
		if (_stacktrace_level < 0) return "\nError in stack trace\n";
//		if (_stacktrace_level > 256) return "\n
		if (max > MAX_STACKTRACE_LEVELS) max = MAX_STACKTRACE_LEVELS;
		if (max > _stacktrace_level) max = _stacktrace_level;
		for (i = 0; i < max; i += 1) {
			l += strlen(_stacktrace_data[(_stacktrace_level-i-1) & MAX_STACKTRACE_LEVELS_MASK].file);
			l += 25;
		}
		char *buf = (char*)malloc(l);
		char *tmp = buf;
		tmp += sprintf(tmp, "\nStack Trace:\n");
		for (i = 0; i < max; i += 1) {
			if (i != 0) tmp += sprintf(tmp, "called from");
			const char *str = _stacktrace_data[(_stacktrace_level-i-1) & MAX_STACKTRACE_LEVELS_MASK].file;
			const char *tmp2 = strstr(str, "source");
			if (tmp2 && tmp2[6]) str = tmp2 + 7;
			int line = _stacktrace_data[(_stacktrace_level-i-1) & MAX_STACKTRACE_LEVELS_MASK].line;
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
int tw_alert(const char *message, const char *b1, const char *b2, const char *b3, const char *b4) {
	char *s1 = strdup(message);
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

	int i = tw_popup_dialog(&videosystem.window, dialog, 2) - 2;
	return i + 1;
	//return alert3(s1, s2, s3, b1, b2, b3, 0, 0, 0);
}


static void tw_error_handler (const char *file, int line, const char *message) {
	char error_string[4096];
	int i;

	if (!strncmp(message, "quit", 4)) throw 0;
	if (!strncmp(message, "Quit", 4)) throw 0;
	if (!strncmp(message, "QUIT", 4)) throw 0;

	if (__error_flag & 2) return;

	strcpy(error_string, message);
	if (line >= 0) {
		sprintf(error_string + strlen(message), 
				"\n\n%s, Line %d", file, line);
	}
#ifdef DO_STACKTRACE
	char *STstring = get_stack_trace_string(8);
	sprintf(error_string + strlen(message), "\n\n%s", STstring);
	free(STstring);
#endif

	if (videosystem.width <= 0) {
		allegro_message("Critical Error$: %s\n", error_string);
		throw -1;
	}

	i = tw_alert(error_string, "&Abort", "&Retry", "&Debug", "&Ignore");
	if (i == 3) {
		__error_flag |= 1;
#		if defined ALLEGRO_MSVC
			__asm int 3;
#		elif defined __GNUC__ && defined __I386__
			asm("int $0x03");
#		else
			if (1) (*((int*)NULL)) = 0;
#		endif
		return;
	}
	if (i == 4) {
		__error_flag |= 2;
		return;
	}
	if (i == 2)
		return;
	throw 0;
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
	free(STstring);
#endif
	int i = tw_alert(error_string, "Okay", "Debug");
	if (i == 2) __error_flag |= 1;

	return;
}


#if defined(USE_ALLEGRO) && defined(DO_STACKTRACE)

volatile int _crash_detected = 0;
static void _crash_detector() {
	if (_crash_detected) return;
	int i = get_time();
	if (videosystem.last_poll < i - 2500) {
		if (videosystem.last_poll == -1) return;
		_crash_detected = 1;
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


#endif


