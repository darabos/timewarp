////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include <allegro.h>
#ifdef ALLEGRO_WINDOWS
#include <winalleg.h>
#ifdef MessageBox
	#undef MessageBox
#endif
#ifdef SendMessage
	#undef SendMessage
#endif
#ifdef DrawText
	#undef DrawText
#endif
#define YIELD() Sleep(1)
#else
#ifdef ALLEGRO_UNIX
#include <sys/time.h>
static void YIELD(void)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1;
	select(0, NULL, NULL, NULL, &tv);
}
#else
#define YIELD() yield_timeslice()
#endif
#endif

#include "../include/MASkinG/dialog.h"
#include "../include/MASkinG/settings.h"
#include "../include/MASkinG/timer.h"
#include "../include/MASkinG/gldriver.h"
#include "../include/MASkinG/drs.h"
#include "../include/MASkinG/doublebuffer.h"
#include "../include/MASkinG/triplebuffer.h"
#include "../include/MASkinG/pageflipping.h"
#include <algorithm>

MAS::MouseState MAS::Dialog::mouseState;
MAS::ScreenUpdate *MAS::Dialog::driver = NULL;
MAS::Mouse *MAS::Dialog::mouse = NULL;
int MAS::Dialog::tickTimerID = -2;

bool just_switched_in = false;
void mas_switch_callback() {
	just_switched_in = true;
}
END_OF_FUNCTION(mas_switch_callback);

#ifndef ALLEGRO_DOS
bool clicked_exit_icon = false;
void closehook(void) {
	clicked_exit_icon = true;
}
END_OF_FUNCTION(clicked_exit_icon);
#else
#define clicked_exit_icon 0
#endif


MAS::Dialog::Dialog()
	:MAS::Widget(),
	focusObject(NULL),
	mouseObject(NULL),
	running(false),
	joyOn(false),
	escapeExits(true),
	close(false),
	action(NONE)
{
	Shape(0, 0, SCREEN_W, SCREEN_H);
	widgets.reserve(50);
	SetFlag(D_HASCHILDREN);
	Widget::SetParent(this);
	minSize = MAS::Size(20, 8);
	tooltipObject = &defaultTooltipObject;
	tooltipObject->Hide();
}


MAS::Widget *MAS::Dialog::Execute(MAS::Widget *object) {
#ifdef		MASKING_GL
	allegro_gl_set_allegro_mode();
#endif
	Widget *ret = object;
	GiveFocusTo(object);
	close = false;
	Add(*tooltipObject);
	MsgStart();
	while (!close) MsgIdle();
	ret = focusObject;
	MsgEnd();
	Remove(*tooltipObject);
#ifdef		MASKING_GL
	allegro_gl_unset_allegro_mode();
#endif
	return ret;
}

class DummyBack : public MAS::Widget {
	private:
		MAS::Bitmap back;
	
	public:
		void MsgStart() {
			Widget::MsgStart();
			back.Create(SCREEN_W, SCREEN_H);
			blit(screen, back, 0, 0, 0, 0, back.w(), back.h());
			Shape(0,0,SCREEN_W,SCREEN_H);
		}
		
		void MsgEnd() {
			Widget::MsgEnd();
			back.Destroy();
		}
		
		void Draw(MAS::Bitmap &canvas) {
			back.Blit(canvas, 0, 0, 0, 0, back.w(), back.h());
		}
};

class DummyDlg : public MAS::Dialog {
	public:
		MAS::Widget *ret;
		MAS::Dialog *dlg;
		int xx, yy;
		MAS::Widget *f;
	
	private:
		DummyBack back;

	public:
		DummyDlg() : MAS::Dialog(), ret(NULL), dlg(NULL), xx(0), yy(0), f(NULL) {
			Add(back);
		}
		
		void MsgStart() {
			Dialog::MsgStart();
			ret = dlg->Popup(this, xx, yy, f);
			Close();
		}
};

MAS::Widget *MAS::Dialog::Popup(MAS::Dialog *p, int xx, int yy, MAS::Widget *f) {
	Widget *previousFocusObject = NULL;
	if (!p) {
		DummyDlg *dlg = new DummyDlg;
		dlg->dlg = this;
		dlg->xx = xx;
		dlg->yy = yy;
		dlg->f = f;
		dlg->Execute();
		Widget *ret = dlg->ret;
		delete dlg;
		SetParent(this);
		return ret;
	}
	else {
		previousFocusObject = p->GetFocusObject();
	}
	Dialog *r = p->Root();
	SetParent(r);
	if (xx != MAXINT) x(xx);
	if (yy != MAXINT) y(yy);
	Widget *ret = f;
	close = false;
	int oldf = Flags();
	SetFlag(D_MODAL);
	Add(*tooltipObject);
	r->Add(*this);
	MoveFocusTo(f);
	while (!close) r->MsgIdle();
	ret = GetFocusObject();
	Remove(*tooltipObject);
	r->Remove(*this);
	SetFlag(oldf);
	if (previousFocusObject) {
		r->MoveFocusTo(p);
		p->MoveFocusTo(previousFocusObject);
	}
	return ret;
}


MAS::Widget *MAS::Dialog::Popup(MAS::Dialog *p, const MAS::Point &pos, MAS::Widget *f) {
	return Popup(p, pos.x(), pos.y(), f);
}


void MAS::Dialog::Centre() {
	MAS::Rect r = (parent != this && parent) ? *parent : MAS::Rect(0,0,SCREEN_W,SCREEN_H);
	x(r.x() + (r.w() - w())/2);
	y(r.y() + (r.h() - h())/2);
}


bool MAS::Dialog::AlreadyAdded(const Widget &w) {
	std::vector<Widget *>::iterator i;
	for (i = widgets.begin(); i != widgets.end(); ++i) {
		if (*i == &w) {
			return true;
		}
	}
	
	return false;
}


