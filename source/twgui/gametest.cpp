
/*
TIMEWARP

- voor de ship-selection
- always random
- commentaar bovenop (player #)

- en een Normalgame in-game control panel erbij !!
- oftewel, een volledig functioneel in-game menu-systeem !!!
- in een melee-type sub-game
- (joepie!!)

- shortcut toetsen laten werken - ook de pijltjes toetsen ...

- automatisch een ander menu openen (soft touch optie).

- only works for 32 bpp color depth ?!
*/

// Three distinctions: 1. area detection and definition, 2. bitmap feedback
// (possibly 3: you can define very specific button classes with all their data
// in there)

// This makes (imo) best use of classes and their
// overloading (specialization) abilities. I think that's the most general way and can support
// a wide variety of implementations.

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

//#include "../melee/mcbodies.h"
//#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mitems.h"
//#include "../melee/mfleet.h"   //fleets...

#include "../melee/mcbodies.h"

#include "../scp.h"
#include "../util/history.h"

#include "gametest.h"
#include "twgui.h"
//#include "GamePlanetView.h"

#include "../scp.h"
#include "../melee/mfleet.h"

#include "../frame.h"



/*
class ShipPanelToggeable : public ShipPanel
{
protected:
	int *toggle;
public:
	ShipPanelToggeable(Ship *s, int *t, Game *g);
	virtual void animate(Frame *space);
};


ShipPanelToggeable::ShipPanelToggeable(Ship *s, int *t, Game *g)
:
ShipPanel(s)
{
	toggle = t;

	// a bit weird ... now you got a buffer writing to a buffer before it's finally
	// written to the screen... I think ;)
	this->window->surface = g->view->frame->surface;
}

void ShipPanelToggeable::animate(Frame *space)
{
	if (*toggle)
		ShipPanel::animate(space);
}
*/


class WedgeIndicatorToggable : public WedgeIndicator
{
public:
	WedgeIndicatorToggable(SpaceLocation *target, int length, int color, int *atoggle);
	int *toggle;

	virtual void animate(Frame *space);
	virtual void calculate();
};

WedgeIndicatorToggable::WedgeIndicatorToggable(SpaceLocation *target, int length, int color, int *atoggle)
:
WedgeIndicator(target, length, color)
{
	toggle = atoggle;
}

void WedgeIndicatorToggable::animate(Frame *space)
{
	if (*toggle)
		WedgeIndicator::animate(space);
}

void WedgeIndicatorToggable::calculate()
{
	//if (*toggle)
	// better not disable the calculation, cause you'd lose your wedge target if you do !!
	WedgeIndicator::calculate();
}


