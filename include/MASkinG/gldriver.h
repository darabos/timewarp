// ************************************************************************** //
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
// ************************************************************************** //

#ifdef			MASKING_GL

#ifndef			MASKING_GLDRIVER_H
#define			MASKING_GLDRIVER_H

#include "drs.h"
#include <alleggl.h>

namespace MAS {
/**
	Simple OpenGL/AllegroGL driver.
	Draws to the screen and does an allegro_gl_flip() for every frame.
	This driver is most useful for applications that use OpenGL in a
	fullscreen context (i.e. games). It's the fastest OpenGL driver you'll
	get (for pure OpenGL graphics) but has a few drawbacks. Some Allegro
	drawing primitives are not supported and others are very slow. This
	means that for this	driver font antialiasing and mouse shadows have
	to be turned off (this is done automatically).
*/
class GLDriver : public ScreenUpdate {
	public:
		/**
			This driver requires full redraw.
		*/
		bool RequiresFullRedraw();

		/**
			Flips the buffers.
		*/
		void Redraw();

		char *GetDescription();
};

/**
	Simple OpenGL/AllegroGL driver for use with DRS.
	This driver is most suitable for applications that use OpenGL only in a
	small viewport of some sort (e.g. 3D editors). It may not be as fast as
	the normal GLDriver but allows you to use it alongside all Allegro
	gfx routines without problems.
*/
class GLDriverDRS : public DRS {
	public:
		/**
			Redraws the dirty rectangles and flips the buffers.
		*/
		void Redraw();

		char *GetDescription();
};

/**
	Simple software double buffered OpenGL/AllegroGL driver.
	This driver is a fallback driver for the DRS driver in fullscreen mode. It's
	a lot slower than the video double buffered drivers but as the DRS driver
	doesn't seem to work properly in fullscreen mode I see no other solution than
	to use this one.
*/
class GLDriverDoubleBuffer : public DoubleBuffer {
	public:
		/**
			Redraws the dirty rectangles and flips the buffers.
		*/
		void Redraw();

		char *GetDescription();
		bool RequiresFullRedraw();
};
}


#endif			//MASKING_GLDRIVER_H

#endif			//MASKING_GL