bool MAS::Dialog::Add(Widget &w) {
	if (AlreadyAdded(w)) {
		return false;
	}
	
	// add the object to the list
	widgets.push_back(&w);

	// let the object know of its parent dialog
	w.SetParent(this);
	
	// send the object a start message if it is being added
	// while the dialog is already running
	if (running) {
		// make sure the widget has a skin
		if (!w.skin) {
			w.SetSkin(skin);
		}
	
		w.MsgInitSkin();
		w.MsgStart();
	}

	// make sure the object will draw itself when it needs to
	w.Redraw();
	
	if (w.TestFlag(D_MODAL)) {
		MoveFocus(focusObject, &w);
	}
	
	return true;
}


void MAS::Dialog::Remove(Widget &w) {
	// look for the object
	std::vector<Widget *>::iterator i;
	for (i=widgets.begin(); i != widgets.end(); ++i) {
		if (*i == &w) {
			if (running) {
				w.MsgEnd();
			}
			
			// take focus away from the object if it had it
			if (mouseObject == (*i)) {
				mouseObject = NULL;
			}
			
			if (focusObject == (*i)) {
				focusObject = NULL;
			}
			
			// remove the object
			widgets.erase(i);
			
			// redraw everything
			Root()->Redraw();
			break;
		}
	}
}


void MAS::Dialog::BringToTop(MAS::Widget &w) {
	std::vector<MAS::Widget *>::iterator i;
	for (i = widgets.begin(); i != widgets.end(); ++i) {
		if (*i == &w) {
			break;
		}
	}
	widgets.erase(i);
	widgets.push_back(&w);
	
	Redraw();
}


// sends out a message to all the widgets in the dialog
void MAS::Dialog::DialogMessage(int msg, int arg) {
	int force = ((msg == MSG_TICK) || (msg == MSG_TIMER) || (msg == MSG_START) || (msg == MSG_END) || (msg == MSG_INITSKIN));

	std::vector<MAS::Widget *>::iterator i;
	for (i=widgets.begin(); i != widgets.end(); ++i) {
		if (force || !(*i)->Hidden()) {
			(*i)->SendMessage(msg, arg);
		}
	}
}


void MAS::Dialog::CheckForRedraw() {
	// check if any widget needs to be redrawn
	std::vector<Widget *>::iterator i, j;
	MAS::Widget *w;
	
	// first pass: see which widgets need to be redrawn
	if (!driver->RequiresFullRedraw()) {
		for (i=widgets.begin(); i != widgets.end(); ++i) {
			// if the widget is dirty also make all other widgets that
			// are on top of it dirty
			if ((*i)->Dirty() && !(*i)->Hidden()) {
				for (j = i; j != widgets.end(); ++j) {
					if ((*i)->Intersects(**j)) {
						(*j)->Redraw();
					}
				}

				if (parent != this) {
					parent->RedrawArea(this, *i);
				}
			}
		}
	}
		
	// second pass: redraw dirty widgets
	driver->AcquireCanvas();
	if (parent == this) mouse->Hide();
	if (Dirty()) {
		MsgDraw();
		ClearFlag(D_DIRTY);
	}
	for (i=widgets.begin(); i != widgets.end(); ++i) {
		w = *i;
		if ((w->Dirty() || driver->RequiresFullRedraw()) && !w->Hidden()) {
			w->MsgDraw();
			w->ClearFlag(D_DIRTY);
			//driver->InvalidateRect(*w);
		}
		
#ifdef	DEBUGMODE
		rect(driver->GetCanvas(), w->x(), w->y(), w->x2(), w->y2(), Color::red);
		textprintf(driver->GetCanvas(), font, w->x()+1, w->y()+1, Color::red, "%d,%d", w->x(), w->y());
#endif
		
		if (w->HasChildren()) {
			((Dialog *)w)->CheckForRedraw();
		}
	}
	
	if (parent == this) mouse->Show();
	driver->ReleaseCanvas();
	
	ClearFlag(D_DIRTY);
}


void MAS::Dialog::RedrawArea(Widget *dlg, Rect *r) {
	std::vector<Widget *>::iterator i;
	for (i=widgets.begin(); *i != dlg; ++i);
	++i;
	for (; i != widgets.end(); ++i) {
		if ((*i)->Intersects(*r)) {
			(*i)->Redraw();
		}
	}
	
	if (parent != this) {
		parent->RedrawArea(this, r);
	}
}


void MAS::Dialog::CheckGeometry() {
	// check if any widget needs to be redrawn
	std::vector<Widget *>::iterator i;
	MAS::Widget *w;
	int f = Flags();
	if ((f & D_MOVED) && (f & D_RESIZED)) {
		MsgShape();
		ClearFlag(D_MOVED | D_RESIZED);
	}
	else if (f & D_MOVED) {
		MsgMove();
		ClearFlag(D_MOVED);
	}
	else if (f & D_RESIZED) {
		MsgResize();
		ClearFlag(D_RESIZED);
	}
	
	for (i=widgets.begin(); i != widgets.end(); ++i) {
		w = *i;
		f = w->Flags();
		if ((f & D_MOVED) && (f & D_RESIZED)) {
			w->MsgShape();
			w->ClearFlag(D_MOVED | D_RESIZED);
		}
		else if (f & D_MOVED) {
			w->MsgMove();
			w->ClearFlag(D_MOVED);
		}
		else if (f & D_RESIZED) {
			w->MsgResize();
			w->ClearFlag(D_RESIZED);
		}
		
		if (w->HasChildren()) {
			((Dialog *)w)->CheckGeometry();
		}
	}
}


void MAS::Dialog::CheckForSkinChange() {
	MAS::Widget *w;
	
	if (Flags() & D_CHANGEDSKIN) {
		MsgInitSkin();
	}
	else {
		// check if any widget's skin has changed
		for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
			w = *i;
			if (w->TestFlag(D_CHANGEDSKIN)) {
				w->MsgInitSkin();
			}
			
			if (w->HasChildren()) {
				((Dialog *)w)->CheckForSkinChange();
			}
		}
	}
}


void MAS::Dialog::SetSkin(MAS::Skin *skin) {
	Widget::SetSkin(skin);

	MAS::Widget *w;
	std::vector<Widget *>::iterator i;
	for (i=widgets.begin(); i != widgets.end(); ++i) {
		w = *i;
		w->SetSkin(this->skin);

		if (w->HasChildren()) {
			((Dialog *)w)->CheckForSkinChange();
		}
	}
}