// W = max width allowed; anything above is scaled down ...
BITMAP ** load_fleet_bmps(Fleet *fleet, int W)
{
	BITMAP **fleet_bmp;

	int i;
	ShipType	*type;
	DATAFILE	*data; //, *data_cached;
	
	
	fleet_bmp = new BITMAP* [fleet->size+1];

	for ( i = 0; i < fleet->size; ++i )
	{
		type = fleet->ship[i];
		
		DATAFILE *alldata = NULL;

		char fname[512];
		RGB *ptest = 0;

		// hmmm, ship names can have non-standard format ...
		if (type && type->data)
		{


			// first, test if the bitmap is already chached:

			strcpy(fname, "cache/");
			strcat(fname, type->name);//type->data->file);
			strcat(fname, ".bmp");

			BITMAP *btest;
			btest = load_bmp(fname, ptest);

			if (btest)
			{
				BITMAP *bmp = btest;
				
				fleet_bmp[i] = create_bitmap_ex(32, W, W);
				clear_to_color(fleet_bmp[i], makecol(255,0,255));
				
				if ( bmp->w > W)
				{
					// big sprites must be made smaller
					stretch_blit(bmp, fleet_bmp[i], 0, 0, bmp->w, bmp->h, 0, 0, W, W);
					
				} else {
					
					int dx, dy;
					dx = (W - bmp->w)/2;
					dy = (W - bmp->h)/2;
					blit(bmp, fleet_bmp[i], 0, 0, dx, dy, bmp->w, bmp->h);
				}

				continue;
				// skip the datafile checks ...
			}


			// OTHERWISE,
			// load the bitmap from scratch, from the (more general) ship data file:

			// find the 1st ship name (doh)
			alldata = load_datafile(type->data->file);
			if (alldata)
			{
				int k;
				// well, standard, the 1st (and sometimes only) ship-rle is on 8th position
				// hmm, well, almost always - depends on how many panel bitmaps there are ...

				char *info = (char*) alldata[0].dat;	// the info dat header
				set_config_data(info, alldata[0].size);
				k = get_config_int("Objects", "PanelBitmaps", 7);

				data = &alldata[k+1];
			}
			else
				data = 0;
		}
		else
			data = 0;
		
		if (data)
		{
			BITMAP *bmp = NULL;
			if (data->type == DAT_RLE_SPRITE)
			{
				RLE_SPRITE* rle = (RLE_SPRITE*) data->dat;
				BITMAP *bmp2 = create_bitmap_ex(rle->color_depth, rle->w, rle->h);
				clear_to_color(bmp2, makecol_depth(rle->color_depth, 255,0,255));	// needed, cause rle sprites are drawn transparent by default
				draw_rle_sprite(bmp2, rle, 0, 0);
				// change into 32 bits now?
				bmp = create_bitmap_ex(32, bmp2->w, bmp2->h);
				blit(bmp2, bmp, 0, 0, 0, 0, bmp2->w, bmp2->h);
				destroy_bitmap(bmp2);
			} else if (data->type == DAT_BITMAP)	// useful in case you prefer a panel bmp
			{
				bmp = (BITMAP*) data->dat;
			} else
			{
				tw_error("Unknown data type");
			}

			fleet_bmp[i] = create_bitmap_ex(32, W, W);
			clear_to_color(fleet_bmp[i], makecol(255,0,255));

			if ( bmp->w > W)
			{
				// big sprites must be made smaller
				masked_stretch_blit(bmp, fleet_bmp[i], 0, 0, bmp->w, bmp->h, 0, 0, W, W);

			} else {

				int dx, dy;
				dx = (W - bmp->w) / 2;
				dy = (W - bmp->h) / 2;
				masked_blit(bmp, fleet_bmp[i], 0, 0, dx, dy, bmp->w, bmp->h);
			}

			// also, write a copy of this (a cached bmp) to disk:

			RGB *ptest = 0;
			int ires = save_bmp(fname, fleet_bmp[i], ptest);
			if (ires)
			{
				tw_error("Could not cache bmp data");
			}

			if (data->type == DAT_RLE_SPRITE)
				destroy_bitmap(bmp);
			//unload_datafile_object(data);
		} else {
			if (i == 0)
				fleet_bmp[i] = 0;
			else
				fleet_bmp[i] = fleet_bmp[i-1];
			tw_error("could not load ship data");
		}

		if (alldata)
			unload_datafile( alldata );
	}

	fleet_bmp[fleet->size] = 0;		// indicates the last element.


	return fleet_bmp;
}


void remove_fleet_bmps(BITMAP **fleet_bmp)
{
	int i;
	i = 0;
	while (fleet_bmp[i])
	{
		++i;
		destroy_bitmap(fleet_bmp[i]);
	}

	delete fleet_bmp;
}








