
#include <allegro.h>

#include "../gui.h"
#include "../melee.h"
#include "menugeneral.h"

#include "../melee/mfleet.h"
#include "../melee/mship.h"



#define FLEET_TITLE_DIALOG_BOX    0
#define FLEET_TITLE_DIALOG_EDIT   1
#define FLEET_TITLE_DIALOG_OK     2
#define FLEET_TITLE_DIALOG_CANCEL 3

char title_str[80];

DIALOG fleet_titleDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        180,  210,  280,  60,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_edit_proc,       190,  220,  260,  10,   255,  0,    0,    0,       80,   0,    (void *) title_str, NULL, NULL },
  { my_d_button_proc,  255,  240,  60,   18,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"OK", NULL, NULL },
  { d_button_proc,     325,  240,  60,   18,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Cancel", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

// FLEET - dialog objects
enum {
    //FLEET_DIALOG_CLEAR_SCREEN=0,
    FLEET_DIALOG_AVAILABLE_SHIPS_TEXT=0,
    FLEET_DIALOG_SHIP_CATAGORIES_TEXT,
    //TEMP,
    FLEET_DIALOG_SC1_TOGGLE,
    FLEET_DIALOG_SC2_TOGGLE,
    FLEET_DIALOG_SC3_TOGGLE,
    FLEET_DIALOG_TW_OFFICIAL_TOGGLE,
    FLEET_DIALOG_TW_EXP_TOGGLE,
    FLEET_DIALOG_TW_SPECIAL_TOGGLE,
    FLEET_DIALOG_SORTBY_TEXT1,
    FLEET_DIALOG_SORTBY_BUTTON1,
    FLEET_DIALOG_SORTBY_ASCENDING1,
    FLEET_DIALOG_AVAILABLE_SHIPS_LIST,
    FLEET_DIALOG_FLEET_SHIPS_LIST,
    FLEET_DIALOG_PLAYER_FLEET_BUTTON_INC,
    FLEET_DIALOG_PLAYER_FLEET_BUTTON_DEC,
    FLEET_DIALOG_PLAYER_FLEET_BUTTON,
    FLEET_DIALOG_PLAYER_FLEET_TITLE,
    FLEET_DIALOG_SAVE_BUTTON,
    FLEET_DIALOG_LOAD_BUTTON,
    FLEET_DIALOG_POINT_LIMIT_TEXT,
    FLEET_DIALOG_POINT_LIMIT_BUTTON,
    FLEET_DIALOG_CURRENT_POINTS_TEXT,
    FLEET_DIALOG_CURRENT_POINTS_VALUE,
    FLEET_DIALOG_SORTBY_TEXT2,
    FLEET_DIALOG_SORTBY_BUTTON2,
    FLEET_DIALOG_SORTBY_ASCENDING2,
	FLEET_DIALOG_ADD_BUTTON,
    FLEET_DIALOG_ADD_ALL_BUTTON,
	FLEET_DIALOG_CLEAR,
    FLEET_DIALOG_CLEARALL,
    FLEET_DIALOG_SHIP_PICTURE_BITMAP,
    //FLEET_DIALOG_SHIP_STATS_BITMAP,
    FLEET_DIALOG_SHIP_SUMMARY_TEXT,
    FLEET_DIALOG_BACK_BUTTON,
    FLEET_DIALOG_HELP_TEXT/**/
};


char *numeric_string[] = {"Zero", "One", "Two", "Three", "Four", 
		"Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven", 
		"Twelve"};

char fleetPlayer[18];
char fleetTitleString[100];

int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c);

int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c);