void MAS::Dialog::MsgStart() {
	Widget::MsgStart();
	running = true;

	if (parent == this) {
		// install the system timer
		tickTimerID = Timer::InstallEx(MAS::Settings::logicFrameRate);
	
		// make sure bad things don't happen when switching to and from the program in fullscreen modes
		switch (get_display_switch_mode()) {
			case SWITCH_AMNESIA:
			case SWITCH_BACKAMNESIA:
				LOCK_VARIABLE(just_switched_in);
				LOCK_FUNCTION(mas_switch_callback);
				set_display_switch_callback(SWITCH_IN, mas_switch_callback);
		};
		
#ifndef ALLEGRO_DOS
		// make sure clicking the exit icon in windowed modes is handled properly
		LOCK_VARIABLE(clicked_exit_icon);
		LOCK_FUNCTION(closehook);
#if ALLEGRO_SUB_VERSION < 1
		set_window_close_hook(&closehook);
#else
		set_close_button_callback(&closehook);
#endif
#endif
		
		// create the screen update driver object
		SelectDriver();
	
		SetSkin(skin);
		CheckForSkinChange();

		if (!mouse) {
			mouse = new MAS::Mouse;
		}
		
		mouse->SetParent(driver);
		SetCursor(MAS::Skin::MOUSE_NORMAL);
	}
		
	// send out a MSG_START to all children widgets
	DialogMessage(MSG_START);
	
	// find the object with the mouse and give it the focus or whatever
	if (MAS::Settings::showMouse) {
		mouseState.doubleClickState = 0;
		mouseState.doubleClickDelay = MAS::Settings::logicFrameRate*MAS::Settings::doubleClickDelay/1000;
		mouseState.doubleClickTick = 0;
		
		mouseObject = FindMouseObject();
		if (mouseObject) {
			mouseObject->MsgGotmouse();
		}
		if (!focusObject) {
			GiveFocusTo(mouseObject);
		}
	}

	action = NONE;
}


void MAS::Dialog::MsgEnd() {
	focusObject = NULL;
	mouseObject = NULL;

	DialogMessage(MSG_END);
	Widget::MsgEnd();
	running = false;

	if (parent == this) {
		// remove the timer
		Timer::Kill(tickTimerID);
		tickTimerID = -2;
		
		// remove the switch callback
		remove_display_switch_callback(mas_switch_callback);

		// stop handling the exit icon
#if ALLEGRO_SUB_VERSION < 1
		set_window_close_hook(NULL);
#else
		set_close_button_callback(NULL);
#endif
	
		if (driver) {
			if (mouse) {
				// hide and destroy the mouse
				mouse->Hide();
				delete mouse;
				mouse = NULL;
			}
			
			if (driver) {
				delete driver;
				driver = NULL;
			}
		}
	}
}


void MAS::Dialog::MsgDraw() {
	Widget::MsgDraw();
	DialogMessage(MSG_DRAW);
}


void MAS::Dialog::MsgClick() {
	Widget::MsgClick();

	if (mouseObject) {
		// switch focus if necessary
		if (mouseObject != focusObject) {
			if (mouseObject->MsgWantfocus()) {
				MoveFocus(focusObject, mouseObject);
			}
		}
		
		// send a MSG_CLICK
		mouseObject->MsgClick();
	}
}


void MAS::Dialog::MsgDClick() {
	Widget::MsgDClick();

	if (mouseObject) {
		mouseObject->MsgDClick();
	}
}


//void MAS::Dialog::MsgKey();


//bool MsgChar(int c);


//bool MsgUChar(int c);


//bool MsgXChar(int c);


bool MAS::Dialog::MsgWantfocus() {
	Widget::MsgWantfocus();
	return true;
}



void MAS::Dialog::MsgGotfocus() {
	SetFlag(D_GOTFOCUS);
	GetParent()->HandleEvent(*this, MSG_GOTFOCUS);
}


void MAS::Dialog::MsgLostfocus() {
	ClearFlag(D_GOTFOCUS);
	GetParent()->HandleEvent(*this, MSG_LOSTFOCUS);
}


void MAS::Dialog::MsgGotmouse() {
	Widget::MsgGotmouse();

	/*
	if (!TestFlag(D_PRESSED)) {
		SelectAction(GetAction());
	}
	*/
}


void MAS::Dialog::MsgLostmouse() {
	Widget::MsgLostmouse();

	/*
	if (!TestFlag(D_PRESSED)) {
		SelectAction(GetAction());
	}
	*/
}


void MAS::Dialog::MsgIdle() {
	Widget::MsgIdle();
	if (parent == this) {
		bool needToRedraw = false;
		
		if (just_switched_in) {
			just_switched_in = false;
			if (MAS::Settings::useVideoMemory) {
				if (theSkin != skin) {
					theSkin->Reload();
				}
				skin->Reload();
				SetSkin(skin);
			}
			Redraw();
		}
		
		// do the logic if a timer ticked
		int timerID;
		while ((timerID = Timer::Check()) >= 0) {
			// if it's the dialog manager timer do the dialog managing thingy
			if (timerID == tickTimerID) {
				// send a tick message
				MsgTick();
				
				// make sure we redraw at least once every tick
				needToRedraw = true;
			}
			// otherwise send out a timer message
			else {
				MsgTimer(timerID);
			}

			// update the timer variable
			Timer::Update(timerID);
		}
		
		Timer::ResetFrameSkip();
		
		// redraw if necessary
		if (needToRedraw || MAS::Settings::unlimitedFrameRate) {
			// check if any widget needs a redraw
			CheckForRedraw();
			
			// update the screen
			driver->Redraw();

			needToRedraw = false;
		}
	
		// yield a timeslice to play nice with the OS
		if (MAS::Settings::yield) {
			while (Timer::Check() < 0) {
				YIELD();
			}
		}

#ifndef ALLEGRO_DOS
		if (clicked_exit_icon) {
			clicked_exit_icon = false;
			Close();
		}
#endif
	}

	DialogMessage(MSG_IDLE);
}


//void MAS::Dialog::MsgRadio(int g);


void MAS::Dialog::MsgWheel(int z) {
	Widget::MsgWheel(z);
	if (mouseObject) {
		mouseObject->MsgWheel(z);
	}
}


