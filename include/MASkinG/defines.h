// ************************************************************************** //
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
// ************************************************************************** //

#ifndef		MASKING_DEFINES_H
#define		MASKING_DEFINES_H

#define		MAXINT			32768

/**
	sent whenever a timer has ticked
*/
#define		MSG_TIMER			MSG_USER
/**
	sent once per logic frame
*/
#define		MSG_TICK			MSG_USER+1
/**
	sent whenever a new skin is loaded (and whe the dialog starts)
*/
#define		MSG_INITSKIN		MSG_USER+2
/**
	sent when a widget has been activated (button pressed, checkbox toggled, a list entry double clicked, etc)
*/
#define		MSG_ACTIVATE		MSG_USER+3
/**
	sent by widgets to ask to be removed from the dialog
*/
#define		MSG_REMOVEME		MSG_USER+4
/**
	sent when the position of a scroller, slider, etc has changed
*/
#define		MSG_SCROLL			MSG_USER+5
/**
	sent when a widget or dialog was moved
*/
#define		MSG_MOVE			MSG_USER+6
/**
	sent when a widget or dialog was resized
*/
#define		MSG_RESIZE			MSG_USER+7
/**
	sent when a widget or dialog was moved and resized
*/
#define		MSG_SHAPE			MSG_USER+8
/**
	sent when the mouse cursor moves
*/
#define		MSG_MOUSEMOVE		MSG_USER+9
/**
	sent when a widget that has the D_EXIT flag set was selected
*/
#define		MSG_CLOSE			MSG_USER+10
/**
	sent to query whether a widget wants the mouse or not
*/
#define		MSG_WANTMOUSE		MSG_USER+11
/**
	sent to query whether a widget can be tabbed to
*/
#define		MSG_WANTTAB			MSG_USER+12
/**
	this is the first user available message.
	
	MASkinG uses messages for communication between widgets and dialogs.
	For this it uses both the message constants it defines and those defined
	by Allegro itself. All messages above MSG_SUSER may be used by the user.
*/
#define		MSG_SUSER			MSG_USER+13


/**
	set when the widget or dialog was moved
*/
#define		D_MOVED				D_USER
/**
	set when the widget or dialog was resized
*/
#define		D_RESIZED			D_USER<<1
/**
	set when the widget's or dialog's skin was changed
*/
#define		D_CHANGEDSKIN		D_USER<<2
/**
	set when the widget starts animating
*/
#define		D_ANIMATING			D_USER<<3
/**
	set if the widget has child widgets (ie it is some type of a dialog)
*/
#define		D_HASCHILDREN		D_USER<<4
/**
	set if the widget is a window (ie it can be moved and resized)
*/
#define		D_WINDOW			D_USER<<5
/**
	set when the widget is being pressed down (similar to D_SELECTED)
*/
#define		D_PRESSED			D_USER<<6
/**
	set if the widget is resized automatically. For example a label might resize itself
	automatically according to the length of the text or a button might
	automatically resize itself according to the size of its bitmap.
*/
#define		D_AUTOSIZE			D_USER<<7
/**
	set if the widget can be toggled (a toggle button for example)
*/
#define		D_TOGGLE			D_USER<<8
/**
	set if the widget should generate MSG_ACTIVATE messages while being pressed
	down (for example a spinner button)
*/
#define		D_SPINNER			D_USER<<9
/**
	set if the widget is movable
*/
#define		D_MOVABLE			D_USER<<10
/**
	set if the widget is resizable
*/
#define		D_RESIZABLE			D_USER<<11
/**
	set if the widget (or window) is modal, that is if it captures all input
	until it's closed
*/
#define		D_MODAL				D_USER<<12
/**
	you can use any powers of 2 above this.
	
	MASkinG uses flags to define the state of each widget and dialog. For this
	it uses both the flags it defines and those defined by Allegro. All values
	that are powers of two and above D_SUSER can be used by the user.
*/
#define		D_SUSER				D_USER<<13

#endif		//MASKING_DEFINES_H