void TWgui::init(Log *_log)
{
	NormalGame::init(_log);

	view->window->locate(		// 1=in pixels, a fixed amount; 2=in screen width
			0, 0,		//double x1, double x2
			0, 0,		//double y1, double y2
			0, 1,		//double w1, double w2
			0, 1		//double h1, double h2
			);

	// ------- add the menu items ---------

	unscare_mouse();
	show_mouse(window->surface);


	// which font to use ... that depends on the screen resolution:
	int i = 2;
	if (screen->w == 640)
		i = 2;
	if (screen->w == 800)
		i = 3;
	if (screen->w == 1024)
		i = 4;					// more pixels available for the same "real-life" size.

	FONT *usefont = videosystem.get_font(i);

	this->view->frame->prepare();


	char menu_filename[512];
	set_config_file("client.ini");
	strcpy(menu_filename, get_config_string("Menu", "skin", menu_filename));


	meleemenu = new AreaReserve("MELEE", 50, 450, menu_filename, this->view->frame->surface);


	screenswitch = new SwitchButton(meleemenu, "VIEWSCREEN", -1, -1, mapkey(KEY_R, KEY_LCONTROL));

	healthbarswitch = new SwitchButton(meleemenu, "HEALTHBARS", -1, -1, mapkey(KEY_H, KEY_ALT));
	wedgeindicatorswitch = new SwitchButton(meleemenu, "WEDGEINDIC", -1, -1, mapkey(KEY_W, KEY_ALT));
	captainpanelswitch = new SwitchButton(meleemenu, "CPTPANEL", -1, -1, mapkey(KEY_C, KEY_ALT));
	chatlogswitch = new SwitchButton(meleemenu, "CHATLOG", -1, -1, mapkey(KEY_C, KEY_ALT));


	zoomin = new Button(meleemenu, "ZOOMIN", -1, -1, KEY_EQUALS, 1);

	zoomout = new Button(meleemenu, "ZOOMOUT", -1, -1, KEY_MINUS, 1);

	abort = new Button(meleemenu, "ABORT", -1, -1, mapkey(KEY_A, KEY_LCONTROL));

	extraplanet = new Button(meleemenu, "EXTRAPLANET", -1, -1, mapkey(KEY_P, KEY_LCONTROL));

	snapshot = new Button(meleemenu, "SNAPSHOT", -1, -1, mapkey(KEY_S, KEY_LCONTROL));

	callhelp = new Button(meleemenu, "CALLHELP", -1, -1, mapkey(KEY_H, KEY_LCONTROL));

	viewmode = new Button(meleemenu, "VIEWMODE", -1, -1, mapkey(KEY_V, KEY_LCONTROL));

	text_edit = new TextEditBox(meleemenu, "TEXT", -1, -1, usefont);	// text I/O ... experimental !!


	meleemenu->doneinit();	// no more buttons are added, so we can throw away the data file(s) used for this initialization (datafile and search_bitmap).

	// experimental, define an alert menu:
	alertmenu = new AreaReserve("ALERT",  100, 100, menu_filename, this->view->frame->surface);
	alertmenu->center(400, 300);		// center around this position, in relative coordinates

	alert_yes = new Button(alertmenu, "YES", -1, -1, KEY_Y);

	alert_no = new Button(alertmenu, "NO", -1, -1, KEY_N);

	alert_text = new TextButton(alertmenu, "TEXT", -1, -1, usefont);

	alertmenu->doneinit();
	alertmenu->hide();



	popupviewmode = new PopupList(viewmode, "POPUPLIST", -50, -50, menu_filename, usefont, 0 );
	popupviewmode->tbl->set_optionlist(view_name, num_views, makecol(0,0,0));
	



	// a little help menu
	// copied from show_file()
	int L;
	L = file_size("ingame.txt");
	if (L < 0) L = 0;
	char *helptext = new char [L+1];
	PACKFILE *f;
	f = pack_fopen ("ingame.txt", F_READ);
	if (f)
	{
		L = pack_fread (helptext, L, f);
		pack_fclose(f);
		helptext[L] = 0;
	} else
		helptext[0] = 0;

	popup2 = new PopupTextInfo(callhelp, "POPUPHELP", -50, -50, menu_filename, usefont, helptext, L+1);
	// the routine makes a copy of the text, so you can delete the original
	delete helptext;



	if (!reference_fleet)
	{
		tw_error("reference fleet is not available !!");
	}
	BITMAP **bmplist;
	//bmplist = load_fleet_bmps(reference_fleet, 100);
	bmplist = 0;

	// this is invoked in a separate routine, and uses no trigger (=0).
	fleetsel = new PopupFleetSelection("POPUPFLEET", -50, -50, menu_filename,
					this->view->frame->surface,
					bmplist, 1.0, usefont);




	winman = new WindowManager;
	winman->add(meleemenu);
	winman->add(alertmenu);
	winman->add(popupviewmode);
	winman->add(popup2);

//	winman->add(fleetsel);	// hmm, needed or not ?


	// some other stuff

	option.healthbar = 1;
	option.wedge = 1;
	option.captain = 1;

	for ( i = 0; i < 16; ++i )
	{
		always_random[i] = false;	// always randomly select ships - prevents menu from popping up
		player_panel[i] = 0;
	}
}




bool TWgui::handle_key(int k)
{
	// toggle the menu on or off, by pressing the space bar.
	if ((k >> 8) == KEY_SPACE)
	{
		if (meleemenu->hidden)
			meleemenu->show();
		else
			meleemenu->hide();
		return true;
	}

	return NormalGame::handle_key(k);
}


