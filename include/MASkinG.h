// ************************************************************************** //
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
// ************************************************************************** //

#ifndef			MASKING_H
#define			MASKING_H

#ifdef ALLEGRO_MSVC
#pragma warning (disable:4786,4788)
#endif

#include <alfont.h>

////////////////////////////////////////////////////////////////////////////////
// Version Info
////////////////////////////////////////////////////////////////////////////////

/**
	The major version number. For example 0 in v0.70.
*/
#define MASKING_VERSION			0
/**
	The minor version number. For example 70 in v0.70.
*/
#define MASKING_SUB_VERSION		76
/**
	The version string. For example "0.70" for v0.70.
*/
#define MASKING_VERSION_STR		"0.76b"
/**
	The release date. The format of the date is yyyymmdd. For example 20030806
	means August 6, 2003.
*/
#define MASKING_DATE			20040325

#include "MASkinG/defines.h"
#include "MASkinG/global.h"
#include "MASkinG/error.h"
#include "MASkinG/settings.h"
#include "MASkinG/color.h"
#include "MASkinG/point.h"
#include "MASkinG/size.h"
#include "MASkinG/rect.h"
#include "MASkinG/bitmap.h"
#include "MASkinG/sample.h"
#include "MASkinG/cursor.h"
#include "MASkinG/font.h"
#include "MASkinG/skin.h"
#include "MASkinG/screenupdate.h"
#include "MASkinG/doublebuffer.h"
#include "MASkinG/pageflipping.h"
#include "MASkinG/triplebuffer.h"
#include "MASkinG/drs.h"
#include "MASkinG/mouse.h"
#include "MASkinG/timer.h"
#include "MASkinG/widget.h"
#include "MASkinG/dialog.h"
#include "MASkinG/animator.h"
#include "MASkinG/clearscreen.h"
#include "MASkinG/label.h"
#include "MASkinG/fps.h"
#include "MASkinG/image.h"
#include "MASkinG/wallpaper.h"
#include "MASkinG/button.h"
#include "MASkinG/hypertext.h"
#include "MASkinG/checkbox.h"
#include "MASkinG/radiobutton.h"
#include "MASkinG/panel.h"
#include "MASkinG/groupbox.h"
#include "MASkinG/editbox.h"
#include "MASkinG/scrollobj.h"
#include "MASkinG/spinbox.h"
#include "MASkinG/separator.h"
#include "MASkinG/progress.h"
#include "MASkinG/slider.h"
#include "MASkinG/scroller.h"
#include "MASkinG/listbox.h"
#include "MASkinG/shortcut.h"
#include "MASkinG/combobox.h"
#include "MASkinG/window.h"
#include "MASkinG/menu.h"
#include "MASkinG/messagebox.h"
#include "MASkinG/fileselect.h"
#include "MASkinG/colorsel.h"
#include "MASkinG/scrollbox.h"
#include "MASkinG/textarea.h"
#include "MASkinG/tooltip.h"
#include "MASkinG/accelerator.h"
#include "MASkinG/tabpanel.h"
#include "MASkinG/splitter.h"
#include "MASkinG/gldriver.h"
#include "MASkinG/glviewport.h"
#include "MASkinG/gldialog.h"

#ifdef	MessageBox
#undef	MessageBox
#endif

#ifdef	SendMessage
#undef	SendMessage
#endif

#endif		//MASKING_H