void MAS::Dialog::MsgLPress() {
	Widget::MsgLPress();
	if (focusObject && focusObject != mouseObject) {
		if (focusObject->Flags() & D_PRESSED) {
			focusObject->MsgLPress();
		}
	}
	if (mouseObject) {
		mouseObject->MsgLPress();
	}

	actionOrigin = GetMousePos();
	SetFlag(D_PRESSED);
}


void MAS::Dialog::MsgMPress() {
	Widget::MsgMPress();
	if (focusObject && focusObject != mouseObject) {
		if (focusObject->Flags() & D_PRESSED) {
			focusObject->MsgMPress();
		}
	}
	if (mouseObject) {
		mouseObject->MsgMPress();
	}
}


void MAS::Dialog::MsgRPress() {
	Widget::MsgRPress();
	if (focusObject && focusObject != mouseObject) {
		if (focusObject->Flags() & D_PRESSED) {
			focusObject->MsgRPress();
		}
	}
	if (mouseObject) {
		mouseObject->MsgRPress();
	}
}


void MAS::Dialog::MsgLRelease() {
	Widget::MsgLRelease();
	if (focusObject && focusObject != mouseObject) {
		if (focusObject->Flags() & D_PRESSED) {
			focusObject->MsgLRelease();
		}
	}
	if (mouseObject) {
		mouseObject->MsgLRelease();
	}

	ClearFlag(D_PRESSED);
}


void MAS::Dialog::MsgMRelease() {
	Widget::MsgMRelease();
	if (focusObject && focusObject != mouseObject) {
		if (focusObject->Flags() & D_PRESSED) {
			focusObject->MsgMRelease();
		}
	}
	if (mouseObject) {
		mouseObject->MsgMRelease();
	}
}


void MAS::Dialog::MsgRRelease() {
	Widget::MsgRRelease();
	if (focusObject && focusObject != mouseObject) {
		if (focusObject->Flags() & D_PRESSED) {
			focusObject->MsgRRelease();
		}
	}
	if (mouseObject) {
		mouseObject->MsgRRelease();
	}
}


void MAS::Dialog::MsgTimer(int t) {
	Widget::MsgTimer(t);
	DialogMessage(MSG_TIMER, t);
}


void MAS::Dialog::MsgTick() {
	Widget::MsgTick();
	
	// only the top level dialog can do the dialog manager thingy
	if (parent == this) {
		// see if any widget's skin was changed
		CheckForSkinChange();
	
		// handle mouse input (moving focus, clicks, double clicks, etc.)
		if (MAS::Settings::showMouse) {
			HandleMouseInput();
		}
		
		// handle keyboard input (moving focus, keyboard shortcuts, etc.)
		if (MAS::Settings::useKeyboard) {
			HandleKeyboardInput();
		}

		// see if any widget was moved and/or resized
		CheckGeometry();
	}

	// send out a MSG_TICK
	DialogMessage(MSG_TICK);
}


void MAS::Dialog::MsgMove() {
	Widget::MsgMove();
	UpdateSize();
}



void MAS::Dialog::MsgResize() {
	Widget::MsgResize();
	UpdateSize();
}



void MAS::Dialog::MsgShape() {
	Widget::MsgShape();
	UpdateSize();
}

void MAS::Dialog::UpdateSize() {
	std::vector<MAS::Widget *>::iterator i;
	MAS::Widget *w;
	for (i = widgets.begin(); i != widgets.end(); ++i) {
		w = *i;
		w->SendMessage(MSG_SHAPE);
		// move the widget to the right absolute position
		w->UpdatePosition();
		
		if (w->HasChildren()) {
			((Dialog *)w)->UpdateSize();
		}
	}
}


void MAS::Dialog::MsgMousemove(const MAS::Point& d) {
	Widget::MsgMousemove(d);

	// send a MSG_MOUSEMOVE message to the widget with the mouse/focus
	if (focusObject && focusObject != mouseObject) {
		if (focusObject->Flags() & D_PRESSED) {
			focusObject->MsgMousemove(d);
		}
	}
	if (mouseObject) {
		mouseObject->MsgMousemove(d);
	}

	// see which widget is under the mouse
	MAS::Widget *newMouseObject = FindMouseObject();
	
	// see if mouse object has changed
	if (newMouseObject != mouseObject) {
		MoveMouse(mouseObject, newMouseObject);
		
		// switch input focus if necessary
		switch (skin->focus) {
			case 0:
				if (mouseObject != focusObject) {
					if ((focusObject && (!(focusObject->Flags() & D_PRESSED))) || !focusObject) {
						MoveFocus(focusObject, mouseObject);
					}
				}
				break;

			case 1:
				if (mouseObject != focusObject) {
					if ((focusObject && (!(focusObject->Flags() & D_PRESSED))) || !focusObject) {
						if (mouseObject && mouseObject->MsgWantfocus()) {
							MoveFocus(focusObject, mouseObject);
						}
					}
				}
				break;

			case 2:
				break;
		};
		
		// let it know the mouse moved
		if (mouseObject) {
			mouseObject->MsgMousemove(d);
		}
	}

	// move or resize the dialog
	if (TestFlag(D_PRESSED)) {
		Rect oldRect = *this;
		Point d2 = GetMousePos() - actionOrigin;
		int x2 = MAX(x() + d2.x(), 0);
		int y2 = MAX(y() + d2.y(), 0);
		int w2, h2;
		if (action == MOVE && TestFlag(D_MOVABLE)) {
			Place(x2, y2);
			MsgMove();
			Root()->Redraw();
		}
		else if ((action != NONE && TestFlag(D_RESIZABLE))) {
			switch (action) {
				case RESIZE_UP_LEFT:
					w2 = MAX(w() + x() - x2, minSize.w());
					h2 = MAX(h() + y() - y2, minSize.h());
					x2 = this->x2() - w2;
					y2 = this->y2() - h2;
					Resize(w2, h2);
					Place(x2, y2);
					break;
				
				case RESIZE_DOWN_RIGHT:
					w2 = MAX(w() + d2.x(), minSize.w());
					h2 = MAX(h() + d2.y(), minSize.h());
					actionOrigin.x(actionOrigin.x() + w2 - w());
					actionOrigin.y(actionOrigin.y() + h2 - h());
					Resize(w2, h2);
					break;
					
				case RESIZE_UP:
					h2 = MAX(h() + y() - y2, minSize.h());
					y2 = this->y2() - h2;
					h(h2);
					y(y2);
					break;
				
				case RESIZE_DOWN:
					h2 = MAX(h() + d2.y(), minSize.h());
					actionOrigin.y(actionOrigin.y() + h2 - h());
					h(h2);
					break;
					
				case RESIZE_UP_RIGHT:
					w2 = MAX(w() + d2.x(), minSize.w());
					h2 = MAX(h() + y() - y2, minSize.h());
					y2 = this->y2() - h2;
					actionOrigin.x(actionOrigin.x() + w2 - w());
					Resize(w2, h2);
					y(y2);
					break;
				
				case RESIZE_DOWN_LEFT:
					w2 = MAX(w() + x() - x2, minSize.w());
					h2 = MAX(h() + d2.y(), minSize.h());
					x2 = this->x2() - w2;
					actionOrigin.y(actionOrigin.y() + h2 - h());
					Resize(w2, h2);
					x(x2);
					break;
				
				case RESIZE_LEFT:
					w2 = MAX(w() + x() - x2, minSize.w());
					x2 = this->x2() - w2;
					w(w2);
					x(x2);
					break;
				
				case RESIZE_RIGHT:
					w2 = MAX(w() + d2.x(), minSize.w());
					actionOrigin.x(actionOrigin.x() + w2 - w());
					w(w2);
					break;
				
				default:
					return;
			};

			MsgShape();
			Root()->Redraw();
		}
	}
	else {
		SelectAction(GetAction());
	}
}