void TWgui::calculate()
{

	NormalGame::calculate();



	// ------------ INTERPRET RESULTS FROM THE MELEE WINDOW ---------------


	FULL_REDRAW = true;
	// only a full_redraw draws the whole frame-surface onto the physical screen.
	// (otherwise only the small sprites are drawn there ... I think)
	// I guess this can be a bit slow, but, I don't know why it's slow... perhaps simply
	// because it's a big screen, or is it because of conversion between color depths?
	// or something else?

	winman->calculate();

	if (zoomin->flag.left_mouse_hold)
	{
		// copied this from view_hero :
		//if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
		//if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
		view->camera.z /= 1 + 0.002 * frame_time;
	}
	if (zoomout->flag.left_mouse_hold)
	{
		view->camera.z *= 1 + 0.002 * frame_time;
	}

	// create a snapshot of the screen ...
	if (snapshot->flag.left_mouse_press)
	{
		// copied from game::handle_key
		pause();
		save_screenshot();
		unpause();
	}


	// create an extra planet
	if (extraplanet->flag.left_mouse_press)
	{
		// copied from normalgame::handle_key()
		//if (log->type == Log::log_normal)
		//{
			message.out("MUHAHAHAHAHA!!!!", 5000, 12);
			create_planet(random(size));
			//add(new Planet(random(size), planetSprite, random(planetSprite->frames())));
		//}
	}


	if (abort->flag.left_mouse_press)
	{
		meleemenu->focus2other(alertmenu);
		alert_text->set_text("Quit game?", makecol(50,50,50));
	}

	option.healthbar = healthbarswitch->state;
	option.wedge = wedgeindicatorswitch->state;
	option.captain = captainpanelswitch->state;


	// ------------ INTERPRET RESULTS FROM THE ALERT WINDOW ---------------
	// (if the alert window is active, that is).


	// can an overall menu return a value ??
	if (alert_yes->flag.left_mouse_press)
		game->quit("quit - Game aborted from keyboard");

	if (alert_no->flag.left_mouse_press)
	{
		alertmenu->back2other();
		FULL_REDRAW = true;		// redraw the whole scene next iteration.
	}

	
	// check if the viewmode should change:
	if (popupviewmode->returnvalueready)
	{
		int imode;
		imode = popupviewmode->getvalue();
		//message.print(1500, 15, "imode = %i", imode);

		if (imode >= 0 && imode < num_views)
		{
			char *name = popupviewmode->tbl->optionlist[imode];
			// change the view, if needed
			if (get_view_num(view->type->name) != get_view_num(name))
				change_view(name);
		}
	}

}


void TWgui::animate(Frame *frame)
{
	/*
	BITMAP *tmp;
	tmp = create_bitmap_ex(bitmap_color_depth(screen), screen->w, screen->h);
	//clear_to_color(tmp, makecol(50,100,150));		// this takes about 2 ms.
	//blit(tmp, screen, 0, 0, 0, 0, tmp->w, tmp->h);	// this takes a long time !!
	memcpy(screen->dat, tmp->dat, tmp->w * tmp->h);
	destroy_bitmap(tmp);
	*/
//	clear_to_color(screen, makecol(50,100,150));		// this takes 1 to 2 ms.

	NormalGame::animate(frame);

	// for this to work, you need a full_redraw.
	// (otherwise, you may need to create dirty items of the menu bitmaps... or something)
	// I think those write directly to screen memory, and since these bitmaps are big,
	// you can get some annoying flicker.

	show_mouse(window->surface);
	//show_mouse(frame->surface);
	winman->setscreen(view->frame->surface);
	winman->animate();

	// and the ship panels (which I've put in here ...
	animate_stuff(frame);
}



// copied and modified from gflmelee ...
void TWgui::animate_stuff(Frame *frame)
{

	if (option.captain)
	{
		int ix, iy, iw, ih;
		BITMAP *src;
		BITMAP *dest = frame->surface;
		
		// show the ship panel(s)
		Ship		*s;
		SpaceObject	*t;
		int		i;
		
		for ( i = 0; i < num_players; ++i )
		{
			
			if ( is_local(player_control[0]->channel) )
			{
				
				s = player_control[i]->ship;
				
				if (s)
				{
					if (!player_panel[i])
						player_panel[i] = new ShipPanelBmp(s);
					
					if (player_panel[i]->ship != s)
						player_panel[i]->ship = s;
					
					
					player_panel[i]->animate_panel();
				}
				
				// drawing the panel bitmap onto the frame
				if (player_panel[i])	// needed in case game (ships) isn't initialized yet...
				{
					src = player_panel[i]->panel;
					
					ix = dest->w - src->w;
					iy = 0;
					iw = src->w;
					ih = src->h;
					
					blit(src, dest, 0, 0 , ix, iy, iw, ih);
					frame->add_box(ix, iy, iw, ih);
					
					i = 0;
					t = player_control[i]->target;
					
					i = 1;
					if (t && t->exists() && t->isShip() )
					{
						if (!player_panel[i])
							player_panel[i] = new ShipPanelBmp((Ship*) t);
						
						if (player_panel[i]->ship != t)
							player_panel[i]->ship = (Ship*) t;
						
						
						if (player_panel[i])
						{
							
							player_panel[i]->animate_panel();
							
							src = player_panel[i]->panel;
							
							ix = dest->w - src->w;
							iy = dest->h - src->h;
							iw = src->w;
							ih = src->h;
							blit(src, dest, 0, 0 , ix, iy, iw, ih);
							frame->add_box(ix, iy, iw, ih);
							
						}
					}
				}
				
			}
			
		}

	}
}


