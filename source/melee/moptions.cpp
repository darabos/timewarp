#include <allegro.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../scp.h"
#include "../gui.h"

#include "../util/aastr.h"

/*

-	master menu
		client.ini
-			video mode
				exit (exit menu)
				apply (use settings)
				make default (use settings and save)
				screen resolution
				bits per pixel
				fullscreen
				gamma correction?
				custom color filter settings?
-			audio mode
				done (use settings, save, and exit menu)
				cancel (exit menu)
				sound disable
				sound volume
				sound channels?
				music disable
				music volume
				music channels?
				game specific settings?
-			other
-				keyboard configuration
				star depth
				star density
				antialiasing mode
				raw backup images
				alpha blending?
				mouse sensitivity?
-		server.ini
			tic rate
			friendly fire
			shot relativity
			map width
			map height
-		turbo
-		f4turbo

*/

enum {
	DIALOG_OPTIONS_BOX = 0,
	DIALOG_OPTIONS_DONE,
	DIALOG_OPTIONS_VIDEO,
	DIALOG_OPTIONS_AUDIO,
	DIALOG_OPTIONS_CONFIG,
	DIALOG_OPTIONS_PHYSICS,
	DIALOG_OPTIONS_DEFAULT
	};
DIALOG options_dialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg) (bg) (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        40,    30,  190,  270,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_button_proc,     70,    40,  110,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Done", NULL, NULL },
  { d_button_proc,     50,    90,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Video Mode", NULL, NULL },
  { d_button_proc,     50,   130,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Audio Settings", NULL, NULL },
  { d_button_proc,     50,   170,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Game && Rendering", NULL, NULL },
//  { d_button_proc,     50,   210,  170,   30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Physics Settings", NULL, NULL },
//  { d_button_proc,     50,   250,  170,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Restore Defaults", NULL, NULL },
  { d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,     0,    0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
	};

void options_menu (Game *game) {STACKTRACE
	int a;
	while (true) {
		a = tw_popup_dialog(NULL, options_dialog, 0);
		switch (a) {
			default: 
			case DIALOG_OPTIONS_DONE: {
				return;
				}
			break;
			case DIALOG_OPTIONS_VIDEO: {
				video_menu(game);
				}
			break;
			case DIALOG_OPTIONS_AUDIO: {
				audio_menu(game);
				}
			break;
			case DIALOG_OPTIONS_CONFIG: {
				config_menu(game);
				}
			break;
			case DIALOG_OPTIONS_PHYSICS: {
				physics_menu(game);
				}
			break;
			case DIALOG_OPTIONS_DEFAULT: {
				}
			break;
			}
		}
	return;
	}



char *resolution[] = { 
	"320x200", "640x480", "800x600", "1024x768", "1280x1024", "Custom", NULL
	};
char *color_depth[] = { 
	"8", "15", "16", "24", "32", NULL
	};
enum {
	DIALOG_VIDEO_BOX = 0,
	DIALOG_VIDEO_FULLSCREEN,
	DIALOG_VIDEO_CUSTOM_TEXT,
	DIALOG_VIDEO_CUSTOM_BOX,
	DIALOG_VIDEO_CUSTOM_EDIT,
	DIALOG_VIDEO_RESTEXT,
	DIALOG_VIDEO_RESLIST,
	DIALOG_VIDEO_BPPTEXT,
	DIALOG_VIDEO_BPPLIST,
	DIALOG_VIDEO_EXIT,
	DIALOG_VIDEO_APPLY,
	DIALOG_VIDEO_GET_DEFAULT,
	DIALOG_VIDEO_SET_DEFAULT,
	DIALOG_VIDEO_GAMMA_TEXT,
	DIALOG_VIDEO_GAMMA_SLIDER,
	};
DIALOG video_dialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg) (bg) (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        20,   20,  400, 400,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },

  { d_check_proc,      190,  120,  160,  30,   255,  0,    0,    0,       0,    0,    (void *)"Full-screen ", NULL, NULL },

  { d_text_proc,       190,  180, 160,  30,   255,  0,    0,    0,       0,    0,    (void *)"Custom", NULL, NULL },
  { d_box_proc,        188,  208, 164,  34,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_edit_proc,       190,  210, 150,  30,   255,  0,    0,    0,       75,   0,    (void *) dialog_string[3], NULL, NULL },

  { d_text_proc,       30,  120,  140,  30,   255,  0,    0,    0,       0,    0,    (void *)"Resolution", NULL, NULL },
  { d_list_proc2,      30,  145,  140, 115,   255,  0,    0,    0,       0,    0,    (void *) genericListboxGetter, NULL, resolution },
  { d_text_proc,       30,  290,  100,  30,   255,  0,    0,    0,       0,    0,    (void *)"Color Depth", NULL, NULL },
  { d_list_proc2,      30,  310,  100, 100,   255,  0,    0,    0,       0,    0,    (void *) genericListboxGetter, NULL, color_depth },

  { d_button_proc,      32,  30,  100,   35,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Exit", NULL, NULL },
  { d_button_proc,      32,  70,  100,   35,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Apply", NULL, NULL },
  { d_button_proc,     143,  30,  260,  35,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Restore Default", NULL, NULL },
  { d_button_proc,     143,  70,  260,  35,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Set Default", NULL, NULL },

  { d_text_proc,       170,  310,  160,  20,   255,  0,    0,    0,       0,    0,    (void *)"Gamma Correction", NULL, NULL },
  { d_slider_proc,     170,  330,  160,  15,   255,  0,    0,    0,       255,  0,    NULL, NULL, NULL },
  { d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,   0,    0,    0,    255,  0,    0,    0,       3,    0,    NULL, NULL, NULL }
	};


void video_menu (Game *game) {STACKTRACE
	int choice = -1;
	while (choice != DIALOG_VIDEO_EXIT) {
		sprintf(dialog_string[3], "%dx%d", videosystem.width, videosystem.height);

		//set index for resolution
		int x, y, x2, y2, i, bpp, bpp2, fs;
		x2 = videosystem.width;
		y2 = videosystem.height;
		for (i = 0; resolution[i+1]; i += 1) {
			x = strtol(resolution[i], NULL, 10);
			y = strtol(strchr(resolution[i], 'x') + 1, NULL, 10);
			if ((x == x2) && (y == y2)) break;
			}
		video_dialog[DIALOG_VIDEO_RESLIST].d1 = i;

		//set index for bpp
		bpp = videosystem.bpp;
		for (i = 0; true; i += 1) {
			if (!color_depth[i]) tw_error("video_menu - current bpp invalid?");
			if (strtol(color_depth[i], NULL, 10) == bpp) break;
			}
		video_dialog[DIALOG_VIDEO_BPPLIST].d1 = i;

		//set button for fullscreen
		video_dialog[DIALOG_VIDEO_FULLSCREEN].flags = videosystem.fullscreen ? D_SELECTED : 0;

		//set gamma correction
		video_dialog[DIALOG_VIDEO_GAMMA_SLIDER].d2 = get_gamma();

		//do the dialog
		choice = tw_popup_dialog(NULL, video_dialog, 0);
		if (choice == -1) choice = DIALOG_VIDEO_EXIT;

		//set resolution
		i = video_dialog[DIALOG_VIDEO_RESLIST].d1;
		char *tmp = resolution[i];
		if (!resolution[i+1]) tmp = dialog_string[3];
		x2 = strtol(tmp, NULL, 10);
		y2 = strtol(strchr(tmp, 'x') + 1, NULL, 10);

		//set bpp from menu
		i = video_dialog[DIALOG_VIDEO_BPPLIST].d1;
		bpp2 = strtol(color_depth[i], NULL, 10);

		//set fullscreen from menu
		fs = video_dialog[DIALOG_VIDEO_FULLSCREEN].flags & D_SELECTED;

		if (choice == DIALOG_VIDEO_GET_DEFAULT) {
			set_config_file("client.ini");
			bpp2   = get_config_int("Video", "BitsPerPixel", 16);
			x2     = get_config_int("Video", "ScreenWidth", 640);
			y2     = get_config_int("Video", "ScreenHeight", 480);
			fs     = get_config_int("Video", "FullScreen", false);
			set_gamma(get_config_int("Video", "Gamma", 128));
			choice = DIALOG_VIDEO_APPLY;
		}
		if ((choice == DIALOG_VIDEO_APPLY) | (choice == DIALOG_VIDEO_SET_DEFAULT)) {
			if ((bpp2 != bpp) && game) {
				tw_alert ("Color depths cannot be changed in\nthe middle of a game\nin this version", "Okay");
				}
			else {
				set_gamma(video_dialog[DIALOG_VIDEO_GAMMA_SLIDER].d2);
				i = videosystem.set_resolution(x2, y2, bpp2, fs);
				if (i && (choice == DIALOG_VIDEO_SET_DEFAULT)) {
					set_config_file("client.ini");
					set_config_int("Video", "BitsPerPixel", bpp2);
					set_config_int("Video", "ScreenWidth", x2);
					set_config_int("Video", "ScreenHeight", y2);
					set_config_int("Video", "FullScreen", fs);
					set_config_int("Video", "Gamma", get_gamma());
				}
			}
		}
	}
	return;
}

enum {
	DIALOG_AUDIO_BOX = 0,
	DIALOG_AUDIO_OK,
	DIALOG_AUDIO_CANCEL,
	DIALOG_AUDIO_SOUND_ON,
	DIALOG_AUDIO_SOUND_VOL,
	DIALOG_AUDIO_MUSIC_ON,
	DIALOG_AUDIO_MUSIC_VOL
	};
DIALOG audio_dialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg) (bg) (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        30,   50,  410, 140,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_button_proc,     100,  60,  80,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"OK", NULL, NULL },
  { d_button_proc,     200,  60,  80,   40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Cancel", NULL, NULL },
  { d_check_proc,      40,  110,  160,  20,   255,  0,    0,    0,       0,    0,    (void *)"Sound Volume ", NULL, NULL },  
  { d_slider_proc,     205, 110,  180,  15,   255,  0,    0,    0,       255,  0,    NULL, NULL, NULL },
  { d_check_proc,      40,  140,  160,  20,   255,  0,    0,    0,       0,    0,    (void *)"Music Volume ", NULL, NULL },  
  { d_slider_proc,     205, 140,  180,  15,   255,  0,    0,    0,       255,  0,    NULL, NULL, NULL },
  { d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
	};

void audio_menu (Game *game) {STACKTRACE
	int i;

	//set dialog values
	audio_dialog[DIALOG_AUDIO_SOUND_ON].flags = sound.sound_on ? D_SELECTED : 0;
	audio_dialog[DIALOG_AUDIO_MUSIC_ON].flags = sound.music_on ? D_SELECTED : 0;
	audio_dialog[DIALOG_AUDIO_SOUND_VOL].d2 = sound.sound_volume;
	audio_dialog[DIALOG_AUDIO_MUSIC_VOL].d2 = sound.music_volume;

	//do the dialog
	i = tw_popup_dialog(NULL, audio_dialog, 0);
	if (i != DIALOG_AUDIO_OK) return;

	//set actual values
	sound.set_volumes(
		audio_dialog[DIALOG_AUDIO_SOUND_VOL].d2,
		audio_dialog[DIALOG_AUDIO_MUSIC_VOL].d2,
		audio_dialog[DIALOG_AUDIO_SOUND_ON].flags & D_SELECTED,
		audio_dialog[DIALOG_AUDIO_MUSIC_ON].flags & D_SELECTED
		);
	set_config_file("client.ini");
	sound.save();
	};






#include "mview.h"
#include "mgame.h"

char *viewListboxGetter(int index, int *list_size) 
{
	static char tmp[40];
	tmp[0] = 0;
	if(index < 0) {
		*list_size = num_views;
		return NULL;
	} else {
		return(view_name[index]);
	}
}

enum {
OPTIONS_DIALOG_BOX = 0,
OPTIONS_DIALOG_STARS_TEXT,
OPTIONS_DIALOG_STARS_SLIDER,
OPTIONS_DIALOG_RELATIVITY_TEXT,
OPTIONS_DIALOG_RELATIVITY_SLIDER,
OPTIONS_DIALOG_FRIENDLY_FIRE,
//OPTIONS_DIALOG_TURBO_TEXT
//OPTIONS_DIALOG_TURBO_SLIDER
//OPTIONS_DIALOG_TURBOF4_TEXT
//OPTIONS_DIALOG_TURBOF4_SLIDER
//OPTIONS_DIALOG_TICRATE_TEXT
//OPTIONS_DIALOG_TICRATE
//OPTIONS_DIALOG_MAPSIZE_TEXT
//OPTIONS_DIALOG_MAPSIZE
//OPTIONS_DIALOG_AA_TEXT,
//OPTIONS_DIALOG_AA,
OPTIONS_DIALOG_VIEW_TEXT,
OPTIONS_DIALOG_VIEW,
OPTIONS_DIALOG_OK,
OPTIONS_DIALOG_CANCEL,

OPTIONS_DIALOG_QUALITY_TEXT,
OPTIONS_DIALOG_INTERPOLATION_ON,
OPTIONS_DIALOG_AA_ON,
OPTIONS_DIALOG_NOALIGN_ON,
OPTIONS_DIALOG_BLEND_ON,
OPTIONS_DIALOG_ALPHA_ON,

OPTIONS_DIALOG_END
};

DIALOG old_optionsDialog[] =
{
   /* (proc)          (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)                        (dp2) (dp3)          */
   { d_box_proc,      28,  40,  460, 325, 255, 0,   0,    0,      0,   0,   NULL,                       NULL, NULL          },
   { d_text_proc,     40,  56,  160, 20,  255, 0,   0,    0,      0,   0,   (void *)"Star Depth",       NULL, NULL          },
   { d_slider_proc,   212, 56,  160, 16,  255, 0,   0,    0,      255, 0,   NULL,                       NULL, NULL          },
   { d_text_proc,     40,  94,  160, 20,  255, 0,   0,    0,      0,   0,   (void*)"Shot Relativity",   NULL, NULL          },
   { d_slider_proc,   212, 96,  160, 16,  255, 0,   0,    0,      1000,0,   NULL,                       NULL, NULL          },
   { d_check_proc,    60,  144, 184, 20,  255, 0,   0,    0,      1,   0,   (void*)"Friendly Fire",     NULL, NULL          },
   { d_text_proc,     292, 244, 120, 20,  255, 0,   0,    0,      0,   0,   (void *)"View",             NULL, NULL          },
   { d_list_proc,     284, 264, 180, 90,  255, 0,   0,    0,      0,   0,   (void *) viewListboxGetter, NULL, NULL          },
   { d_button_proc,   400, 60,  80,  40,  255, 0,   0,    D_EXIT, 0,   0,   (void *)"OK",               NULL, NULL          },
   { d_button_proc,   400, 116, 80,  40,  255, 0,   0,    D_EXIT, 0,   0,   (void *)"Cancel",           NULL, NULL          },

   { d_text_proc,      40, 216, 120, 20,  255, 0,   0,    0,      1,   0,   (void *)"Rendering Quality:", NULL, NULL          },

   { d_check_proc,     40, 236, 120, 20,  255, 0,   0,    0,      1,   0,   (void *)"Interpolation",    NULL, NULL          },
   { d_check_proc,     40, 260, 120, 20,  255, 0,   0,    0,      1,   0,   (void *)"Anti-Aliasing",    NULL, NULL          },
   { d_check_proc,     40, 284, 120, 20,  255, 0,   0,    0,      1,   0,   (void *)"AA:Non-integer",   NULL, NULL          },
   { d_check_proc,     40, 308, 120, 20,  255, 0,   0,    0,      1,   0,   (void *)"AA:Blend",         NULL, NULL          },
   { d_check_proc,     40, 332, 120, 20,  255, 0,   0,    0,      1,   0,   (void *)"AA:Alpha",         NULL, NULL          },

   { d_tw_yield_proc, 0,   0,   0,   0,   255, 0,   0,    0,      0,   0,   NULL,                       NULL, NULL          },
   { NULL,            0,   0,   0,   0,   0,   0,   0,    0,      0,   0,   NULL,                       NULL, NULL          }
};

void change_options() {STACKTRACE
	int optionsRet, i;

	set_config_file("client.ini");
//	old_optionsDialog[OPTIONS_DIALOG_AA].d1 = get_aa_mode();

	old_optionsDialog[OPTIONS_DIALOG_INTERPOLATION_ON].flags = 
		interpolate_frames ? D_SELECTED : 0;
	int aa = get_config_int("Rendering", "AA_Mode", AA_NO_AA);
	old_optionsDialog[OPTIONS_DIALOG_AA_ON].flags = 
		(aa&AA_NO_AA) ? 0 : D_SELECTED;
	old_optionsDialog[OPTIONS_DIALOG_NOALIGN_ON].flags = 
		(aa&AA_NO_ALIGN) ? D_SELECTED : 0;
	old_optionsDialog[OPTIONS_DIALOG_BLEND_ON].flags = 
		(aa&AA_BLEND) ? D_SELECTED : 0;
	old_optionsDialog[OPTIONS_DIALOG_ALPHA_ON].flags = 
		(aa&AA_ALPHA) ? D_SELECTED : 0;


	set_config_file("server.ini");
	old_optionsDialog[OPTIONS_DIALOG_STARS_SLIDER].d2 = 
		get_config_int("Stars", "Depth", 192);
	old_optionsDialog[OPTIONS_DIALOG_RELATIVITY_SLIDER].d2 = 
		iround(get_config_float("Game", "ShotRelativity", 0.5) * 1000);
	if (get_config_int("Game", "FriendlyFire", 1)) 
		old_optionsDialog[OPTIONS_DIALOG_FRIENDLY_FIRE].flags = D_SELECTED;
	else
		old_optionsDialog[OPTIONS_DIALOG_FRIENDLY_FIRE].flags = 0;


	set_config_file("client.ini");
	i = get_view_num ( get_config_string ( "View", "View", NULL ) );
	if (i == -1) i = 0;
	old_optionsDialog[OPTIONS_DIALOG_VIEW].d1 = i;

 
	optionsRet = tw_popup_dialog(NULL, old_optionsDialog, OPTIONS_DIALOG_OK);

	if (optionsRet == OPTIONS_DIALOG_CANCEL) return;

//	set_aa_mode(old_optionsDialog[OPTIONS_DIALOG_AA].d1);

	set_config_file("client.ini");
//	set_config_int("View", "Anti-Aliasing", get_aa_mode());
	interpolate_frames = 
		(old_optionsDialog[OPTIONS_DIALOG_INTERPOLATION_ON].flags & D_SELECTED) ? 1 : 0;
	set_config_int("View", "InterpolateFrames", interpolate_frames);
	aa&=~AA_NO_AA;aa|= (old_optionsDialog[OPTIONS_DIALOG_AA_ON].flags) ? 
		0 : AA_NO_AA;
	aa&=~AA_NO_ALIGN;aa|= (old_optionsDialog[OPTIONS_DIALOG_NOALIGN_ON].flags) ? 
		AA_NO_ALIGN : 0;
	aa&=~AA_BLEND;aa|= (old_optionsDialog[OPTIONS_DIALOG_BLEND_ON].flags) ? 
		AA_BLEND : 0;
	aa&=~AA_ALPHA;aa|= (old_optionsDialog[OPTIONS_DIALOG_ALPHA_ON].flags) ? 
		AA_ALPHA : 0;
	set_config_int("Rendering", "AA_Mode", aa);
	set_tw_aa_mode(aa);

	View *v = get_view(
		view_name[old_optionsDialog[OPTIONS_DIALOG_VIEW].d1],
		NULL
		);
	set_view(v);
	twconfig_set_string("/cfg/client.ini/view/view", 
		view_name[old_optionsDialog[OPTIONS_DIALOG_VIEW].d1]);
	if (game && !game->view_locked) game->change_view(
		view_name[old_optionsDialog[OPTIONS_DIALOG_VIEW].d1]);


	twconfig_set_int("/cfg/server.ini/stars/depth", 
		old_optionsDialog[OPTIONS_DIALOG_STARS_SLIDER].d2);

	twconfig_set_float("/cfg/server.ini/game/shotrelativity", 
		old_optionsDialog[OPTIONS_DIALOG_RELATIVITY_SLIDER].d2 / 1000.0);


	if (old_optionsDialog[OPTIONS_DIALOG_FRIENDLY_FIRE].flags & D_SELECTED) 
		i = 1;
	else i = 0;
	twconfig_set_int("/cfg/server.ini/game/friendlyfire", i);

	return;
	}

void config_menu (Game *game) {STACKTRACE
	change_options();
	};
void physics_menu (Game *game) {STACKTRACE
	};