void MAS::Dialog::MsgInitSkin() {
	DialogMessage(MSG_INITSKIN);
	Widget::MsgInitSkin();
}


void MAS::Dialog::HandleMouseInput() {
	if (focusObject && focusObject->TestFlag(D_MODAL)) {
		((MAS::Dialog *)focusObject)->HandleMouseInput();
		return;
	}
	
	// handle mouse clicks
	int click = 0;
	Point mousePos = mouse->pos;
	int f = mouse->flags;
	int z = mouse->z;

	if (mouse) {
		poll_mouse();
		mouse->Update();
	}

	if (!(f & 1) && (mouse->flags & 1)) {
		// left mouse button was pressed
		MsgClick();
		MsgLPress();
		click |= 1;
	}
	else if ((f & 1) && !(mouse->flags & 1)) {
		// left mouse button was released
		MsgLRelease();
	}

	if (!(f & 2) && (mouse->flags & 2)) {
		// right mouse button was pressed
		MsgClick();
		MsgRPress();
		click |= 2;
	}
	else if ((f & 2) && !(mouse->flags & 2)) {
		// right mouse button was released
		MsgRRelease();
	}

	if (!(f & 4) && (mouse->flags & 4)) {
		// middle mouse button was pressed
		MsgClick();
		MsgMPress();
		click |= 4;
	}
	else if ((f & 4) && !(mouse->flags & 4)) {
		// middle mouse button was released
		MsgMRelease();
	}
	
	// handle double clicks
	int bit = 0;
	switch (mouseState.doubleClickState) {
		case 1:	bit = 1; break;
		case 2:	bit = 2; break;
		case 3:	bit = 4; break;
	};
	
	switch (mouseState.doubleClickState) {
		case 0:
			if (click & 1) mouseState.doubleClickState = 1; else
			if (click & 2) mouseState.doubleClickState = 2; else
			if (click & 4) mouseState.doubleClickState = 3;
			break;
			
		case 1:
		case 2:
		case 3:
			if (click & bit) {
				mouseState.doubleClickState = 4;
			}
			else {
				++mouseState.doubleClickTick;
				if (mouseState.doubleClickTick >= mouseState.doubleClickDelay) {
					mouseState.doubleClickTick = 0;
					mouseState.doubleClickState = 0;
				}
			}
			break;
			
		case 4:
			MsgDClick();
			mouseState.doubleClickTick = 0;
			mouseState.doubleClickState = 0;
			break;
	};
	
	// handle mouse moves
	if (mousePos != mouse->pos) {
		// the mouse has moved!
		MsgMousemove(mouse->pos - mousePos);
	}
	
	// handle mouse scroller button
	if (z != mouse->z) {
		MsgWheel(mouse->z - z);
	}
}


void MAS::Dialog::HandleKeyboardInput() {
	if (focusObject && focusObject->TestFlag(D_MODAL)) {
		((MAS::Dialog *)focusObject)->HandleKeyboardInput();
		return;
	}

	int cAscii, cScan;

	// fake joystick input by converting it to key presses
	if (MAS::Settings::useJoystick) {
		if (joyOn) {
			rest(1000/MAS::Settings::logicFrameRate);
		}
	
		poll_joystick();
	
		if (joyOn) {
			if ((!joy[0].stick[0].axis[0].d1) && (!joy[0].stick[0].axis[0].d2) && 
				(!joy[0].stick[0].axis[1].d1) && (!joy[0].stick[0].axis[1].d2) &&
				(!joy[0].button[0].b) && (!joy[0].button[1].b))
			{
				joyOn = false;
				rest(1000/MAS::Settings::logicFrameRate);
			}
			cAscii = cScan = 0;
		}
		else {
			if (joy[0].stick[0].axis[0].d1) {
				cAscii = 0;
				cScan = KEY_LEFT;
				joyOn = true;
			}
			else if (joy[0].stick[0].axis[0].d2) {
				cAscii = 0;
				cScan = KEY_RIGHT;
				joyOn = true;
			}
			else if (joy[0].stick[0].axis[1].d1) {
				cAscii = 0;
				cScan = KEY_UP;
				joyOn = true;
			}
			else if (joy[0].stick[0].axis[1].d2) {
				cAscii = 0;
				cScan = KEY_DOWN;
				joyOn = true;
			}
			else if ((joy[0].button[0].b) || (joy[0].button[1].b)) {
				cAscii = ' ';
				cScan = KEY_SPACE;
				joyOn = true;
			}
			else {
				cAscii = cScan = 0;
			}
		}
	}

	// actually handle keyboard input
	if (keypressed()) {
		cAscii = ureadkey(&cScan);
		SendKeyboardMessages(cAscii, cScan);
	}
}