void TWgui::animate_predict(Frame *frame, int time)
{

	NormalGame::animate_predict(frame, time);
	animate_stuff(frame);

}

void TWgui::animate()
{

	NormalGame::animate();

}









void TWgui::choose_new_ships()
{
	// check if there is only 1 nonzero fleet left
	// in that case, the game should end ...
	int i, k, m = 0;
	k = 0;
	for ( i = 0; i < num_players; ++i )
	{
		if (player_fleet[i]->size != 0)
		{
			++k;
			m = i;
		}
	}
	if (k == 1)
	{
		message.print(1500, 15, "The game ends in victory for player %s", player_name[m]);
		this->quit("ending game");
	}

	char tmp[40];
	pause();
	message.out("Selecting ships...", 1000);
	int *slot = new int[num_players];
	//choose ships and send them across network
	for (i = 0; i < num_players; i += 1) {
		slot[i] = -2;
		if (player_control[i]->ship) {
			}
		else {

			bool oncerandom = false;

			if (!always_random[i])
			{
				//			if (player_panel[i]) player_panel[i]->window->hide();
				//			player_panel[i] = NULL;
				sprintf (tmp, "Player%d", i+1);
				Fleet *fleet = player_fleet[i];
				if (fleet->size == 0) continue;
				char buffy[512];
				sprintf(buffy, "%s\n%s\n%d of ??? points", player_name[i], fleet->title, fleet->cost);
				
				
				// -------- CHOOSE A SHIP -----------
				// (why is that stuffed in class "Control" ??)
				//slot[i] = player_control[i]->choose_ship2(window, buffy, fleet);
				BITMAP **ico = load_fleet_bmps(fleet, 100);	// 100 = default width
				fleetsel->newscan(ico, 0.8, player_name[i]);
				fleetsel->setscreen(screen);
				show_mouse(screen);
				unscare_mouse();
				for (;;)
				{
					idle();
					
					fleetsel->calculate();
					fleetsel->animate();
					
					if (fleetsel->returnvalueready)
						break;
					
					if (fleetsel->oncerandom->flag.left_mouse_press)
					{
						oncerandom = true;
						fleetsel->oncerandom->flag.left_mouse_press = false;
						break;
					}
					
					if (fleetsel->alwaysrandom->flag.left_mouse_press)
					{
						always_random[i] = true;
						break;
					}
					
					
					
				}

				fleetsel->hide();
				remove_fleet_bmps(ico);	// remove the bitmaps from memory

			}

			if (oncerandom || always_random[i])
				slot[i] = -1;
			else
				slot[i] = fleetsel->getvalue();

			if (player_control[i]->channel != channel_none) {
				slot[i] = intel_ordering(slot[i]);
				log->buffer(player_control[i]->channel, &slot[i], sizeof(int));
				log->flush();
				//slot[i] = intel_ordering(slot[i]);
				}
			}
		}
	//recieve the ships that were chosen
	log->listen();
	for (i = 0; i < num_players; i += 1) {
		if (slot[i] == -2) continue;
		if (player_control[i]->channel != channel_none) {
			log->unbuffer(player_control[i]->channel, &slot[i], sizeof(int));
			slot[i] = intel_ordering(slot[i]);
			}
		}
	//create the ships that were chosen
	for (i = 0; i < num_players; i += 1) {
		if (slot[i] == -2) continue;
		sprintf (tmp, "Player%d", i+1);
		//fleet->load("./fleets.tmp", tmp);
		Fleet *fleet = player_fleet[i];
		if (slot[i] == -1) slot[i] = random() % fleet->size;
		Ship *s = create_ship(fleet->ship[slot[i]]->id, player_control[i], random(size), random(PI2), player_team[i]);
		fleet->clear_slot(slot[i]);
		fleet->sort();
		//fleet->save("./fleets.tmp", tmp);
		s->locate();
		add ( new WedgeIndicatorToggable ( s, 30, i+1, &option.wedge ) );

		// don't add a ship panel ... that's done later on, if needed.

		add(s->get_ship_phaser());

		// add a healthbar to the ship.
		add(new HealthBar(s, &option.healthbar));
		}
	delete slot;
	message.out("Finished selecting ships...", 1500);
	unpause();


	return;
}








REGISTER_GAME(TWgui, "TWgui")

