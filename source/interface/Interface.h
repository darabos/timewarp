/** 
Interface.h
$Id$

  Some common functions and definitions for handling the menu interface.  In particular, 
  stuff related to drawing the background when the menus are shown.
  
  Revision history:
    2004.06.16 yb started
	
  This file is part of "Star Control: TimeWarp" 
  http://timewarp.sourceforge.net/
  Copyright (C) 2001-2004  TimeWarp development team
	  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
		
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifndef INTERFACE_HEADER
#define INTERFACE_HEADER 1


//#include "MainMenu.h"
#include "MASkinG.h"
using namespace MAS;

#include <vector>
using namespace std;

namespace Interface {
	enum { NSTARS=200 };
	enum MenuDialogs { QUIT=0, MAIN_MENU, PLAY_LOCAL, PLAY_ONLINE, HOST_GAME, OPTIONS };
	enum OverlayDialogState { IDLE=0, FOLLOW_NEXT, FOLLOW_PREV }; 
};
using namespace Interface;

class OverlayDialog;
class MyUpdateDriver;

// a custom screen update driver
class MyUpdateDriver : public ScreenUpdate {
protected:
	OverlayDialog * _dialog;
	
public:
	MyUpdateDriver( OverlayDialog * dialog ) :
	  _dialog(dialog)
	  {
	  }
	  
	  BITMAP *GetCanvas();
	  
	  bool RequiresFullRedraw() {
		  return true;
	  }
};

/** A Dialog which can show a changing background underneath. */
class OverlayDialog : public Dialog {
public:

	/** the bitmap that you can write onto.  */
	BITMAP * _buffer;

	OverlayDialog(BITMAP * buffer, MenuDialogs prev, MenuDialogs next = (MenuDialogs)NULL) :
	  Dialog(),
	  _buffer(buffer),
	  _state(IDLE),
	  _prev(prev),
	  _next(next)
	{
		init();
	}

    /** this is where GUI elements should be added, not in the constructor. */
    virtual void init(){ }

	virtual void MsgStart() { 
		_state = IDLE;
		_next = (MenuDialogs)NULL;
		init();
		Dialog::MsgStart(); 
	}

	virtual void MsgEnd() { Dialog::MsgEnd(); }
	virtual void Close() { Dialog::Close(); }
	virtual void MsgIdle() { Dialog::MsgIdle(); }
	virtual void HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
		Dialog::HandleEvent(obj, msg, arg1, arg2); 
	}

	virtual OverlayDialogState getState() { return _state; }
	virtual OverlayDialogState setState(OverlayDialogState state) { return _state; }

	

	/** if the user is done interacting with this thing and wants to go back, what kind of thing should
		it return to? */
	virtual MenuDialogs getNext() { return _next; }

	/** if the user is done interacting with this thing and wants to go forward, what kind of thing should
		it return to? */
	virtual MenuDialogs getPrev() { return _prev; }

protected:
	/** what's the used doing?  Interacting with this dialog, following a link forward, 
		or following a link backwards? */
	OverlayDialogState _state;

	/** if the user is done interacting with this thing and wants to go back, what kind of thing should
		it return to? */
	MenuDialogs _prev;

	/** if the user is done interacting with this thing and wants to go forward, what kind of thing should
		it return to? */
	MenuDialogs _next;

	virtual void SelectDriver();
};

namespace Interface {

	// a simple star class
	class Star {
	protected:
		float x, y;
		float vy;
		MAS::Color col;
		
		void NewStar() {
			x = (float)(rand()%SCREEN_W);
			y = 0.0f;
			int l = rand()%128+128;
			vy = (float)(l)/128.0f;
			col = MAS::Color(l,l,l);
		}
		
	public:
		Star() {
			NewStar();
			y = (float)(rand()%SCREEN_H);
		}
		
		void Draw(BITMAP *buffer) {
			putpixel(buffer, (int)x, (int)y, col);
		}
		
		void Update() {
			y += vy;
			if ((int)y >= SCREEN_H) {
				NewStar();
			}
		}
	};

};


#endif