////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#ifdef		MASKING_GL

#include "../include/MASkinG/gldriver.h"


bool MAS::GLDriver::RequiresFullRedraw() {
	return true;
}


char *MAS::GLDriver::GetDescription() {
	return "default OpenGL";
}


void MAS::GLDriver::Redraw() {
	allegro_gl_flip();
}


void MAS::GLDriverDRS::Redraw() {
	DRS::Redraw();
	allegro_gl_flip();
}


char *MAS::GLDriverDRS::GetDescription() {
	return "OpenGL with DRS";
}


void MAS::GLDriverDoubleBuffer::Redraw() {
	blit(canvas, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	allegro_gl_flip();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}


char *MAS::GLDriverDoubleBuffer::GetDescription() {
	return "software double buffered OpenGL";
}


bool MAS::GLDriverDoubleBuffer::RequiresFullRedraw() {
	return false;
}

#endif		//MASKING_GL
