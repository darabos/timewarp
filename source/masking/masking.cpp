////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG.h"
#include <allegro/internal/aintern.h>

#ifdef	MASKING_GL
#include <alleggl.h>
#endif

static bool oldAntialiasing = true;
static bool oldMouseShadow = true;

////////////////////////////////////////////////////////////////////////////////
// Installs the MASkinG library by setting up Allegro and Allegro Font,
// reseting the skin and optionally loading a user skin from disk
MAS::Error MAS::InstallMASkinG(const char *file) {
	MAS::Error err = MAS::Error(MAS::Error::NONE);
	if (_allegro_count <= 0) {
		if (allegro_init() != 0) {
			return MAS::Error(MAS::Error::ALLEGRO);
		}
	}
	if (file) MAS::Settings::Load(file);

	oldAntialiasing = MAS::Settings::antialiasing;
	oldMouseShadow = MAS::Settings::mouseShadow;

#ifdef	MASKING_GL
	if (MAS::Settings::gfxMode == GFX_OPENGL || MAS::Settings::gfxMode == GFX_OPENGL_WINDOWED || MAS::Settings::gfxMode == GFX_OPENGL_FULLSCREEN) {
		install_allegro_gl();
		if (MAS::Settings::screenUpdateMethod == -1) {
			MAS::Settings::antialiasing = false;
			MAS::Settings::mouseShadow = false;
		}
	}
#endif	//MASKING_GL
	
	if (install_keyboard() != 0) return MAS::Error(MAS::Error::KEYBOARD);
	if (install_timer() != 0) return MAS::Error(MAS::Error::TIMER);
	Timer::Lock();
	if (install_mouse() < 0) return MAS::Error(MAS::Error::MOUSE);
	if (install_sound(MAS::Settings::soundDriver, MAS::Settings::midiDriver, NULL) != 0) err = MAS::Error(MAS::Error::SOUND);
		
	if (MAS::Settings::keyDelay != -1 && MAS::Settings::keyRepeat != -1) {
		set_keyboard_rate(MAS::Settings::keyDelay, MAS::Settings::keyRepeat);
	}
	
	set_mouse_speed(MAS::Settings::mouseSpeed, MAS::Settings::mouseSpeed);
	request_refresh_rate(MAS::Settings::refreshRate);
	
	err = ChangeResolution(MAS::Settings::gfxMode, MAS::Settings::fullscreen, MAS::Settings::screenWidth, MAS::Settings::screenHeight, MAS::Settings::colorDepth, NULL);
	if (err) {
		return err;
	}
	
	if (alfont_init() != ALFONT_OK) {
		return MAS::Error(MAS::Error::ALFONT);
	}

	theSkin = new MAS::Skin((char *)MAS::Settings::skinPath);
	if (!theSkin) {
		return MAS::Error(MAS::Error::MEMORY);
	}
	else if (theSkin->GetError()) {
		return theSkin->GetError();
	}
	
	return err;
}


MAS::Error MAS::ChangeResolution(int gfxMode, bool fullscreen, int w, int h, int bpp, MAS::Dialog *mainDlg) {
	int mode = gfxMode;
	if (mode <= 2) {
		mode = fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED;
	}

	if (bpp <= 0) {
		if (desktop_color_depth() > 0) {
			bpp = desktop_color_depth();
		}
		else {
			bpp = 16;
		}
	}
	
	set_color_depth(bpp);
	bool mouseShadow = MAS::Settings::mouseShadow;
	if (bpp == 8) {
		mouseShadow = false;
	}

#ifdef	MASKING_GL
	allegro_gl_clear_settings();
	allegro_gl_set (AGL_Z_DEPTH, 16);
	allegro_gl_set (AGL_COLOR_DEPTH, bpp);
	allegro_gl_set (AGL_DOUBLEBUFFER, 1);
	allegro_gl_set (AGL_WINDOWED, fullscreen ? FALSE : TRUE);	
	allegro_gl_set (AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_Z_DEPTH | AGL_DOUBLEBUFFER
			| AGL_WINDOWED);
#endif

	if (set_gfx_mode(mode, w, h, 0, 0) != 0) {
		int alternative;
		switch (bpp) {
			case 8:		alternative = 0;	break;
			case 15:	alternative = 16;	break;
			case 16:	alternative = 15;	break;
			case 24:	alternative = 32;	break;
			case 32:	alternative = 24;	break;
			default:	alternative = 16;	break;
		}
		set_color_depth(alternative);

#ifdef	MASKING_GL
		allegro_gl_clear_settings();
		allegro_gl_set (AGL_Z_DEPTH, 16);
		allegro_gl_set (AGL_COLOR_DEPTH, bpp);
		allegro_gl_set (AGL_DOUBLEBUFFER, 1);
		allegro_gl_set (AGL_WINDOWED, fullscreen ? FALSE : TRUE);	
		allegro_gl_set (AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_Z_DEPTH | AGL_DOUBLEBUFFER
				| AGL_WINDOWED);
#endif

		if (set_gfx_mode(mode, w, h, 0, 0) != 0) {
			// can't change gfx mode -> return to previous settings if there are any
			if (mainDlg) {
				set_color_depth(MAS::Settings::colorDepth);
#ifdef	MASKING_GL
				allegro_gl_clear_settings();
				allegro_gl_set (AGL_Z_DEPTH, 16);
				allegro_gl_set (AGL_COLOR_DEPTH, MAS::Settings::colorDepth);
				allegro_gl_set (AGL_DOUBLEBUFFER, 1);
				allegro_gl_set (AGL_WINDOWED, fullscreen ? FALSE : TRUE);	
				allegro_gl_set (AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_Z_DEPTH | AGL_DOUBLEBUFFER
						| AGL_WINDOWED);
#endif
				set_gfx_mode(MAS::Settings::gfxMode, MAS::Settings::screenWidth, MAS::Settings::screenHeight, 0, 0);
				mainDlg->GetDriver()->Create();
				mainDlg->Redraw();
			}
			return MAS::Error(MAS::Error::GFX);
		}
		
		bpp = alternative;
	}
	
	MAS::Settings::gfxMode = mode;
	MAS::Settings::screenWidth = w;
	MAS::Settings::screenHeight = h;
	MAS::Settings::colorDepth = bpp;
	MAS::Settings::mouseShadow = mouseShadow;
	
	MAS::Color::OnColorDepthChange();

#ifndef		MASKING_GL
	if (MAS::Settings::useVideoMemory) {
		if (screen) destroy_bitmap(screen);
		screen = create_video_bitmap(SCREEN_W, SCREEN_H);
		show_video_bitmap(screen);
	}
#endif
	
	if (mainDlg) {
		mainDlg->GetDriver()->Create();
		mainDlg->Resize(SCREEN_W, SCREEN_H);
		mainDlg->GetSkin()->Reload();
		mainDlg->SetSkin(mainDlg->GetSkin());
	}

	if (MAS::Settings::runInBackground) {
		set_display_switch_mode(SWITCH_BACKGROUND);
	}
	
	return MAS::Error(MAS::Error::NONE);
}


void MAS::ExitMASkinG() {
	if (theSkin) {
		delete theSkin;
		theSkin = NULL;
	}

	alfont_exit();

	MAS::Settings::antialiasing = oldAntialiasing;
	MAS::Settings::mouseShadow = oldMouseShadow;
	MAS::Settings::Save();
}
