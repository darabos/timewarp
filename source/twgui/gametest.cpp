
// NOTE: the high-level routines and structures are on the bottom of this file.
// the lowest level functions are on the top.

// Three distinctions: 1. area detection and definition, 2. bitmap feedback, 3. a small
// class for each button you need to perform a specific task based on a particular mouse
// operation.

// This makes (imo) best use of classes and their
// overloading (specialization) abilities. I think that's the most general way and can support
// a wide variety of implementations.

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE
#include "../frame.h"
#include "../melee/mgame.h"
//#include "../melee/mcbodies.h"
//#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../melee/mmain.h"
//#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mitems.h"
//#include "../melee/mfleet.h"   //fleets...

#include "../scp.h"
#include "../util/history.h"

#include "gametest.h"
#include "twgui.h"



// ----------------- AND NOW THE IMPLEMENTATION PART -------------------
// (where you use it: a Button with a specific function)


stopbutton::stopbutton()
{
	exitcode = 0;
}


void stopbutton::handle_lpress()
{
	// this code executed when the left mouse button is pressed
	exitcode = 1;
}


int stopbutton::stop()
{
	return exitcode;
}



// how to generate a (short) list of ship sprites ??

class TestSprites
{
public:
	BITMAP **sprites;
	int Nx, Ny, D;

	~TestSprites();
	void init();
};

void TestSprites::init()
{
	Nx = 10;
	Ny = 10;

	D = 30;

	sprites = new BITMAP* [Nx*Ny];

	int i;
	for ( i = 0; i < Nx*Ny; ++i )
	{
		sprites[i] = create_bitmap_ex(bitmap_color_depth(screen), D, D);
		clear_to_color(sprites[i], makecol(4*i,255-5*i,128+3*i));
	}
	
}

TestSprites::~TestSprites()
{
	int i;
	for ( i = 0; i < Nx*Ny; ++i )
	{
		if (sprites[i])
			destroy_bitmap(sprites[i]);
	}
}

// ------------- the Game where I test this button -----------






void TWgui::init(Log *_log)
{
	NormalGame::init(_log);

	
	
	
	//	MenuBox(int dx, int dy, int dW, int dH, int default_W, DATAFILE *dataf, BITMAP *backgr = 0);
	message.out("create areareserve");
	testmenu = new AreaReserve("TEST", 10, 10, 200, 200, 800,
		"twgui.dat", window->surface, "BACKGR");
	
	
	// FIRST add, THEN init (cause the adding adds information and is part of the init)
	//	void MenuBox::add(MenuItem *item, int dx, int dy, int dW, int dH)
	
	stopbutton *tb;
	tb = new stopbutton();
	tb->init(testmenu, "_STOP", 10, 10);				// init button stuff here

	TextButton *tt;
	tt = new TextButton();
	tt->init(testmenu, "_STOP", 10, 60);
	tt->set_text("hello");


	TestSprites *bmplist;
	bmplist = new TestSprites();
	bmplist->init();

	MatrixIcons *mi;
	mi = new MatrixIcons();
	mi->init(testmenu, "_MATRIX", 60, 60,
					bmplist->sprites, bmplist->Nx, bmplist->Ny, bmplist->D);


	unscare_mouse();
	show_mouse(window->surface);


	// a more "serious" application, does a few small buttons in the melee environment:
	// (very simple tasks, so there's no real need to create separate classes for
	// each of these buttons).

	meleemenu = new AreaReserve("MELEE", 50, 500,  600, 100,  800,
		"twgui.dat", window->surface, "MELEE_BACKGR");

	zoomout = new Button();
	zoomout->init(meleemenu, "_ZOOMOUT", 45, 11);

	zoomin = new Button();
	zoomin->init(meleemenu, "_ZOOMIN", 46, 32);

	abort = new Button();
	abort->init(meleemenu, "_ABORT", 46, 58);

}

void TWgui::calculate()
{

	NormalGame::calculate();


	testmenu->calculate();			// in principle, drawing operations shouldn't go here.




	meleemenu->calculate();

	if (zoomin->selected)
	{
		// copied this from view_hero :
		//if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
		//if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
		view->camera.z /= 1 + 0.002 * frame_time;
	}
	if (zoomout->selected)
	{
		view->camera.z *= 1 + 0.002 * frame_time;
	}

	if (abort->selected)
	{
		pause();
		if (tw_alert("Game is paused", "&Abort game", "&Resume playing") == 1) {
			game->quit("quit - Game aborted from keyboard");
		}
		unpause();
	}		
}


void TWgui::animate(Frame *frame)
{

	NormalGame::animate(frame);

	
	//if (!mouse_sprite)		// if the mouse is hidden
	show_mouse(window->surface);

	BITMAP *tmp = screen;
	screen = frame->surface;
	testmenu->animate();			// in principle, drawing operations shouldn't go here.

	meleemenu->animate();
	screen = tmp;
	//if (tb->stop())
	//	this->quit("bye");
	
}







REGISTER_GAME(TWgui, "TWgui")

