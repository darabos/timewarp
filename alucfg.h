/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Configuration defines for use on Unix platforms.
 *
 *      By Michael Bukin.
 *
 *      See readme.txt for copyright information.
 */


#ifndef ALUCFG_H
#define ALUCFG_H

#include <fcntl.h>
#include <unistd.h>

/* Describe this platform.  */
#define ALLEGRO_PLATFORM_STR  "Unix"

#define ALLEGRO_CONSOLE_OK

#define ALLEGRO_EXTRA_HEADER "allegro/platform/alunix.h"
#define ALLEGRO_INTERNAL_HEADER "allegro/platform/aintunix.h"

#ifndef O_BINARY
# define O_BINARY  0
# define O_TEXT    0
#endif

/* These defines will be provided by configure script.  */
#undef ALLEGRO_COLOR8
#undef ALLEGRO_COLOR16
#undef ALLEGRO_COLOR24
#undef ALLEGRO_COLOR32

/* Include configuration generated by configure script.  */
#include "alunixac.h"

/* Provide implementations of missing functions.  */
#ifndef HAVE_STRICMP
#define ALLEGRO_NO_STRICMP
#endif

#ifndef HAVE_STRLWR
#define ALLEGRO_NO_STRLWR
#endif

#ifndef HAVE_STRUPR
#define ALLEGRO_NO_STRUPR
#endif

#ifndef HAVE_MEMCMP
#define ALLEGRO_NO_MEMCMP
#endif

#endif /* ifndef ALUCFG_H */

