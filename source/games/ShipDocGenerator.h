/** 
ShipDocGenerator.h
$Id$

  Class definition of the "ShipDocGenerator" class.  This class
  looks at each ship type, and for each, writes out its stats,
  a summary, and creates a snapshot of the ship.  To make the
  snapshot, it creates a game type, and brings the ship in, then
  exports a properly named and sized picture.  Someday, it'll export
  several, and make them into an animated GIF. 
  
  Revision history:
    2004.06.28 yb started
	
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

#ifndef SHIP_DOC_GENERATOR_HEADER
#define SHIP_DOC_GENERATOR_HEADER 1


#include <allegro.h> //allegro library header

#include "../melee.h"          //used by all TW source files.  well, mostly.  
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "../melee/mgame.h"    //Game stuff
#include "../melee/mcbodies.h" //asteroids & planets
#include "../melee/mship.h"    //ships
#include "../melee/mshot.h"   //shots, missiles, lasers
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mview.h"    //Views & messages
#include "../melee/mshppan.h"  //ship panels...

#include "../melee/mitems.h"  //ship panels...
#include "../melee/manim.h"  //ship panels...
#include "../melee/mview.h"

#include "../melee/mcontrol.h"
#include "../melee/mfleet.h"
#include "../melee/mcbodies.h"
#include "../frame.h"

#include <stdio.h>

static Ship * ship = NULL;

enum { timeBeforeScreenshots = 4000 };
enum { timeBetweenScreenshots = 10 };
enum { numberOfScreenShots = 30 };
//enum { numberOfScreenShots = 2 };

enum { screenshotWidth = 100 };
enum { screenshotHeight = 100 };

class ViewZoomRightIn : public View {

public:

	virtual void calculate (Game *game) {STACKTRACE
		CameraPosition n = camera;
	    n.z = 1000;

		focus ( &n, ship );
		track ( n, frame_time );
		return;
	}

};


class GoStraightControl : public Control
{
public:
	GoStraightControl() :
      Control("GoStraightControl", channel_server)
	  {
	  }
    virtual int think() {return keyflag::thrust;}

	virtual const char *getTypeName() {return "GoStraightControl";}
};


class RepeatingStars : public Stars {
public:
	RepeatingStars() :
      Stars()
	{
	}

	/*virtual void animate(Frame *space) {STACKTRACE
		if (v && (space_view != *v)) return;

		double d = space_zoom;
		//double w = width * d;
		//double h = height * d;

		double w = screenshotWidth*4 * d;
		double h = screenshotHeight*4 * d;

		int x, y, layer;
		for (layer = 0; layer < num_layers; layer += 1) {
			d = space_zoom * pow(1.0 - field_depth / 260.0, (num_layers-layer)/(double)num_layers);
			for (y = 0; y * h < space->surface->h; y+=1) {
				for (x = 0; x * w < space->surface->w; x+=1) {
					if (aa_mode & 0x80000000) {
						_draw_starfield_cached ( 
							space, 
							pic[layer%num_pics], 
							0,
							num_stars / num_layers,
							(x+0.5)*w, (y+0.5)*h, 
							-space_center_nowrap.x * d + space_view_size.x / 2, 
							-space_center_nowrap.y * d + space_view_size.y / 2, 
							w,h, 
							w,h, 
							d + d*fabs(50.0-(((game->game_time / 10 + layer * 70)) % 100)) / 100.0,
							seed + layer,
							aa_mode
						);
					}
					else {
						_draw_starfield_raw ( 
							space, 
							pic[layer%num_pics], 0, 1,
							num_stars / num_layers,
							(x+0.5)*w, (y+0.5)*h, 
							-space_center_nowrap.x * d + space_view_size.x / 2, 
							-space_center_nowrap.y * d + space_view_size.y / 2, 
							w, h, 
							w, h, 
							d + d*fabs(50.0-(((game->game_time / 10 + layer * 70)) % 100)) / 100.0,
							seed + layer,
							aa_mode & 0x7fffFFFF
						);
					}
				}
			}
		}
		return;
	}/**/

};

/** Creates documentation for each ship type.  */
class ShipDocGenerator : public Game {


public:
	ShipDocGenerator() :
	  Game()
	{
        state = PICK_NEXT_SHIP;
		currentScreenShot = 0;
	}

	virtual void init(Log *_log)
	{
		Game::init(_log);
		prepare(); 

		Game::set_resolution(640,480);
		redraw();

		change_view(new ViewZoomRightIn());

		//add the starscape background
		add(new RepeatingStars());
		autoBots = new_team();

		::remove("scrshots/output.html");

	    FILE * fileshipNames = fopen("scrshots/shipnames.txt", "w");
		char shipNames[5000] = "";

		for (int i=0; i<num_shiptypes; i++) {
			ShipType * shiptype = &shiptypes[i];
			if (shiptype) {

				strcat(shipNames, shiptype->name);
				strcat(shipNames, "\n");

				char * space = NULL;
				while ((space = strchr(shipNames,' ')) != NULL) {
					space[0] = '_';
				}
			}
		}
		fwrite(shipNames,sizeof(char), strlen(shipNames),fileshipNames);
		fclose(fileshipNames);/**/

		//message.out(shipNames);
	}

	