bool MAS::Dialog::SendKeyboardMessages(int cAscii, int cScan) {
	int cCombo = (cScan<<8) | ((cAscii <= 255) ? cAscii : '^');

	// send messages
	if (focusObject) {
		if (focusObject->HasChildren()) {
			if (((MAS::Dialog *)focusObject)->SendKeyboardMessages(cAscii, cScan)) {
				return true;
			}
		}
		
		if (focusObject->MsgChar(cCombo)) {
			return true;
		}

		if (focusObject->MsgUChar(cAscii)) {
			return true;
		}
	}
	
	// keyboard shortcuts
	std::vector<MAS::Widget *>::iterator i;
	for (i = widgets.begin(); i != widgets.end(); ++i) {
		if ((((cAscii > 0) && (cAscii <= 255) && (utolower((*i)->key) == utolower(cAscii))) || ((!cAscii) && ((*i)->key == (cScan<<8)))) && (!((*i)->flags & (D_HIDDEN | D_DISABLED))))	{
			(*i)->MsgKey();
			return true;
		}
	}
	
	// broadcast key
	for (i = widgets.begin(); i != widgets.end(); ++i) {
		if (!((*i)->flags & (D_HIDDEN | D_DISABLED))) {
			if ((*i)->MsgXChar(cCombo)) {
				return true;
			}
		}
	}
	
	// handle <CR> and <SPACE>
	if (((cAscii == '\r') || (cAscii == '\n') || (cAscii == ' ')) && (focusObject)) {
		focusObject->MsgKey();
		return true;
	}
	
	// handle <ESC>
	if (cAscii == 27 && escapeExits && (parent == this || IsWindow())) {
		Close();
		if (close) {
			TakeFocusFrom(focusObject);
		}
		return true;
	}
	
	// move focus
	if (parent == this || IsWindow()) {
		int dir;
		switch (cScan) {
			case KEY_TAB:	dir = (key_shifts & KB_SHIFT_FLAG) ? 1 : 0;		break;
			case KEY_RIGHT: dir = 2;	break;
			case KEY_LEFT:	dir = 3;	break;
			case KEY_DOWN:	dir = 4;	break;
			case KEY_UP:	dir = 5;	break;
			default:		return false;
		}
		MoveFocus(dir);
		return true;
	}
	
	return false;
}


MAS::Widget *MAS::Dialog::FindMouseObject() {
	Point pos = GetMousePos() + GetOffset();
	std::vector<MAS::Widget *>::iterator i = widgets.end();
	Widget *w;
	
	for (; i != widgets.begin();) {
		--i;
		w = *i;
		
		if (!w->Hidden() && !w->Disabled()) {
			if (w->HasChildren() && ((Dialog *)w)->FindMouseObject()) {
				return w;
			}
			
			if (pos <= *w && w->MsgWantmouse()) {
				return w;
			}
		}
	}
	
	return NULL;
}


void MAS::Dialog::GiveFocusTo(MAS::Widget *w) {
	if (w && w->MsgWantfocus() && !w->Hidden() && !w->Disabled()) {
		w->MsgGotfocus();
		w->GetParent()->focusObject = w;

		if (w->IsWindow()) {
			BringToTop(*w);
		}
		
		if (w->GetParent() != this) {
			GiveFocusTo(w->GetParent());
		}
	}
}


void MAS::Dialog::TakeFocusFrom(MAS::Widget *w) {
	if (w && !w->TestFlag(D_MODAL)) {
		w->GetParent()->focusObject = NULL;
		w->MsgLostfocus();
		
		//focusObject = NULL;

		if (w->GetParent() != this) {
			TakeFocusFrom(w->GetParent());
		}
	}
}


void MAS::Dialog::MoveFocus(MAS::Widget *src, MAS::Widget *dest) {
	if (src && src->HasChildren()) {
		((Dialog *)src)->MoveFocus(((Dialog *)src)->focusObject, NULL);
	}
	TakeFocusFrom(src);
	GiveFocusTo(dest);
	if (parent != this) {
		parent->MoveFocusTo(this);
	}
}


void MAS::Dialog::MoveFocusTo(MAS::Widget *dest) {
	TakeFocusFrom(focusObject);
	GiveFocusTo(dest);
}


void MAS::Dialog::GiveMouseTo(MAS::Widget *w) {
	if (w && w->MsgWantmouse() && !w->Hidden() && !w->Disabled()) {
		w->MsgGotmouse();
		w->GetParent()->mouseObject = w;

		if (w->GetParent() != this) {
			GiveMouseTo(w->GetParent());
		}
	}
}


void MAS::Dialog::TakeMouseFrom(MAS::Widget *w) {
	if (w) {
		w->GetParent()->mouseObject = NULL;
		w->MsgLostmouse();

		if (w->GetParent() != this) {
			TakeMouseFrom(w->GetParent());
		}
	}
}


void MAS::Dialog::MoveMouse(MAS::Widget *src, MAS::Widget *dest) {
	if (src && src->HasChildren()) {
		((Dialog *)src)->MoveMouse(((Dialog *)src)->mouseObject, NULL);
	}
	TakeMouseFrom(src);
	GiveMouseTo(dest);
	if (parent != this) {
		parent->GiveMouseTo(this);
	}
}


// comparison functions for comparing rectangles (so we can sort them)
static int s_cmp_tab(const MAS::Widget *d1, const MAS::Widget *d2) {
	int ret = (int)d2 - (int)d1;
	return (ret < 0) ? ret += 0x10000 : ret;
}

static int s_cmp_shift_tab(const MAS::Widget *d1, const MAS::Widget *d2) {
	int ret = (int)d1 - (int)d2;
	return (ret < 0) ? ret += 0x10000 : ret;
}

static int s_cmp_right(const MAS::Widget *d1, const MAS::Widget *d2) {
	int ret = (d2->x() - d1->x()) + ABS(d1->y() - d2->y()) * 2;
	return (d1->x() >= d2->x()) ? ret += 0x10000 : ret;
}