int d_check_proc_fleeteditor(int msg, DIALOG *d, int c)
{
	if (msg == MSG_CLICK)
	{
		
		/* track the mouse until it is released */
		while (gui_mouse_b()) {
//			state2 = ((gui_mouse_x() >= d->x) && (gui_mouse_y() >= d->y) &&
//				(gui_mouse_x() < d->x + d->w) && (gui_mouse_y() < d->y + d->h));
			
			/* let other objects continue to animate */
			broadcast_dialog_message(MSG_IDLE, 0);
		}
		
		/* should we close the dialog? */
		// imo the following mucho better/ simplere than that messy stuff in the allegro routine
		// ... check d_button_proc in guiproc.c in the allegro sources...

		if (d->flags & D_SELECTED)
			d->flags &= ~D_SELECTED;
		else
			d->flags |= D_SELECTED;

		if ( d->flags & D_EXIT)
			return D_CLOSE;

		return D_O_K; 
	}
	
	return d_check_proc(msg, d, 0);
}


// FLEET - dialog structure
DIALOG fleetDialog[] = {
  // (dialog proc)     (x)  (y)   (w)   (h)   (fg)(bg)(key) (flags)    (d1)   (d2)        (dp)

  //{ d_clear_proc,       0,    0,    0,    0,   255,  0,    0,    0,       0,    0,    NULL },//FLEET_DIALOG_CLEAR_SCREEN
  { d_textbox_proc,     10,  10,   240,  20,   255,  0,    0,     0,       0,    0,    (void *)"Available Ships", NULL, NULL },//FLEET_DIALOG_AVAILABLE_SHIPS_TEXT TODO specify font here in d2 I think
  { d_textbox_proc,     10,  35,   128,  17,   255,  0,    0,     0,       0,    0,    (void *)"Ship Catagories:", NULL, NULL },//FLEET_DIALOG_SHIP_CATAGORIES_TEXT
  
  { d_check_proc_fleeteditor,	// x=30-->x=10
                        10,  52,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"SC1", NULL, NULL },//FLEET_DIALOG_SC1_TOGGLE
  { d_check_proc_fleeteditor,		
                        10,  66,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"SC2", NULL, NULL },//FLEET_DIALOG_SC2_TOGGLE
  { d_check_proc_fleeteditor,		
                        10,  79,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"SC3", NULL, NULL },//FLEET_DIALOG_SC3_TOGGLE
  { d_check_proc_fleeteditor,		
//                      30,  93,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"TW (Official)", NULL, NULL },//FLEET_DIALOG_TW_OFFICIAL_TOGGLE
                        80,  52,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"TW (Official)", NULL, NULL },//FLEET_DIALOG_TW_OFFICIAL_TOGGLE
  { d_check_proc_fleeteditor,       
//                      30, 107,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Experimental)", NULL, NULL },//FLEET_DIALOG_TW_EXP_TOGGLE
                        80,  66,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Experimental)", NULL, NULL },//FLEET_DIALOG_TW_EXP_TOGGLE
  { d_check_proc_fleeteditor,       
//                      30, 107,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Special)", NULL, NULL },//FLEET_DIALOG_TW_SPECIAL_TOGGLE
                        80,  79,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Special)", NULL, NULL },//FLEET_DIALOG_TW_SPECIAL_TOGGLE

  { d_textbox_proc,     10, 121,    64,  17,   255,  0,    0,     0,       0,    0,    (void *)"Sort By:", NULL, NULL },//FLEET_DIALOG_SORTBY_TEXT1
  { d_button_proc,      69, 121,   128,  17,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Cost", NULL, NULL },//FLEET_DIALOG_SORTBY_BUTTON1
  { d_button_proc,     197, 121,    16,  17,   255,  0,    0,D_EXIT,       0,    0,    (void *)"^", NULL, NULL },//FLEET_DIALOG_SORTBY_ASCENDING1

  { scp_fleet_dialog_text_list_proc,
                       10,  141,   240, 227,   255,  0,    0,D_EXIT,       0,    0,    (void *)shippointsListboxGetter, NULL, NULL },//FLEET_DIALOG_AVAILABLE_SHIPS_LIST

  //{ d_text_list_proc,
    //                   10,  141,   240, 227,   255,  0,    0,D_EXIT,       0,    0,    (void *)shippointsListboxGetter, NULL, NULL },//FLEET_DIALOG_AVAILABLE_SHIPS_LIST
  { d_list_proc2,      390, 141,   240, 227,   255,  0,    0,D_EXIT,       0,    0,    (void *)fleetpointsListboxGetter, NULL, NULL },//FLEET_DIALOG_FLEET_SHIPS_LIST

  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { my_d_button_proc,  390,  10,   8,    20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"+", NULL, NULL },//FLEET_DIALOG_PLAYER_FLEET_BUTTON_INC
  { my_d_button_proc,  602,  10,   8,    20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"-", NULL, NULL },//FLEET_DIALOG_PLAYER_FLEET_BUTTON_DEC
  { my_d_button_proc,  400,  10,   200,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Player 1 Fleet", NULL, NULL },//FLEET_DIALOG_PLAYER_FLEET_BUTTON

  { my_d_button_proc,  390,  40,   128,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Fleet Title", NULL, NULL },//FLEET_DIALOG_PLAYER_FLEET_TITLE
  { my_d_button_proc,  518,  40,    56,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Save", NULL, NULL },//FLEET_DIALOG_SAVE_BUTTON
  { my_d_button_proc,  574,  40,    56,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Load", NULL, NULL },//FLEET_DIALOG_LOAD_BUTTON
  { d_textbox_proc,    390,  60,   128,  20,   255,  0,    0,     0,       0,    0,    (void *)"Point Limit", NULL, NULL },//FLEET_DIALOG_POINT_LIMIT_TEXT
  { my_d_button_proc,  518,  60,   112,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"300\0              ", NULL, NULL },//FLEET_DIALOG_POINT_LIMIT_BUTTON
  { d_textbox_proc,    390,  80,   128,  20,   255,  0,    0,     0,       0,    0,    (void *)"Current Points", NULL, NULL },//FLEET_DIALOG_CURRENT_POINTS_TEXT
  { d_textbox_proc,    518,  80,   112,  20,   255,  0,    0,     0,       0,    0,    (void *)"100\0              ", NULL, NULL },//FLEET_DIALOG_CURRENT_POINTS_VALUE
  { d_textbox_proc,    390, 120,    64,  20,   255,  0,    0,     0,       0,    0,    (void *)"Sort By:", NULL, NULL },//FLEET_DIALOG_SORTBY_TEXT2
  { d_button_proc,     454, 120,   128,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Cost\0             ", NULL, NULL },//FLEET_DIALOG_SORTBY_BUTTON2
  { d_button_proc,     582, 120,    16,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"^", NULL, NULL },//FLEET_DIALOG_SORTBY_ASCENDING2

  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)  
  { my_d_button_proc,  270, 210,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Add", NULL, NULL },//FLEET_DIALOG_ADD_BUTTON
  { my_d_button_proc,  270, 235,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Add All", NULL, NULL },//FLEET_DIALOG_ADD_ALL_BUTTON
  { my_d_button_proc,  270, 265,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Remove", NULL, NULL },//FLEET_DIALOG_CLEAR
  { my_d_button_proc,  270, 290,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Remove All", NULL, NULL },//FLEET_DIALOG_CLEARALL
  { scp_fleet_dialog_bitmap_proc,
                        10, 372,   85,   85,   255,  0,    0,    0,       0,    0,    (void *)NULL, NULL, NULL },//FLEET_DIALOG_SHIP_PICTURE_BITMAP

  //{ d_textbox_proc,     10, 372,  310,   85,   255,  0,    0,     0,       0,    0,    (void *)"SHIP PICTURE TODO", NULL, NULL },//FLEET_DIALOG_SHIP_STATS_BITMAP


  { d_textbox_proc,    325, 372,  305,   85,   255,  0,    0,     0,       0,    0,    (void *)"Summary Text\0                                                                  ", NULL, NULL },//FLEET_DIALOG_SHIP_SUMMARY_TEXT
  { d_button_proc,      10, 460,   64,   20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Back", NULL, NULL },//FLEET_DIALOG_BACK_BUTTON
  { d_textbox_proc,     74, 460,  556,   20,   255,  0,    0,     0,       0,    0,    (void *)"Help Text\0                                                                     ", NULL, NULL },//FLEET_DIALOG_HELP_TEXT

  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,     0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,     0,       0,    0,    NULL, NULL, NULL }/**/
};

bool safeToDrawPreview = false;

static ShipType* showing_shipimage_type = 0;

static int rotationFrame = 0;

void add_ship_image(int k)
{
	// show a new ship image.
	ShipType* type = reference_fleet->getShipType(k);
	
	if (type && type->data)
	{
		type->data->lock();

		if (type->data->spriteShip)
			rotationFrame = 0;//(int)(fractionRotated * type->data->spriteShip->frames());

		showing_shipimage_type = type;
	}
}

void remove_ship_image()
{
	if (showing_shipimage_type)
	{
		// if some image was shown, then you should remove/dereference it.
		
		safeToDrawPreview = false;
		
		ShipType* type = showing_shipimage_type;
		
		if (type && type->data)
		{
			type->data->unlock();

			showing_shipimage_type = 0;
		}
        
	}
}



// FLEET - dialog function
void edit_fleet(int player) {STACKTRACE
	char tmp[40];
	char path[80];
    char fleetCostString[80] = "";
    char maxFleetCostString[80] = "";
    bool availableFleetDirty = true;

	// reset the ship image ?
	showing_shipimage_type = 0;

    static Fleet::SortingMethod sortMethod1 = (Fleet::SortingMethod) Fleet::SORTING_METHOD_DEFAULT,
        sortMethod2 = (Fleet::SortingMethod) Fleet::SORTING_METHOD_DEFAULT;
    static bool sortAscending1 = false,
        sortAscending2 = false;


	sprintf (tmp, "Player%d", player+1);
	Fleet* fleet = new Fleet();
	fleet->load("fleets.ini", tmp);

	if (player + 1 <= 12)
		sprintf(fleetPlayer, "Player %s Fleet", numeric_string[player+1]);
	else sprintf(fleetPlayer, "Player%d Fleet", player+1);
	showTitle();

    int fleetRet;
    int selectedSlot;

    fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].dp = fleetCostString;
    fleetDialog[FLEET_DIALOG_POINT_LIMIT_BUTTON].dp = maxFleetCostString;
    
//	// the reference_fleet is used in the list in a hardcoded way, so over"load" it
//    Fleet *old_reference_fleet = reference_fleet;

	do {
		sprintf(title_str, fleet->getTitle());
		sprintf(fleetTitleString, "%s\n%d points", fleet->getTitle(), fleet->getCost());
        
        fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].dp3 = fleet;
        fleetDialog[FLEET_DIALOG_SORTBY_BUTTON1].dp = Fleet::getSortingMethodName(sortMethod1);
        fleetDialog[FLEET_DIALOG_SORTBY_BUTTON2].dp = Fleet::getSortingMethodName(sortMethod2);

        sprintf(fleetCostString,"%d", fleet->getCost());
        if (fleet->getCost() > fleet->getMaxCost())
            fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].bg = makecol8(255,0,0);
        else
            fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].bg = 0;

        sprintf(maxFleetCostString,"%d %s", fleet->getMaxCost(), 
                                            Fleet::getFleetCostName(fleet->getMaxCost()));

        if (sortAscending1)
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"^";
        else
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"v";
        
        if (sortAscending2)
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"^";
        else
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"v";

        //if the user has selected a different choice of available ships, regenerate the
        //list of available ships
        if (availableFleetDirty) {
            availableFleetDirty = false;
            
            //clear out the fleet
            reference_fleet->reset();

            for (int c=0; c<num_shiptypes; c++) {
                switch (shiptypes[c].origin) {
                case SHIP_ORIGIN_SC1:
                    if (fleetDialog[FLEET_DIALOG_SC1_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_SC2:
                    if (fleetDialog[FLEET_DIALOG_SC2_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_SC3:
                    if (fleetDialog[FLEET_DIALOG_SC3_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_TW_ALPHA:
                    if (fleetDialog[FLEET_DIALOG_TW_OFFICIAL_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_TW_BETA:
                    if (fleetDialog[FLEET_DIALOG_TW_EXP_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;

                case SHIP_ORIGIN_TW_SPECIAL:
                    if (fleetDialog[FLEET_DIALOG_TW_SPECIAL_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                }
            }
            reference_fleet->Sort( sortMethod1, sortAscending1 );
            fleetDialog[FLEET_DIALOG_AVAILABLE_SHIPS_LIST].flags |= D_DIRTY;
        }/**/

		fleetRet = tw_do_dialog(NULL, fleetDialog, -1);

        switch( fleetRet ) {
           case FLEET_DIALOG_AVAILABLE_SHIPS_TEXT: break;
           case FLEET_DIALOG_SHIP_CATAGORIES_TEXT: break;

           case FLEET_DIALOG_SC1_TOGGLE:
           case FLEET_DIALOG_SC2_TOGGLE:
           case FLEET_DIALOG_SC3_TOGGLE:
           case FLEET_DIALOG_TW_OFFICIAL_TOGGLE:
           case FLEET_DIALOG_TW_EXP_TOGGLE:
           case FLEET_DIALOG_TW_SPECIAL_TOGGLE:
               availableFleetDirty = true;
			   break;

           case FLEET_DIALOG_SORTBY_TEXT1: break;
           case FLEET_DIALOG_SORTBY_BUTTON1: 
               sortMethod1 = Fleet::cycleSortingMethod(sortMethod1);
               reference_fleet->Sort( sortMethod1, sortAscending1 );
               fleetDialog[FLEET_DIALOG_SORTBY_BUTTON1].dp = Fleet::getSortingMethodName(sortMethod1);
               break;

           case FLEET_DIALOG_SORTBY_ASCENDING1: 
               sortAscending1 = 1 - sortAscending1;
               reference_fleet->Sort( sortMethod1, sortAscending1 );
               if (sortAscending1)
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"^";
               else
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"v";
               break;

           
           

           case FLEET_DIALOG_AVAILABLE_SHIPS_LIST:
           case FLEET_DIALOG_ADD_BUTTON: 
			   int k;
			   k = fleetDialog[FLEET_DIALOG_AVAILABLE_SHIPS_LIST].d1;
			   if (k < 0 || k >= reference_fleet->getSize()) {tw_error("invalid ship choice - bug");}

               selectedSlot = fleet->addShipType(reference_fleet->getShipType(k));
               if (selectedSlot != -1)
                   fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = selectedSlot;
               
               break;

           case FLEET_DIALOG_PLAYER_FLEET_BUTTON_INC:
		   case FLEET_DIALOG_PLAYER_FLEET_BUTTON_DEC:
			   {

			   sprintf (tmp, "Player%d", player+1);
			   fleet->save("fleets.ini", tmp);
			   delete fleet;
			   
			   if (fleetRet == FLEET_DIALOG_PLAYER_FLEET_BUTTON_INC)
				   ++player;
			   else
				   --player;

			   if (player >= 8)
				   player = 0;
			   if (player < 0)
				   player = 7;
			   
			   sprintf(fleetPlayer, "Player %i Fleet", player+1);
			   fleetDialog[FLEET_DIALOG_PLAYER_FLEET_BUTTON].dp = fleetPlayer;		// dp points to the text string
			   
			   sprintf (tmp, "Player%d", player+1);
			   fleet = new Fleet();
			   fleet->load("fleets.ini", tmp);
			   
			   showTitle();
			   break;
			   }

           case FLEET_DIALOG_PLAYER_FLEET_TITLE:               
               if(do_dialog(fleet_titleDialog, FLEET_TITLE_DIALOG_BOX) == FLEET_TITLE_DIALOG_OK)
                   sprintf(fleet->getTitle(), title_str);
               showTitle();
               break;

           case FLEET_DIALOG_SAVE_BUTTON: 
               sprintf(path, "fleets/");
               if(file_select("Save Fleet", path, "scf")) fleet->save(path, "Fleet");
               showTitle();
               break;

           case FLEET_DIALOG_LOAD_BUTTON: 
               sprintf(path, "fleets/");
               if(file_select("Load Fleet", path, "scf")) fleet->load(path, "Fleet");
               sprintf(title_str, fleet->getTitle());
               sprintf(fleetTitleString, "%s\n%d points", fleet->getTitle(), fleet->getCost());
               showTitle();
               break;

           case FLEET_DIALOG_POINT_LIMIT_TEXT: break;

           case FLEET_DIALOG_POINT_LIMIT_BUTTON:
               fleet->cycleMaxFleetCost();
               break;

           case FLEET_DIALOG_CURRENT_POINTS_TEXT: break;
           case FLEET_DIALOG_CURRENT_POINTS_VALUE: break;
           case FLEET_DIALOG_SORTBY_TEXT2: break;

           case FLEET_DIALOG_SORTBY_BUTTON2: 
               sortMethod2 = Fleet::cycleSortingMethod(sortMethod2);
               fleet->Sort( sortMethod2, sortAscending2 );
               fleetDialog[FLEET_DIALOG_SORTBY_BUTTON2].dp = Fleet::getSortingMethodName(sortMethod2);
               break;

           case FLEET_DIALOG_SORTBY_ASCENDING2: 
               sortAscending2 = 1 - sortAscending2;
               fleet->Sort( sortMethod2, sortAscending2 );
               if (sortAscending2)
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"^";
               else
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"v";
               break;

           case FLEET_DIALOG_ADD_ALL_BUTTON: 
               fleet->addFleet(reference_fleet);
               break;

	       case FLEET_DIALOG_CLEAR: 
           case FLEET_DIALOG_FLEET_SHIPS_LIST: 
			fleet->clear_slot(fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1);
            if (fleet->getSize() <= 0)
                fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = 0;
               break;

           case FLEET_DIALOG_CLEARALL: 
               fleet->reset();
               fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = 0;
               break;

           case FLEET_DIALOG_SHIP_PICTURE_BITMAP: break;
           
           case FLEET_DIALOG_SHIP_SUMMARY_TEXT: break;
           case FLEET_DIALOG_BACK_BUTTON: break;
           case FLEET_DIALOG_HELP_TEXT:/**/
           default:
               ;
        }
		/*if (fleetRet == FLEET_DIALOG_INFO) {
			ship_view_dialog(fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1, reference_fleet);
			showTitle();
		}*/

	} while((fleetRet != FLEET_DIALOG_BACK_BUTTON) && (fleetRet != -1));

//	reference_fleet = old_reference_fleet;

	remove_ship_image();

	fleet->save("fleets.ini", tmp);
	delete fleet;
	showTitle();
}

int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c) {

	static int next_anim_time = get_time();
	int old_d1 = d->d1;
    int ret = 0;
   

    // allow user to select the ships based on keystrokes:
    // select based on the ship's name
    bool shouldConsumeChar = false;
    if (msg == MSG_CHAR) {
        char typed = (char)(0xff & c);
//        if (isalnum (typed)) {
            d->d1 = reference_fleet->getNextFleetEntryByCharacter( d->d1, typed);
            shouldConsumeChar = true;
            if (d->d1 != old_d1) {

                int size = reference_fleet->getSize();
                int height = (d->h-4) / text_height(font);

                ret = D_USED_CHAR; 
                d->flags |= D_DIRTY;

                //scroll such that the selection is shown.
                //only change the scroll if the selection is not already shown,
                //and the number of ships in the list is greater than the number
                //of slots that can be shown simultaneously.
                if ( (size > height) &&
                     ( (d->d1 < d->d2) ||
                       (d->d1 >= d->d2 + height))) 
                {
                    if (d->d1 <= (height/2))
                        d->d2 = 0;
                    else {
                        
                        if (d->d1 >= (size - height))
                            d->d2 = (size - height);
                        else {
                            d->d2 = d->d1 - height/2;
                        }
                    }
                }
            }
//        }
    }
    ret = d_text_list_proc( msg, d, c );
    
    if (shouldConsumeChar)
        ret = D_USED_CHAR;

	// this is initialized once
    static BITMAP* panel = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
                                         fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);
    fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp = panel;

	// this is initialized once
    static BITMAP * sprite = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
                                           fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);

	if (!sprite || !panel)
		tw_error("bitmap error");

	//selection has changed
	// (or nothing is shown, yet)

    if (d->d1 != old_d1 || !showing_shipimage_type)
	{
		remove_ship_image();
        
		add_ship_image(d->d1);
    }

	if ( ( d->d1 != old_d1 || msg == MSG_START) || 
         (msg == MSG_IDLE && next_anim_time < get_time()) ) {
        safeToDrawPreview = false;

        //next_anim_time = get_time() + 50 + rand() % 200;
        next_anim_time = get_time() + 20;
        
		ShipType* type = reference_fleet->getShipType(d->d1);
        
        clear_to_color(sprite, 0);

        if (type && type->data && type->data->spriteShip) {
            
            rotationFrame++;
            if (rotationFrame >= type->data->spriteShip->frames())
                rotationFrame = 0;

			if (rotationFrame < 0)
				rotationFrame = 0;
				

            type->data->spriteShip->draw(
                Vector2(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w/2,
                        fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h/2) - type->data->spriteShip->size()/2, 
                type->data->spriteShip->size(), 
                rotationFrame, sprite 
                );

        }

        stretch_blit(sprite, panel, 0, 0, sprite->w, sprite->h, 0, 0, panel->w, panel->h);
        safeToDrawPreview = true;
        
        


		//if(data) {


		//static DATAFILE* data = NULL;
		//if (type && type->data) 
		//	data = load_datafile_object(type->data->file, "SHIP_P00_PCX");

/*
			BITMAP* bmp = (BITMAP*)data->dat;

			BITMAP* tmp = create_bitmap(bmp->w, bmp->h);
            //BITMAP* tmp = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
              //                          fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);

			//blit(bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h);

            stretch_blit(bmp, tmp, 
                         0, 0, bmp->w, bmp->h,
                         0, 0, fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
			 fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);*//*
			//unload_datafile_object(data);

			char obj[32];
			sprintf(obj, "SHIP_P0%d_PCX", 1 + rand() % 4);
			data = load_datafile_object(type->data->file, obj);
			bmp = (BITMAP*)data->dat;
			blit(bmp, tmp, 0, 0, 4, 65, bmp->w, bmp->h);
            //blit(bmp, tmp, 0, 0, 4, 65, bmp->w, bmp->h);

			unload_datafile_object(data);*/
//			gamma_correct_bitmap( tmp, gamma_correction, 0 );

			//panel = create_bitmap(128, 200);
            

            
			//destroy_bitmap(sprite);

			//stretch_blit(tmp, panel, 0, 0, tmp->w, tmp->h, 0, 0, panel->w, panel->h);
			//destroy_bitmap(tmp);
		//}

		//if(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp)
		//	destroy_bitmap( (BITMAP*)fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp );

		//fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp = panel;


        //TODO decide if these next 3 lines should be here
		scare_mouse();
        SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP], MSG_DRAW, 0);
		unscare_mouse();
	}

	return ret;
}

int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c) {
	//TODO address this: bitmap has to be deleted, but MSG_END does not mean the dialog isn't coming back
    /*if (msg == MSG_END && d->dp) {
		destroy_bitmap( (BITMAP*)d->dp );
		d->dp = NULL;
	}*/

	if ((msg != MSG_DRAW || d->dp) && (safeToDrawPreview) )
		return d_bitmap_proc(msg, d, c);
	return D_O_K;
}