	virtual void calculate() {
		Game::calculate();

		static myTime = 0;

		myTime -= frame_time;

		if (myTime <=0) {

			if (state==WAIT_FOR_ACCEL) {
				state = TAKE_SCREEN_SHOTS;
			}

			if (state == TAKE_SCREEN_SHOTS) {

				if (currentScreenShot < numberOfScreenShots) {
					if (ship && ship->type) {
						save_screenshot(ship->type->name, currentScreenShot);
					}
				}
				else {
					state = PICK_NEXT_SHIP;
				}

				myTime = timeBetweenScreenshots;
				currentScreenShot++;
			}

			if (state== PICK_NEXT_SHIP) {
				myTime = timeBeforeScreenshots;
				currentScreenShot = 0;
				state = WAIT_FOR_ACCEL;

				if (ship != NULL) {
					ship->die();
					remove(ship);
				}

				static int shipCounter = 0;

				ShipType * shiptype = &shiptypes[shipCounter];
				ASSERT(shiptype);
				
				shipCounter ++;
				if (shipCounter >= num_shiptypes) {
					char msg[100];
					sprintf(msg, "Done saving %i screenshots of each of %i ships.", numberOfScreenShots, shipCounter);
					this->quit(msg);
				}

				Control * control = new GoStraightControl();;
				add(control);
				

				ship = create_ship(shiptype->id,
					control,
					Vector2(
					rand()%(int)size.x,
					rand()%(int)size.y),
					PI2 - PI/8,
					autoBots);
				
				add(ship);
				add_focus(ship);

				dumpShipStats("scrshots/output.html");
			}
		}
	}

	virtual void save_screenshot(const char * filename, int i) {STACKTRACE
		char path[80];
	    
	    pause();

		sprintf(path, "./scrshots/%s-%02i.bmp", filename, i);

	    //replace ' ' characters with '_'
		char * space = NULL;
		while ((space = strchr(path,' ')) != NULL) {
			space[0] = '_';
		}

		BITMAP *bmp;
		PALETTE pal;
		get_palette(pal);
		//bmp = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H);
		bmp = create_sub_bitmap(screen,
			                    (SCREEN_W-screenshotWidth)/2,(SCREEN_H-screenshotHeight)/2,
			                    screenshotWidth,screenshotHeight);
		//save_bitmap(path, bmp, pal);
		save_bmp(path, bmp, pal);
		destroy_bitmap(bmp);

		unpause();
		return;
	}

	virtual void dumpShipStats(const char * filename) {
		
		ShipData * data = ship->type->data;
		ShipType * type = ship->type;
		DATAFILE * datafile = data->data;
		
		ASSERT(data);
		ASSERT(type);
		ASSERT(datafile);
		
		char stats[7000] = "";

		char textDesc[5000] = "";
		

		{
			FILE * descFile = fopen(type->text, "r");
			if (descFile != NULL) {
				
				::fseek (descFile , 0 , SEEK_END);
				long lSize = ::ftell (descFile);
				::rewind (descFile);
				::fread(textDesc,1,lSize,descFile);
				::fclose(descFile);
			}
		}

		char imageFilename[2000] = "";
		sprintf(imageFilename, "%s.gif", ship->type->name);
		char * space = NULL;
		while ((space = strchr(imageFilename,' ')) != NULL) {
			space[0] = '_';
		}


        //pardon the bizzre syntax, I'll clean it up later (yb)		
		sprintf(stats,
"<a name=\"%s\"><h2>%s</h2></a> \
<img src=\"%s\" style=\"float:right;\"/>\
<table>\
<tr><th>Max Crew</th><td>%i</td></tr>\
<tr><th>Max Battery</th><td>%i</td></tr>\
<tr><th>Battery Recharge</th><td>%i</td></tr>\
<tr><th>Turning</th><td>%.8lf</td></tr>\
<tr><th>Acceleration</th><td>%.8lf</td></tr>\
<tr><th>Max Speed</th><td>%.8lf</td></tr>\
</table>\
%s\
"
,
			type->name,       //a tag
			type->name,       //h2 tag
			imageFilename,    //img tag

			(int)ship->crew_max,
			(int)ship->batt_max,
			ship->recharge_amount * ship->recharge_rate,

			ship->turn_step * ship->turn_rate,
			ship->accel_rate,
			ship->speed_max,

			textDesc          //after table
		);
		FILE * file = fopen(filename, "a");
		fwrite(stats, 1, strlen(stats), file);
		//fprintf(file, stats);
		fclose(file);


		/*		double crew;
		double crew_max;
		double batt;
		double batt_max;
		double turn_rate;
		double turn_step;
		double speed_max;
		double accel_rate;
		int    recharge_amount;
		int    recharge_rate;
		int    weapon_drain;
		int    weapon_rate;
		int    weapon_sample;
		int    special_drain;
		int    special_rate;
		int    special_sample;
		int    hotspot_rate;
		char   captain_name*/
	}


protected:
	TeamCode autoBots;
	enum { PICK_NEXT_SHIP=0, WAIT_FOR_ACCEL, TAKE_SCREEN_SHOTS } state;
	int currentScreenShot;

};

REGISTER_GAME(ShipDocGenerator, "ShipDocumentationGenerator")
//REGISTER_VIEW ( ViewZoomRightIn, "ViewZoomRightIn" )


#endif