static int s_cmp_left(const MAS::Widget *d1, const MAS::Widget *d2) {
	int ret = (d1->x() - d2->x()) + ABS(d1->y() - d2->y()) * 2;
	return (d1->x() <= d2->x()) ? ret += 0x10000 : ret;
}

static int s_cmp_down(const MAS::Widget *d1, const MAS::Widget *d2) {
	int ret = (d2->y() - d1->y()) + ABS(d1->x() - d2->x()) * 2;
	return (d1->y() >= d2->y()) ? ret += 0x10000 : ret;
}

static int s_cmp_up(const MAS::Widget *d1, const MAS::Widget *d2) {
	int ret = (d1->y() - d2->y()) + ABS(d1->x() - d2->x()) * 2;
	return (d1->y() <= d2->y()) ? ret += 0x10000 : ret;
}

// comparison function for list::sort()
static bool cmp_obj_list(const MAS::MAS_OBJ_LIST *obj1, const MAS::MAS_OBJ_LIST *obj2) {
	return obj1->diff < obj2->diff;
}


// dir:
// 0 - TAB
// 1 - shift+TAB
// 2 - RIGHT
// 3 - LEFT
// 4 - DOWN
// 5 - UP
void MAS::Dialog::MoveFocus(int dir) {
	// a pointer to the comparison function
	int (*cmp)(const MAS::Widget *d1, const MAS::Widget *d2);
	
	// choose a comparison function
	switch (dir) {
		case 0:	cmp = s_cmp_tab;		break;
		case 1:	cmp = s_cmp_shift_tab;	break;
		case 2: cmp = s_cmp_right;		break;
		case 3:	cmp = s_cmp_left;		break;
		case 4:	cmp = s_cmp_down;		break;
		case 5:	cmp = s_cmp_up;			break;
		default: return;
	};
	
	// a temporary list of rectangles
	std::vector<MAS_OBJ_LIST *> objList;
	Widget *f = Root()->GetFocusObject();
	int c=0;

	// fill temporary table
	FillObjectList(objList, cmp, f, c);
	
	// sort table
	std::sort(objList.begin(), objList.end(), cmp_obj_list);

	// find an object to give the focus to
	std::vector<MAS_OBJ_LIST *>::iterator j;
	for (j = objList.begin(); j != objList.end(); ++j) {
		if ((*j)->w->MsgWantTab()) {
			MoveFocus(focusObject, (*j)->w);
			break;
		}
	}
	
	// clean up
	while (!objList.empty()) {
		j = objList.end();
		--j;
		delete *j;
		objList.pop_back();
	}
}


void MAS::Dialog::FillObjectList(std::vector<MAS_OBJ_LIST *> &objList, int (*cmp)(const MAS::Widget *, const MAS::Widget *), MAS::Widget *f, int &c) {
	std::vector<MAS::Widget *>::iterator i;
	Widget *w;
	for (i = widgets.begin(); i != widgets.end(); ++i) {
		w = *i;
		if (!w->Hidden() && !w->Disabled() && !(w->IsWindow())) {
			if (w->HasChildren()) {
				((Dialog *)w)->FillObjectList(objList, cmp, f, c);
			}
			else if (!w->HasFocus()) {
				MAS_OBJ_LIST *item = new MAS_OBJ_LIST;
				item->w = w;
				if (f) {
					item->diff = cmp(f, w);
				}
				else {
					item->diff = c;
				}
				objList.push_back(item);
				++c;
			}
		}
	}
}


MAS::Dialog *MAS::Dialog::Root() {
	if (parent == this) {
		return this;
	}
	else {
		return parent->Root();
	}
}


void MAS::Dialog::HandleEvent(Widget &obj, int msg, int arg1, int arg2) {
	if (msg == MSG_ACTIVATE && obj.WillExit()) {
		Close();
	}
	else if (parent != this) {
		parent->HandleEvent(obj, msg, arg1, arg2);
	}
}


BITMAP *MAS::Dialog::GetCanvas(Point &offset) {
	if ((flags & D_ANIMATING) && !animator->Animating()) {
		return NULL;
	}
	if (!(flags & D_ANIMATING) && animator->Animating()) {
		offset -= topLeft();
		return animator->GetBuffer();
	}
	else if (parent != this) {
		return parent->GetCanvas(offset);
	}
	else {
		return driver->GetCanvas();
	}
}


BITMAP *MAS::Dialog::GetCanvas(MAS::Widget *obj) {
	Point p = obj->topLeft();
	BITMAP *pCanvas = GetCanvas(p);
	if (pCanvas && p.x() < pCanvas->w && p.y() < pCanvas->h) {
		int ww = p.x() + obj->w() > x2() ? x2() - p.x() : obj->w();
		int hh = p.y() + obj->h() > y2() ? y2() - p.y() : obj->h();
		if (ww <= 0 || hh <= 0) {
			return NULL;
		}
		else {
			return create_sub_bitmap(pCanvas, p.x(), p.y(), ww, hh);
		}
	}
	else {
		return NULL;
	}
}


MAS::ScreenUpdate *MAS::Dialog::GetDriver() const {
	return driver;
}


MAS::Mouse *MAS::Dialog::GetMouse() const {
	return mouse;
}


void MAS::Dialog::SetMouse(MAS::Mouse *m) {
	if (mouse) {
		m->SetParent(driver);
		m->pos = mouse->pos;
		m->flags = mouse->flags;
		m->z = mouse->z;

		delete mouse;
		mouse = m;
		SetCursor(MAS::Skin::MOUSE_NORMAL);
	}
	else {
		mouse = m;
	}
}


MAS::Point MAS::Dialog::GetOffset() const {
	if (parent == this) {
		return Point(0,0);
	}
	else {
		return relativePos + parent->GetOffset();
	}
}


void MAS::Dialog::Close() {
	if (MsgClose()) {
		if (parent == this || TestFlag(D_MODAL)) {
			close = true;
		}
		else {
			GetParent()->HandleEvent(*this, MSG_CLOSE);
		}
	}
}


void MAS::Dialog::SetMouseCursor(int index) {
	// set the mouse cursor
	if (mouse) {
		mouse->SetCursor(skin->GetCursor(index));
	}
}


bool MAS::Dialog::HasMouse() {
	for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		if ((*i)->HasMouse()) {
			return true;
		}
	}
	
	return false;
}


bool MAS::Dialog::HasFocus() {
	for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		if ((*i)->HasFocus()) {
			return true;
		}
	}
	
	return false;
}


void MAS::Dialog::Hide() {
	Widget::Hide();
	for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		(*i)->Hide();
	}
}


void MAS::Dialog::Unhide() {
	Widget::Unhide();
	for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		(*i)->Unhide();
	}
}


void MAS::Dialog::Disable() {
	Widget::Disable();
	for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		(*i)->Disable();
	}
}


void MAS::Dialog::Enable() {
	Widget::Enable();
	for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		(*i)->Enable();
	}
}


void MAS::Dialog::Redraw() {
	Widget::Redraw();
	for (std::vector<Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		(*i)->Redraw();
	}
}


void MAS::Dialog::SelectDriver() {
	if (!driver) {
		MAS::Error e;
#ifdef	MASKING_GL
		int m = MAS::Settings::gfxMode;
		if (m == GFX_OPENGL || m == GFX_OPENGL_WINDOWED || m == GFX_OPENGL_FULLSCREEN) {
			switch (MAS::Settings::screenUpdateMethod) {
				case 3:
					e = CreateUpdateDriver(new MAS::GLDriverDRS);
					if (e) CreateUpdateDriver(new MAS::ScreenUpdate);
					break;
				
				case 0:
					e = CreateUpdateDriver(new MAS::GLDriverDoubleBuffer);
					if (e) CreateUpdateDriver(new MAS::ScreenUpdate);
					break;
				
				default:
					e = CreateUpdateDriver(new MAS::GLDriver);
					if (e) CreateUpdateDriver(new MAS::ScreenUpdate);
					break;
			};
		}
		else {
			switch (MAS::Settings::screenUpdateMethod) {
				case 3:
					e = CreateUpdateDriver(new MAS::DRS);
					if (e) CreateUpdateDriver(new MAS::ScreenUpdate);
					break;
	
				case 2:
					e = CreateUpdateDriver(new MAS::TripleBuffer);
					if (!e) break;
					// fall through
	
				case 1:
					e = CreateUpdateDriver(new MAS::PageFlipping);
					if (!e) break;
	
				case 0:
					e = CreateUpdateDriver(new MAS::DoubleBuffer);
					if (!e) break;
	
				default:
					e = CreateUpdateDriver(new MAS::ScreenUpdate);
					break;
			};
		}
#else
		switch (MAS::Settings::screenUpdateMethod) {
			case 3:
				e = CreateUpdateDriver(new MAS::DRS);
				if (e) CreateUpdateDriver(new MAS::ScreenUpdate);
				break;

			case 2:
				e = CreateUpdateDriver(new MAS::TripleBuffer);
				if (!e) break;
				// fall through

			case 1:
				e = CreateUpdateDriver(new MAS::PageFlipping);
				if (!e) break;

			case 0:
				e = CreateUpdateDriver(new MAS::DoubleBuffer);
				if (!e) break;

			default:
				e = CreateUpdateDriver(new MAS::ScreenUpdate);
				break;
		};
#endif	//MASKING_GL
	}
}


MAS::Error MAS::Dialog::CreateUpdateDriver(MAS::ScreenUpdate *d) {
	driver = d;
	return driver->Create();
}


MAS::Dialog::ActionType MAS::Dialog::GetAction() {
	if (parent != this && mouseObject != focusObject)
		return MOVE;
	else
		return NONE;
}


void MAS::Dialog::SelectAction(MAS::Dialog::ActionType action) {
	if (this->action != action) {
		this->action = action;
		
		switch (action) {
			case NONE:
			case MOVE:
				SetCursor(Skin::MOUSE_NORMAL);
				break;

			case RESIZE_UP_LEFT:
			case RESIZE_DOWN_RIGHT:
				SetCursor(Skin::MOUSE_SIZE_DIAGONAL1);
				break;
			
			case RESIZE_UP:
			case RESIZE_DOWN:
				SetCursor(Skin::MOUSE_SIZE_VERTICAL);
				break;
			
			case RESIZE_UP_RIGHT:
			case RESIZE_DOWN_LEFT:
				SetCursor(Skin::MOUSE_SIZE_DIAGONAL2);
				break;
			
			case RESIZE_LEFT:
			case RESIZE_RIGHT:
				SetCursor(Skin::MOUSE_SIZE_HORIZONTAL);
				break;
		};
	}
}


MAS::Widget *MAS::Dialog::GetFocusObject() const {
	if (focusObject && focusObject->HasChildren()) {
		return ((Dialog *)focusObject)->GetFocusObject();
	}
	else {
		return focusObject;
	}
}


MAS::Widget *MAS::Dialog::GetMouseObject() const {
	if (mouseObject && mouseObject->HasChildren()) {
		return ((Dialog *)mouseObject)->GetMouseObject();
	}
	else {
		return mouseObject;
	}
}


void MAS::Dialog::SetTooltipObject(MAS::Tooltip *tooltipObject) {
	Remove(*tooltipObject);
	if (tooltipObject) {
		this->tooltipObject = tooltipObject;
	}
	else {
		this->tooltipObject = &defaultTooltipObject;
	}
	Add(*tooltipObject);
}


void MAS::Dialog::Dump() {
	char buf[256];
	int count = 0;
	usprintf(buf, "this: [%d, %d] - [%d, %d]\n", x(), y(), x2(), y2());
	TRACE(buf);
	for (std::vector<MAS::Widget *>::iterator i = widgets.begin(); i != widgets.end(); ++i) {
		usprintf(buf, "%d: [%d, %d] - [%d, %d]\n", count, (*i)->x(), (*i)->y(), (*i)->x2(), (*i)->y2());
		TRACE(buf);
		++count;
	}
	if (parent) {
		usprintf(buf, "parent: [%d, %d] - [%d, %d]\n", parent->x(), parent->y(), parent->x2(), parent->y2());
		TRACE(buf);
	}
}
