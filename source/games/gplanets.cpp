/*hacked version of the solar system melee by UAF and Corona688. This one is
done by rump and geoman.*/

/*

TO DO

networking
	synchronizing the menu

*/


#include <allegro.h>
#include <stdio.h>
#include <string.h>



#include "../melee.h"
REGISTER_FILE
#include "../melee/mgame.h"
#include "../melee/mcbodies.h"
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../melee/mmain.h"
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mitems.h"
#include "../melee/mfleet.h"   //fleets...

#include "../other/orbit.h"
#include "../id.h"
//#define STATION_LOG



// /*
class OrbitHandler_PLSY: public SpaceLocation
{
public:
	SpaceLocation *center, *orbiter;
	double a, eccentricity, b, dafocus, angle_linear, angvel, angle_orbit, angle_orientation;

	OrbitHandler_PLSY(SpaceLocation *creator,
		SpaceLocation *p_center, SpaceLocation *p_orbiter,
		double lwidth, double eccentricity, double lspeed,
		double langle_orientation);

	virtual void calculate();
	virtual int canCollide(SpaceLocation *other);
	void update_xyv();
};


inline double sqr(double x)
{
	return x*x;
}

void OrbitHandler_PLSY::update_xyv()
{

	// this is a linear increase in the angle
	angle_linear += angvel * frame_time;

	if ( angle_linear > PI2 )
		angle_linear -= PI2;

	if ( angle_linear < 0.0 )
		angle_linear += PI2;
	//angle_linear += 0.1 * frame_time;
	

	// NOTE: the following construction of an elliptical orbit has
	// nothing to do with a true simulation. It is simply a bunch of
	// arbitrary equations which give a reasonably looking trajectory.
	
	// result in a non-linear change of the angle of the ellips path:
	double a_linear = angle_linear;
	double a_slow   = (0.5 - 0.5 * cos(0.5 * angle_linear)) * 2*PI;

	// eccentricity = 0 results in a circl; ecc = 1 results in an inf. long and flat ellips
	angle_orbit = eccentricity * a_slow + (1-eccentricity) * a_linear;
	

	// for a normal ellipse:
	double dx = dafocus + a * cos(angle_orbit);
	double dy = b * sin(angle_orbit);

	// now, rotate the elliptical path around its focus:
	
	Vector2 drotated = dx*unit_vector(angle_orientation) - dy*unit_vector(-PI/2+angle_orientation);

	Vector2 pnew = center->pos + drotated;

	orbiter->vel = (pnew - orbiter->pos) / a;

	orbiter->pos = pnew;
}


OrbitHandler_PLSY::OrbitHandler_PLSY(SpaceLocation *creator,
	SpaceLocation *p_center, SpaceLocation *p_orbiter,
	double lwidth, double leccentricity,
	double lspeed, double langle_orientation)
	: SpaceLocation(creator, 0, 0.)
{
	id=ORBIT_ID;
	center=p_center;
	orbiter=p_orbiter;


	// a = halfwidth, b = halfheight, of the ellips.
	a = lwidth;
	eccentricity = leccentricity;
	b = a * sqrt(1 - sqr(eccentricity));

	// the (maximum) angular velocity, scale by orbit !!
	angvel = 0.075 * ANGLE_RATIO / lspeed;	// lspeed is really the orbit period in days

	// the focus, with respect to the ellips center:
	dafocus = a * sqrt(1 - sqr(b/a));

	// orientation of the ellips:
	angle_orientation = langle_orientation;

	// helper
	angle_linear = random(PI2);

	update_xyv();
	//orbiter->x = center->x + xoffset + a * cos(angle);
	//orbiter->y = center->y + b * sin(angle);

	vel = 0;

}

void OrbitHandler_PLSY::calculate()
{
	if((center==NULL)||(orbiter==NULL))
		return;

	//If they don't both exist, then destroy the OrbitHandler
	if((!center->exists())||(!orbiter->exists()))
	{
		center=NULL;
		orbiter=NULL;
		state=0;
		return;
	}

	update_xyv();

	SpaceLocation::calculate();

	// lock the planet position, otherwise it'll be attracted by
	// its moons !
	center->vel = 0;

}

int OrbitHandler_PLSY::canCollide(SpaceLocation *other)
{
	return FALSE;
}





class BigBackgr : public Presence
{
public:
	SpaceSprite		*pic;
	int		x, y;
	double	applyscale;

	BigBackgr(SpaceSprite *lpic, int lx, int ly, double lapplyscale);
	void animate(Frame *space);
};

BigBackgr::BigBackgr(SpaceSprite *lpic, int lx, int ly, double lapplyscale)
{
	x = lx;
	y = ly;
	pic = lpic;
	applyscale = lapplyscale;

//	set_depth(DEPTH_STARS);
	set_depth( 0.5*(DEPTH_STARS + DEPTH_HOTSPOTS) );
}

void BigBackgr::animate(Frame *frame)
{
	int aa_old = get_tw_aa_mode();
	set_tw_aa_mode(0);

	pic->animate(Vector2(x,y), 0, frame, applyscale);

	/*
	int wx, wy;
	wx = (int) (0.5 + pic->width() * space_zoom * applyscale);
	wy = (int) (0.5 + pic->height() * space_zoom * applyscale);

	BITMAP *tmp = create_bitmap(wx, wy);

	clear_to_color(tmp, bitmap_mask_color(tmp));
	pic->draw(0, Vector2(wx, wy), 0, tmp);

	// screen center
	int tx, ty;
	tx = (frame->surface->w - wx)/2;
	ty = (frame->surface->h - wy)/2;

	// coordinate relative to the coordinate of the screen center (space_x) in no-zoom
	int sx, sy;
	sx = x - space_center.x;
	sy = y - space_center.y;

	// coordinate scaled by zoom
	sx = sx * space_zoom + tx;
	sy = sy * space_zoom + ty;

	draw_sprite(frame->surface, tmp, (int)sx, (int)sy);
	frame->add_box(sx, sy, wx, wy);
	
	destroy_bitmap(tmp);
	*/

	set_tw_aa_mode(aa_old);
}



// should be easy enough to make !

class RadarMap : public Presence
{
public:
	BITMAP	*radarscreen;
	int		wx, wy, MSize;

	RadarMap(int MapSize);
	void animate(Frame *frame);
};

RadarMap::RadarMap(int MapSize)
{
	wx = 75;
	wy = 75;
	MSize = MapSize;
	radarscreen = create_bitmap(wx, wy);

	set_depth(DEPTH_STARS);
}

void RadarMap::animate(Frame *frame)
{


	int aa_old = get_tw_aa_mode();
	set_tw_aa_mode(0);

	clear_to_color(radarscreen, makecol(100,100,100));

	double scandist = 2500.0;

	double x0 = space_center.x;
	double y0 = space_center.y;

	Query a;
	SpaceObject *o;
	for (a.begin(NULL, Vector2(x0, y0), bit(LAYER_SHIPS) + bit(LAYER_CBODIES) + bit(LAYER_SHOTS),
		scandist); a.current; a.next())
	{
		o = a.currento;

		// scale the location onto the map:
		double scale = wx / (2.0 * scandist);
		int xpos = wx/2 + (o->normal_pos().x - x0) * scale;
		int ypos = wy/2 + (o->normal_pos().y - y0) * scale;

		// what color to use?
		// makecol(r, g, b)

		int col, i = 255;
		col = 0;

		
		switch( o->id )
		{
		case ID_PLANET:
		case MOON_ID:
		case SUN_ID:
		case COMET_ID:
			col = makecol(i, i, i);
			break;
		case ID_ASTEROID:
			col = makecol(i/2, i/2, i/2);
			break;
		case SPACE_SHIP:
			col = makecol(i, 0, 0);
			break;
		case SPACE_SHOT:
			col = makecol(i, i/2, 0);
			break;
		}

		
		if( (o->layer & bit(LAYER_SHOTS)) != 0 )
			col = makecol(i/3, i/2, i);

		if (col != 0 && !o->isInvisible())
			for ( i = -1; i < 2; ++i )
				for ( int j = -1; j < 2; ++j )
					putpixel(radarscreen, xpos+1, ypos+j, col);
	}



	// add the radar screen to the draw list:
	int sx = 720;
	int sy = 400;
	draw_sprite(screen, radarscreen, sx, sy);
	//frame->add_box(sx, sy, wx, wy);

	set_tw_aa_mode(aa_old);
}




// */


class Planets : public NormalGame
{
	public:
	double	*MoonGrav, *PlanetGrav;
	int		*MoonType, *PlanetType;

	int Num_Planet_Pics;
	int Num_Moon_Pics;

	SpaceSprite **PlanetPics; //[Num_Planet_Pics];
    SpaceSprite **MoonPics; //[Num_Moon_Pics];
	SpaceSprite **backgrimages;

	~Planets();

	virtual void init_objects();
	virtual void ReadPlanetSystem(int iPlanetSystem,
						int &iplanetpic, int &NumMoons,
						double &PlanetPlGrav, double &PlanetPlGravRange,
						int *PlanetMoonPic,
						double *PlanetMoonEllipsW, double *PlanetMoonEllipsE,
						double *PlanetMoonEllipsAngvel,
						double *PlanetMoonEllipsOrientation,
						double *PlanetMoonGrav,
						int MinMoonsInSystem, int MaxMoonsInSystem,
						int OrbitRadiusMin, int OrbitRadiusMax,
						double OrbitPeriodMin, double OrbitPeriodMax,
						double MoonGravMin, double MoonGravMax,
						double MoonMaxEllipsE,
						int &NumStarbases,
						double *PlanetStarbaseEllipsW, double *PlanetStarbaseEllipsE,
						double *PlanetStarbaseEllipsAngvel,
						double *PlanetStarbaseEllipsOrientation,
						char *PlanetText);

	virtual void ChoosePlanetSystem(int iPlanetSystem, int NPlanetSystem,
						int &iplanetpic, int &NumMoons, int MapSize,
						double &PlanetPlGrav, double &PlanetPlGravRange,
						int *PlanetMoonPic,
						double *PlanetMoonEllipsW, double *PlanetMoonEllipsE,
						double *PlanetMoonEllipsAngvel,
						double *PlanetMoonEllipsOrientation,
						double *PlanetMoonGrav,
						int MinMoonsInSystem, int MaxMoonsInSystem,
						int OrbitRadiusMin, int OrbitRadiusMax,
						double OrbitPeriodMin, double OrbitPeriodMax,
						double MoonGravMin, double MoonGravMax,
						double MoonMaxEllipsE,
						int &NumStarbases,
						double *PlanetStarbaseEllipsW, double *PlanetStarbaseEllipsE,
						double *PlanetStarbaseEllipsAngvel,
						double *PlanetStarbaseEllipsOrientation);

	bool GetSprites(SpaceSprite *Pics[], DATAFILE *datafile, char *cmdStr, int numSprites);
	SpaceSprite *GetSprite(DATAFILE *datafile, char *spriteName);


};


int max(int i, int j)
{
	if ( i > j )
		return i;
	else
		return j;
}

// A MENU FOR CHOOSING A PLANET SYSTEM FROM A LIST !!

/* I COPIED THIS FROM THE ALLEGRO SOURCE DIRECTORY, AND ADAPTED IT SO
   THAT IT MAKES A GOOD PUSH BUTTON !!!
 * d_icon_proc:
 *  Allows graphic icons to be used as buttons.
 * 
 *  Parameters:
 *    fg = color dotted line showing focus will be drawn in
 *    bg = shadow color used to fill in top and left sides of
 *         button when "pressed"
 *    d1 = "push depth": number of pixels icon will be shifted
 *         to right and down when selected (default=2) if there is
 *         no "selected" image
 *    d2 = distance dotted line showing focus is indented (default=2)
 *    dp = pointer to a bitmap for the icon
 *    dp2 = pointer to a "selected" bitmap for the icon (OPTIONAL)
 *    dp3 = pointer to a "disabled" bitmap for the icon (OPTIONAL)
 */
int d_icon_proc_PLSY(int msg, DIALOG *d, int c)
{
	 BITMAP *butimage = (BITMAP *)d->dp;
	 int butx;
	 int buty;
	 


	 
	 switch (msg) {
		 
	 case MSG_WANTFOCUS:
		 return D_WANTFOCUS;
		 
	 case MSG_KEY:
		 /* close dialog? */
		 if (d->flags & D_EXIT) {
			 return D_CLOSE;
		 }
		 
		 /* or just toggle */
		 d->flags ^= D_SELECTED;
		 scare_mouse();
		 object_message(d, MSG_DRAW, 0);
		 unscare_mouse();
		 break;
	 }
	 

	 if ((msg == MSG_DRAW) && (!(d->flags & D_HIDDEN))) {

		 // by default, the image is dp (see at the start of the routine)

		 if ((d->dp2 != NULL) && (d->flags & D_GOTFOCUS) && !(d->flags & D_SELECTED)) {
			 butimage = (BITMAP *)d->dp2;
		 }
		 if ((d->dp3 != NULL) && (d->flags & D_SELECTED)) {
			 butimage = (BITMAP *)d->dp3;
		 }
		 

		 butx = butimage->w;
		 buty = butimage->h;
		 masked_stretch_blit(butimage, screen, 0, 0, butx, buty, 
			 d->x, d->y, d->w, d->h);

		 return D_O_K;
	 }


	 if ((msg == MSG_LPRESS) && (d->flags & D_GOTFOCUS))
	 {
		 d->flags |= D_SELECTED;
		 scare_mouse();
		 object_message(d, MSG_DRAW, 0);
		 unscare_mouse();
	 }

	 if ((msg == MSG_GOTFOCUS) && (gui_mouse_b() & 1))
	 {
		 d->flags |= D_SELECTED;
		 scare_mouse();
		 object_message(d, MSG_DRAW, 0);
		 unscare_mouse();
	 }

	 if ((msg == MSG_LRELEASE) || (msg == MSG_LOSTFOCUS))
	 {
		 d->flags &= (int)0x0FFFF - D_SELECTED;
		 scare_mouse();
		 object_message(d, MSG_DRAW, 0);
		 unscare_mouse();
	 }
	 


	 return D_O_K;
 }
 // END OF THE COPY




/*
int d_icon_proc(int msg, DIALOG *d, int c);
A bitmap button. The fg color is used for the dotted line showing focus, and the
bg color for the shadow used to fill in the top and left sides of the button when 
"pressed". d1 is the "push depth", ie. the number of pixels the icon will be shifted 
to the right and down when selected (default 2) if there is no "selected" image. 
d2 is the distance by which the dotted line showing focus is indented (default 2). 
dp points to a bitmap for the icon, while dp2 and dp3 are the selected and disabled 
images respectively (optional, may be NULL).
*/

// DIALOG (part 1)
static int displayvalue = 0;
static char *testtext = "start it!";


static int		B_SELECT, B_REFRESH, B_SIZE_INC, B_SIZE_DEC, B_SIZE_DEF,
				B_SYS_NEXT, B_SYS_PREV;
static DIALOG	*Dialog_star;
static BITMAP	*bmp_, *bitmap_button_right, *bitmap_button_up,
				*bitmap_button_down, *bitmap_button_select,
				*bitmap_button_refresh, *bitmap_selectscreen;
static DIALOG_PLAYER	*Dialog_star_player;
static int		xPutDialog = 0, yPutDialog = 0;
static char		PlanetSystemText[512];



BITMAP* get_data_bmp(DATAFILE *data, char *objname, double scale)
{
	DATAFILE *dataobj;
	BITMAP *r, *old;

	dataobj = find_datafile_object(data, objname);
	if (!dataobj)
		tw_error( "couldn't find data object [%s]!", objname);
	
	// copied from mshipdata.cpp:
	old = (BITMAP*)dataobj->dat;
	r = create_bitmap(old->w*scale, old->h*scale);
	stretch_blit (old, r, 0, 0, old->w, old->h, 0, 0, r->w, r->h);
	
	return r;
}

void add_dialog_icon_proc( DIALOG *Dialog_star, int *k, DATAFILE *tmpdata,
						      char *name, int x, int y, double scale )
{
	BITMAP *bmps[3];

	for ( int i = 0; i < 3; ++i )
	{
		char tmpname[512];
		sprintf(tmpname, "%s%02i", name, i+1);
//		BITMAP *tmpbmp;
		bmps[i] = get_data_bmp(tmpdata, tmpname, scale);	// already does an error check
	}
	// 3 bitmaps, neutral, mouse-over, and on-click ... or something ;)

	
	int j = 0;

	DIALOG tmpdialog =
		{ d_icon_proc_PLSY,      x*scale,  y*scale, bmps[0]->w, bmps[0]->h,   255,  0,    0,    j,  6,    6,    bmps[0], bmps[1], bmps[2] };

	Dialog_star[*k] = tmpdialog;
	++ (*k);	// the next dialog.
}

void init_dialog_PLSY (double dialog_scale)
{
	
	// read the graphics:

	set_color_conversion(COLORCONV_TOTAL);


	Dialog_star = new DIALOG [10];

	int k = 0;

	// sloppy use of memory here... but I suppose the program will take
	// care of the cleanup at the end ;)

	DATAFILE *tmpdata = load_datafile("plbuttons.dat");


	bitmap_selectscreen   = get_data_bmp(tmpdata, "scr_main", dialog_scale);


	B_SELECT = k;	// index of the k-th dialog item
	add_dialog_icon_proc( Dialog_star, &k, tmpdata, "b_start", 686, 485, dialog_scale ); // 700 450
	
	B_REFRESH = k;
	add_dialog_icon_proc( Dialog_star, &k, tmpdata, "b_redraw", 67, 456, dialog_scale ); // 40 450
	


	B_SIZE_INC = k;		// scale the system size up
	add_dialog_icon_proc( Dialog_star, &k, tmpdata, "b_size_inc", 729,  34, dialog_scale ); // 700 50

	B_SIZE_DEF = k;		// reset (default) scale
	add_dialog_icon_proc( Dialog_star, &k, tmpdata, "b_size_def", 729,  76, dialog_scale ); // 700 100

	B_SIZE_DEC = k;		// scale the system size down
	add_dialog_icon_proc( Dialog_star, &k, tmpdata, "b_size_dec", 729, 103, dialog_scale ); // 700 150



	B_SYS_NEXT = k;		// choose next/ previous planet system
	add_dialog_icon_proc( Dialog_star, &k, tmpdata, "b_system_next",  68,  22, dialog_scale ); // 50 50 

	B_SYS_PREV = k;
	add_dialog_icon_proc( Dialog_star, &k, tmpdata, "b_system_prev",  68, 111, dialog_scale ); // 50 100


	unload_datafile_object(tmpdata);




		// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)  (dp2)  (dp3)
			
	DIALOG dialog_timeslice =
		{ d_tw_yield_proc,        0,   0,    0,     0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL };
		
	DIALOG dialog_empty =
		{ NULL,              0,   0,    0,     0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL };


	Dialog_star[k] = dialog_timeslice;
	++k;

	Dialog_star[k] = dialog_empty;
	++k;

	//do_dialog(Dialog_star, -1);

	Dialog_star_player = init_dialog(Dialog_star, -1);

	
}



double drand()
{
	return (random() % 1000) / 1000.0;
}



void Planets::ReadPlanetSystem(int iPlanetSystem,
						int &iplanetpic, int &NumMoons,
						double &PlanetPlGrav, double &PlanetPlGravRange,
						int *PlanetMoonPic,
						double *PlanetMoonEllipsW, double *PlanetMoonEllipsE,
						double *PlanetMoonEllipsAngvel,
						double *PlanetMoonEllipsOrientation,
						double *PlanetMoonGrav,
						int MinMoonsInSystem, int MaxMoonsInSystem,
						int OrbitRadiusMin, int OrbitRadiusMax,
						double OrbitPeriodMin, double OrbitPeriodMax,
						double MoonGravMin, double MoonGravMax,
						double MoonMaxEllipsE,
						int &NumStarbases,
						double *PlanetStarbaseEllipsW, double *PlanetStarbaseEllipsE,
						double *PlanetStarbaseEllipsAngvel,
						double *PlanetStarbaseEllipsOrientation,
						char *PlanetSystemText)
{
	int	i, j;
	
	char	objectname[512];
	sprintf(objectname, "Planets%03i\0", iPlanetSystem);

	strcpy(PlanetSystemText, get_config_string(objectname,"PlanetText",0));
	//strcpy(PlanetSystemText, "HELLO THERE !");

	//iplanetpic = get_config_int(objectname,"PlanetPic00",0);


	char name[512];
	const char	*result;
	i = 0;
	sprintf(name, "PlanetPic%02i\0", i);
	result = get_config_string(objectname, name, 0);
	if ( result )
	{
		sscanf(result, "%i %lf %lf", &iplanetpic,
			&PlanetPlGrav, &PlanetPlGravRange);

	} else {
		error("Could not initialize the planet !");
	}

	if ( iplanetpic == -1 )
	{
		iplanetpic = random() % Num_Planet_Pics;
	}

	NumMoons = get_config_int(objectname,"PlanetNmoons",0);

	if (NumMoons == -1)
	{
		if ( MinMoonsInSystem != MaxMoonsInSystem)
			NumMoons = MinMoonsInSystem + random() % (MaxMoonsInSystem-MinMoonsInSystem);
		else
			NumMoons = MinMoonsInSystem;
		//iMessage("NumMoons = %d *RANDOM*", NumMoons);
		//iMessage("MinMoons = %d *RANDOM*", MinMoonsInSystem);
		//iMessage("MaxMoons = %d *RANDOM*", MaxMoonsInSystem);
	}
	//else
		//iMessage("NumMoons = %d *PRESET*", NumMoons);

	

	for ( i = 0; i < NumMoons; ++i )
	{
		char name[512];
		const char	*result;
		sprintf(name, "PlanetMoon%02i\0", i);
		result = get_config_string(objectname, name, 0);
		if ( result )
		{
			sscanf(result, "%i %lf %lf %lf %lf %lf", &PlanetMoonPic[i],
				&PlanetMoonEllipsW[i],
				&PlanetMoonEllipsE[i], &PlanetMoonEllipsAngvel[i],
				&PlanetMoonEllipsOrientation[i], &PlanetMoonGrav[i]);

		}

	}


	// The starbases:

	NumStarbases = get_config_int(objectname,"PlanetNstarbases",0);

	for ( i = 0; i < NumStarbases; ++i )
	{
		char name[512];
		const char	*result;
		sprintf(name, "PlanetStarbase%02i\0", i);
		result = get_config_string(objectname, name, 0);
		if ( result )
		{
			sscanf(result, "%lf %lf %lf %lf",
				&PlanetStarbaseEllipsW[i],
				&PlanetStarbaseEllipsE[i], &PlanetStarbaseEllipsAngvel[i],
				&PlanetStarbaseEllipsOrientation[i]);

		}

	}


	// determine the pictures ... such that you don't have duplicate moon images:

	int ichosen[1000];
	for ( i = 0; i < Num_Moon_Pics; ++i )
	{
		ichosen[i] = 0;
	}

	for ( i = 0; i < NumMoons; ++i )
	{
		int k;

		if (PlanetMoonPic[i] != -1)
			continue;

		// choose a random moon picture
		k = random() % (Num_Moon_Pics - i);

		// choose the k-th available (free) picture:
		for ( j = 0; j < Num_Moon_Pics; ++j )
		{
			if ( ichosen[j] == 0 )	// this one is available
				--k;
			if ( k < 0 || j >= Num_Moon_Pics )
				break;
		}

		if ( j < Num_Moon_Pics )
		{
			PlanetMoonPic[i] = j;	// this is the k-th available one
			ichosen[j] = 1;
		} else {	// oh my! there are not enough unique moons. Just choose any of them then
			 PlanetMoonPic[i] = random() % Num_Moon_Pics;
		}

		if ( PlanetMoonPic[i] < 0 || PlanetMoonPic[i] > Num_Moon_Pics-1 )
			error("illegal moon pic");
		
	}

	// determine the positions of the moons, in case they have to be random numbers

	int	z[100];
	int	Rmin, Rmax;

	for ( i = 0; i < NumMoons; ++i )
	{
		if (PlanetMoonEllipsW[i] != -1 )
			continue;

		// check a range where the moons are randomly distributed:
		int k = i;
		while ( k < NumMoons && PlanetMoonEllipsW[k] == -1 )
			++k;

		// now, i is the first unknown one, and k-1 the last unknown one.

		if ( i == 0 )
			Rmin = OrbitRadiusMin;
		else
			Rmin = PlanetMoonEllipsW[i-1];

		if ( k == NumMoons )
			Rmax = OrbitRadiusMax;
		else
			Rmax = PlanetMoonEllipsW[k];

		// now, assign a radius to these moons (with indexes i to k-1 )

		// random positions, arbitrary distances (1000 in this case)
		int	j;
		for ( j = i; j <= k; ++j )	// note, j <= k, we take one more range than the number of moons
		{
			z[j] = random() % 1000;
		}

		for ( j = i+1; j <= k; ++j )
		{
			z[j] += z[j-1];
		}

		// scale values, and place them
		for ( j = i; j < k; ++j )
		{
			// Rmax-Rmin = new range, z[k] = the arbitrary range
			PlanetMoonEllipsW[j] = Rmin + (z[j] * (Rmax-Rmin) / z[k] );
			
		}

	}
	
	
	// make sure the moons are not too close to each other
	for ( i = 0; i < NumMoons-1; ++i )
	{
		int R1 = max(MoonPics[PlanetMoonPic[i]]->height(), 
			MoonPics[PlanetMoonPic[i]]->width());

		int R2 = max(MoonPics[PlanetMoonPic[i+1]]->height(),
			MoonPics[PlanetMoonPic[i+1]]->width());
		
		if (PlanetMoonEllipsW[i+1]-PlanetMoonEllipsW[i] < R1+R2)
			PlanetMoonEllipsW[i+1] = PlanetMoonEllipsW[i]+R1+R2;

		//iMessage("RADIUS = %i", PlanetMoonRadius[i]);
		
	}


	// also, check other random parameters

	double PeriodScaling = OrbitPeriodMin + drand() * (OrbitPeriodMax -
		               OrbitPeriodMin);

	

	for ( i = 0; i < NumMoons; ++i )
	{
		// the orbit period, should increase with radius
		if (PlanetMoonEllipsAngvel[i] == 0)
			PlanetMoonEllipsAngvel[i] = PeriodScaling * PlanetMoonEllipsW[i] / 100.0;

		// the gravity, should be randomized as well?
		if (PlanetMoonGrav[i] < 0)
			PlanetMoonGrav[i] = MoonGravMin + drand() * (MoonGravMax - MoonGravMin);

		if ( PlanetMoonEllipsE[i] < 0)
			PlanetMoonEllipsE[i] = MoonMaxEllipsE * drand();

		//iMessage("W = %d *RANDOM*", int(100 * PlanetMoonEllipsW[i]));
		//iMessage("E = %d *RANDOM*", int(100 * PlanetMoonEllipsE[i]));
		//iMessage("P = %d *RANDOM*", int(100 * PlanetMoonEllipsAngvel[i]));
		//iMessage("G = %d *RANDOM*", int(100 * PlanetMoonGrav[i]));
	}
	

}


static int mouse_b_integrated;
static int mouse_b_integrated_return;

inline void mouse_callback_function(int flags) // this is called each time mous_b is updated
{
	mouse_b_integrated |= mouse_b;	// this is asynchronously updated.
}

void (*mouse_callback_old)(int flags) = NULL;

void mouse_callback_reset()	// a synchronizable (cause external) way of refreshing information.
{
	mouse_b_integrated_return = mouse_b_integrated;	// small lag is introduced,
		// but this value is stable in-between calls.
		// the more often this routine is called, the smaller the lag.

	mouse_b_integrated = mouse_b;	// refresh (reset) the new thingy
		// this value is asynchronously updated by the mouse, capturing all mouse
		// button events up to the last call, returning that there was at least one.
}

int (*gui_mouse_b_old)();

int gui_mouse_b_new()	// called by the dialog to get the button status
{
	return mouse_b_integrated_return;
	// normally returns the current mouse_b value, but that value may change
	// too rapidly in-between uses of this function.
}


void Planets::ChoosePlanetSystem(int iPlanetSystem, int NPlanetSystem,
					int &iplanetpic, int &NumMoons, int MapSize,
					double &PlanetPlGrav, double &PlanetPlGravRange,
					int *PlanetMoonPic,
					double *PlanetMoonEllipsW, double *PlanetMoonEllipsE,
					double *PlanetMoonEllipsAngvel,
					double *PlanetMoonEllipsOrientation,
					double *PlanetMoonGrav,
					int MinMoonsInSystem, int MaxMoonsInSystem,
					int OrbitRadiusMin, int OrbitRadiusMax,
					double OrbitPeriodMin, double OrbitPeriodMax,
					double MoonGravMin, double MoonGravMax,
					double MoonMaxEllipsE,
					int &NumStarbases,
					double *PlanetStarbaseEllipsW, double *PlanetStarbaseEllipsE,
					double *PlanetStarbaseEllipsAngvel,
					double *PlanetStarbaseEllipsOrientation)
{
	int iupdate = 1;
	int iredraw = 0;
	int i;

	double orbitangle[100];	// for the moons that rotate... should look nice, I hope.

	for ( i = 0; i < 100; ++i )
	{
		orbitangle[i] = random(PI2);
	}

	// use the mouse_callback of allegro to keep a track whether a button
	// was pressed in the mean time, or not (this mouse_b_integrated gives
	// less resolution than an immediate updated value like mouse_b).

	mouse_callback_old = mouse_callback;	// remember the old call_back (=null)
	mouse_callback = mouse_callback_function;

	gui_mouse_b_old = gui_mouse_b;
	gui_mouse_b = gui_mouse_b_new;

	iMessage("selection screen !%i", 1);

	// note, I'll use mbnew and mbold, because mouse_b may change in-between
	// references to it, whereas I need to know the (virtual) status at ALL
	// times !!

	int mbold, mbnew;
	mbnew = gui_mouse_b();

	if ( mbnew )
		mbold = !mbnew;
	else
		mbold = mbnew;

	mbold = mbnew;
	
	// The dialog was written for an 800x600 screen. The positions also ... thus,
	// scale this for the current screen size:
	double dialog_scale = screen->w / 800.0;

	init_dialog_PLSY (dialog_scale);	// the scale is needed to obtain correct screen positions

	double t_new;
	double t_old = get_time2();
	double wait_time = 250.0;


	BITMAP *tmpscreen = create_bitmap(bitmap_selectscreen->w, bitmap_selectscreen->h);
//	BITMAP *tmpscreen = create_bitmap(screen->w, screen->h);	// use the whole screen
	// this bitmap was initialized and scaled in init_dialog !!

	show_mouse(tmpscreen);	// redirect all mouse output to a temp screen.

	int xcenter = tmpscreen->w / 2 + 20*dialog_scale;
	int ycenter = tmpscreen->h / 2 -  5*dialog_scale;

	double scalefactor = 1.0;	// scaling applied to the planet system

	double t_angle_waittime, t_old_angle, t_new_angle;
	t_angle_waittime = 0.0;
	t_new_angle = get_time2();
	t_old_angle = t_new_angle;
	double t_angle_maxwaittime = 50.0;	// in milliseconds
	

	for (;;)
	{

		// NOTE: the allegro mouse routine provides asynchronous updates to
		// the mouse. Since we use the button information on different places
		// in this loop, we need some kind of synchronized button information.
		// For this, we use a hooked gui_mouse_b and a hooked function to
		// the interrupt mouse routine !!

		mouse_callback_reset();
		// this makes the average mouse button of the last loop available, and
		// resets a variable which will contain the average mouse button of
		// the new loop.


		//update_dialog ( Dialog_star_player );

		// allegro routine
		update_dialog ( Dialog_star_player );

		if ( iupdate || iredraw )
		{

			if ( iupdate )
				ReadPlanetSystem(iPlanetSystem, iplanetpic, NumMoons,
						PlanetPlGrav, PlanetPlGravRange,
						PlanetMoonPic,
						PlanetMoonEllipsW, PlanetMoonEllipsE,
						PlanetMoonEllipsAngvel, PlanetMoonEllipsOrientation,
						PlanetMoonGrav,
						MinMoonsInSystem, MaxMoonsInSystem,
						OrbitRadiusMin, OrbitRadiusMax,
						OrbitPeriodMin, OrbitPeriodMax,
						MoonGravMin, MoonGravMax,
						MoonMaxEllipsE,
						NumStarbases,
						PlanetStarbaseEllipsW, PlanetStarbaseEllipsE,
						PlanetStarbaseEllipsAngvel,
						PlanetStarbaseEllipsOrientation,
						PlanetSystemText);

			// show the planet and moon pictures, so that you know what
			// you choose:

			int i;

			int index = 0;
			int x, y;

			//clear_to_color(tmpscreen, 0x0FFFF00);
			//blit (bitmap_selectscreen, tmpscreen, 0, 0, 0, 0, tmpscreen->w, tmpscreen->h);
			stretch_blit (bitmap_selectscreen, tmpscreen, 0, 0, bitmap_selectscreen->w, bitmap_selectscreen->h,
				0, 0, tmpscreen->w, tmpscreen->h);

			x = xcenter - PlanetPics[iplanetpic]->width()/2;
			y = ycenter - PlanetPics[iplanetpic]->height()/2;

			Vector2 P, S;

			P = Vector2(x,y);
			S = PlanetPics[iplanetpic]->size() * dialog_scale;

			//PlanetPics[iplanetpic]->draw(x, y, index, tmpscreen);
			PlanetPics[iplanetpic]->draw(P, S, index, tmpscreen);
			//x += PlanetPics[iplanetpic]->width() /2;

			scare_mouse();
			// also, show some text information
			gui_textout(tmpscreen, PlanetSystemText, 92*dialog_scale, 82*dialog_scale, 0x000FF00, TRUE);

			// also, force the dialog(s) to redraw itself, otherwise this big image
			// would overwrite all the buttons:
			BITMAP *truescreen = screen;
			screen = tmpscreen;		// very dirty, but necessary to re-direct dialog output
			i = 0;
			while (Dialog_star[i].proc)
			{
				object_message(&Dialog_star[i], MSG_DRAW, 0);
				++i;
			}
			screen = truescreen;

			// also, show the moons .. orbiting, for niceness.
			for ( i = 0; i < NumMoons; ++i )
			{
				double R = PlanetMoonEllipsW[i] * 0.9*tmpscreen->h*dialog_scale * scalefactor / MapSize;
				x = xcenter + int(R * cos(orbitangle[i])) - MoonPics[PlanetMoonPic[i]]->width()/2;
				y = ycenter + int(R * sin(orbitangle[i])) - MoonPics[PlanetMoonPic[i]]->height()/2;

				Vector2 P, S;

				P = Vector2(x,y);
				S = MoonPics[PlanetMoonPic[i]]->size() * dialog_scale;


//void SpaceSprite::draw(Vector2 pos, Vector2 size, int index, BITMAP *surface)
				//MoonPics[PlanetMoonPic[i]]->draw(x, y, index, tmpscreen);
				MoonPics[PlanetMoonPic[i]]->draw(P, S, index, tmpscreen);
				//x += MoonPics[PlanetMoonPic[i]]->width() /2;
			}

			// also, show the starbases .. orbiting, for niceness, but
			// less nice, only as circles ...
			for ( i = 0; i < NumStarbases; ++i )
			{
				double R = PlanetStarbaseEllipsW[i] * 0.9*tmpscreen->h*dialog_scale * scalefactor / MapSize;
				x = xcenter + int(R * cos(orbitangle[i]));
				y = ycenter + int(R * sin(orbitangle[i]));

				circle(tmpscreen, (int)x, (int)y, 15, makecol(180,0,0));
				//MoonPics[PlanetMoonPic[i]]->draw(x, y, index, tmpscreen);
				//x += MoonPics[PlanetMoonPic[i]]->width() /2;
			}

			// also, force the mouse to show itself.
			show_mouse(tmpscreen);
			
			blit (tmpscreen, screen, 0, 0, xPutDialog, yPutDialog, tmpscreen->w, tmpscreen->h);

			unscare_mouse();

			iupdate = 0;
			iredraw = 0;

		}

		//if ( gui_mouse_b() )
		//{
		//	iMessage("x = %i", gui_mouse_x());
		//	iMessage("y = %i", gui_mouse_y());
		//}

		t_new = get_time2();


		mbnew = gui_mouse_b();

		if (!(mbnew & 1))	// left button is not pressed
			t_old = t_new;

		if ( !(mbold & 1) && (mbnew & 1) )
			t_old = t_new - wait_time - 0.01;	// first press is always "ok"

		mbold = mbnew;
		
		
		if ( t_new - t_old >= wait_time )
		{
			t_old += wait_time;

			// check user input.
			if ( Dialog_star[B_SYS_PREV].flags & D_SELECTED )
			{
				iPlanetSystem --;
				if ( iPlanetSystem < 0 )
					iPlanetSystem = NPlanetSystem-1;
				iupdate = 1;
			}
			
			if ( Dialog_star[B_SYS_NEXT].flags & D_SELECTED )
			{
				iPlanetSystem ++;
				if ( iPlanetSystem > NPlanetSystem-1 )
					iPlanetSystem = 0;
				iupdate = 1;
			}
			
			if ( Dialog_star[B_SIZE_DEC].flags & D_SELECTED )
			{
				scalefactor -= 0.1;
				if ( scalefactor < 0.1 )
					scalefactor = 0.1;
				iredraw = 1;
			}

			if ( Dialog_star[B_SIZE_INC].flags & D_SELECTED )
			{
				scalefactor += 0.1;
				iredraw = 1;
			}

			if ( Dialog_star[B_SIZE_DEF].flags & D_SELECTED )
			{
				scalefactor = 1.0;
				iredraw = 1;
			}

			if ( Dialog_star[B_REFRESH].flags & D_SELECTED )
				iupdate = 1;
			
			if ( Dialog_star[B_SELECT].flags & D_SELECTED )
				break;
		}


		t_old_angle = t_new_angle;
		t_new_angle = get_time2();
		double dtime = t_new_angle - t_old_angle;
		t_angle_waittime += dtime;

		for ( int i = 0; i < NumMoons; ++i )
		{
			orbitangle[i] += dtime*0.002 / PlanetMoonEllipsAngvel[i];
		}

		if ( t_angle_waittime > t_angle_maxwaittime )
		{
			iredraw = 1;
			t_angle_waittime -= t_angle_maxwaittime;
		}

	}

	shutdown_dialog( Dialog_star_player );

	// restore the original function routines.
	mouse_callback = mouse_callback_old;
	gui_mouse_b = gui_mouse_b_old;

	show_mouse(NULL);


	// apply the scale factor, then exit:
	for ( i = 0; i < NumMoons; ++i )
	{
		PlanetMoonEllipsW[i] *= scalefactor;
		PlanetMoonEllipsAngvel[i] *= scalefactor;	// increase orbit period
	}

	for ( i = 0; i < NumStarbases; ++i )
	{
		PlanetStarbaseEllipsW[i] *= scalefactor;
		PlanetStarbaseEllipsAngvel[i] *= scalefactor;	// increase orbit period
	}

	// clear the background
	clear_to_color(screen, makecol(0,0,0));
	
}





SpaceSprite *Planets::GetSprite(DATAFILE *datafile, char *spriteName)
{

	DATAFILE *tmpdata;
	tmpdata = find_datafile_object(datafile, spriteName);

	if(tmpdata==NULL)
	{
		#ifdef STATION_LOG
			sprintf(msgStr,"Unable to load %s#%s",fileName,spriteName);
			message.out(msgStr);
		#endif

		return NULL;
	}

	#ifdef STATION_LOG
		sprintf(msgStr, "Succesfully loaded %s#%s!  Hot damn!",fileName,spriteName);
		message.out(msgStr);
	#endif

	SpaceSprite *spr=new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 1);

	return spr;
}

bool Planets::GetSprites(SpaceSprite *Pics[], DATAFILE *datafile, char 
*cmdStr, int numSprites)
{

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++)
	{
		sprintf(dataStr,cmdStr,num);
		spr=GetSprite(datafile, dataStr);
		if(!spr)
		{
			return FALSE;
		}

		Pics[num]=spr;
	}

	return TRUE;
}



SpaceSprite *general_GetSprite(DATAFILE *datafile, char *spriteName)
{

	DATAFILE *tmpdata;
	tmpdata = find_datafile_object(datafile, spriteName);

	if(tmpdata==NULL)
	{
		#ifdef STATION_LOG
			sprintf(msgStr,"Unable to load %s#%s",fileName,spriteName);
			message.out(msgStr);
		#endif

		return NULL;
	}

	#ifdef STATION_LOG
		sprintf(msgStr, "Succesfully loaded %s#%s!  Hot damn!",fileName,spriteName);
		message.out(msgStr);
	#endif

	SpaceSprite *spr=new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 1);

	return spr;
}

bool general_GetSprites(SpaceSprite *Pics[], DATAFILE *datafile, char 
*cmdStr, int numSprites)
{

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++)
	{
		sprintf(dataStr,cmdStr,num);
		spr = general_GetSprite(datafile, dataStr);
		if(!spr)
		{
			return FALSE;
		}

		Pics[num]=spr;
	}

	return TRUE;
}




void Planets::init_objects()
{
	int		i;

	// the information in the dat header file contains information about how many different
	// planet and moon pics there are - read the header first, then allocate memory, then read
	// the pictures

	char *filename = "planets.dat";
	DATAFILE *data = load_datafile(filename);
	if(!data) error("Error loading '%s'", filename);

	DATAFILE *initdata = find_datafile_object(data, "gplanets_txt");
	if (!initdata) error("Error finding gplanet_txt");
	set_config_data((char *)(initdata->dat), initdata->size);

	Num_Moon_Pics = get_config_int(NULL,"NumMoonPics",0);
	Num_Planet_Pics = get_config_int(NULL,"NumPlanetPics",0);

	iMessage("Num_Moon_Pics  = %d *Preset*", Num_Moon_Pics);

	PlanetGrav = new double [Num_Planet_Pics];
	PlanetType = new int [Num_Planet_Pics];


	for ( i = 0; i < Num_Planet_Pics; ++i )
	{
		char name[512];
		const char	*result;
		sprintf(name, "Planet%03i\0", i);
		result = get_config_string(NULL, name, 0);
		if ( result )
			sscanf(result, "%f %i", &PlanetGrav[i], &PlanetType[i]);
		else
			error("Error in reading datafile planet");
	}

	MoonGrav = new double [Num_Moon_Pics];
	MoonType = new int [Num_Moon_Pics];

	for ( i = 0; i < Num_Moon_Pics; ++i )
	{
		char name[512];
		const char	*result;
		sprintf(name, "Moon%03i\0", i);
		result = get_config_string(NULL, name, 0);
		if ( result )
			sscanf(result, "%lf %i", &MoonGrav[i], &MoonType[i]);
		else
			error("Error in reading datafile moons");
	}


	PlanetPics = new SpaceSprite* [Num_Planet_Pics];
	MoonPics =   new SpaceSprite* [Num_Moon_Pics];


	if(GetSprites(PlanetPics, data,"Planet%03d",Num_Planet_Pics)==FALSE)
		error("File error, planet pics.  Bailing out...");

	if(GetSprites(MoonPics, data,"Moon%03d",Num_Moon_Pics)==FALSE)
		error("File error, moon pics.  Bailing out...");

	unload_datafile(data);

	//opening your .ini file.

	//log_file("planets.ini");
	set_config_file("planets.ini");
	//set_config_file("planet.ini");
//	int GasGrav = get_config_int(NULL, "Gasgravity",0);
//	double MoonGrav = get_config_float(NULL, "Moongravity", 0);

	//Probably a good idea to get all your variables immediately after

	int Comets = get_config_int(NULL, "Comets",0);
	int CoMass = get_config_int(NULL, "Comet_mass",0);
	int ComMax = get_config_int(NULL, "Comet_max",0);
	int Asteroids=get_config_int(NULL,"Asteroids",0);
	int ComAcc = get_config_int(NULL, "Comet_acc",0);


	int Radius = get_config_int(NULL, "Radius", 0);
	int PrimRadiusMax=get_config_int(NULL,"PrimRadiusMax",0);
	int MoonRandGrav=get_config_int(NULL,"MRandGrav",0);


	int MapSize = get_config_int(NULL,"MapSize",0);

	int MinMoonsInSystem = get_config_int(NULL, "MinMoonsInSystem", 0);
	int MaxMoonsInSystem = get_config_int(NULL, "MaxMoonsInSystem", 0);

	int OrbitRadiusMin = get_config_int(NULL,"OrbitRadiusMin",0);
	int OrbitRadiusMax = get_config_int(NULL,"OrbitRadiusMax",0);
	int OrbitMinDistance = get_config_int(NULL,"OrbitMinDistance",0);

	double OrbitPeriodMin = get_config_float(NULL, "OrbitPeriodMin", 0.0);
	double OrbitPeriodMax = get_config_float(NULL, "OrbitPeriodMax", 0.0);
	double MoonGravMin = get_config_float(NULL, "MoonGravMin", 0.0);
	double MoonGravMax = get_config_float(NULL, "MoonGravMax", 0.0);
	double MoonMaxEllipsE = get_config_float(NULL, "MoonMaxEllipsE", 0.0);

	// check if the ini file likes to override settings in .dat:

	for ( i = 0; i < Num_Moon_Pics; ++i )
	{
		char name[512];
		const char	*result;
		sprintf(name, "Moon%03i\0", i);
		result = get_config_string(NULL, name, 0);
		if (result)
			sscanf(result, "%f %i", &MoonGrav[i], &MoonType[i]);

		//iMessage("MoonType = %d *DATAFILE*", MoonType[i]);
		//message.print(10,10, "Moon default = %2i %8.4f", MoonType[i], MoonGrav[i]);
	}

	// initialize the possible planetary systems

	int NPlanets = get_config_int(NULL,"Nplanets",0);
	int iDefaultPlanet = get_config_int(NULL,"DefaultPlanet",0);

	// choose one of these planet systems:

	//i = random() % NPlanets;
	if (iDefaultPlanet == -1 ){
		i = random() % NPlanets;
		iMessage("i  = %d *RANDOM*", i);
	}
	else {
		i = iDefaultPlanet;
		iMessage("i  = %d *Preset*", i);
	}
	
	// required information:
	int iplanetpic, NumMoons, NumStarbases;
	int PlanetMoonPic[100];
	double	PlanetMoonEllipsW[100], PlanetMoonEllipsE[100],
			PlanetMoonEllipsAngvel[100], PlanetMoonEllipsOrientation[100],
			PlanetMoonGrav[100],
			PlanetStarbaseEllipsW[100], PlanetStarbaseEllipsE[100],
			PlanetStarbaseEllipsAngvel[100], PlanetStarbaseEllipsOrientation[100],
			PlanetPlGrav, PlanetPlGravRange;

	ChoosePlanetSystem(i, NPlanets, iplanetpic,
			NumMoons, MapSize,
			PlanetPlGrav, PlanetPlGravRange,
			PlanetMoonPic,
			PlanetMoonEllipsW, PlanetMoonEllipsE,
			PlanetMoonEllipsAngvel, PlanetMoonEllipsOrientation,
			PlanetMoonGrav,
			MinMoonsInSystem, MaxMoonsInSystem, OrbitRadiusMin, OrbitRadiusMax,
			OrbitPeriodMin, OrbitPeriodMax,
			MoonGravMin, MoonGravMax,
			MoonMaxEllipsE,
			NumStarbases,
			PlanetStarbaseEllipsW, PlanetStarbaseEllipsE,
			PlanetStarbaseEllipsAngvel,
			PlanetStarbaseEllipsOrientation);





	iMessage("HI !!!!   = %d *PRESET*",(size.x));


	//Select planet
	Planet *Centre;
	Planet *Satellite;
	OrbitHandler_PLSY *handler;


	Centre = new Planet(size/2, PlanetPics[iplanetpic],0);

	// adjust the gravity of the planet
	Centre->gravity_force *= PlanetPlGrav;
	Centre->gravity_range = PlanetPlGravRange;


	// always the same size would be best.
	size.x = size.y = MapSize;

	//prepare needs to be called before you add items, or do physics or graphics or anything like that.  Don't ask why.  
	prepare();

	add(new Stars());

	iMessage("Size   = %d *PRESET*",(size.x));
	game->add(Centre);
	game->add(new WedgeIndicator(Centre, 50, 4));	// this shows the direction/distance of the planet.


	// moons creating loop
	int kind;
	int num;
	for ( num = 0; num < NumMoons; num++)
	{
		//iMessage("treating num   = %d *PRESET*",num);
		kind = random()%2;

		int k = PlanetMoonPic[num];
		if (k == -1)	// this should not occur
			k = random() % Num_Moon_Pics;
		
		//iMessage("moonpic num   = %d *Preset*", k);

		Satellite = new Planet(size/2,MoonPics[k],0);
		handler = new OrbitHandler_PLSY ( Centre,
			(SpaceLocation *)Centre, (SpaceLocation *)Satellite,
			PlanetMoonEllipsW[num], PlanetMoonEllipsE[num],
			PlanetMoonEllipsAngvel[num], PlanetMoonEllipsOrientation[num]);
		Satellite->gravity_force *= PlanetMoonGrav[num]; // should be changed also

		//iMessage("Grav = %d *PRESET*", int(100*MoonGrav[k]));

		game->add(Satellite);
		game->add(handler);

		// this shows the location of the moons by grey lines (very messy when there are many moons).
		//game->add(new WedgeIndicator(Satellite, 50, 8));
	}
	iMessage("Done %i", 1);

	
//	OrbitHandler_PLSY(SpaceLocation *creator,
//	SpaceLocation *p_center, SpaceLocation *p_orbiter,
//	double lwidth, double lheight, double lspeed, double langle_orientation);


	//comet code
	for (num = 0; num < Comets; num++)
	{
		Planet *c;
		c = new Planet (random(size), kaboomSprite, 1);
		c->id=COMET_ID;
		c->mass = CoMass;
		c->gravity_force *= 0;
		c->gravity_whip = 0;
		//double a=(random(PI2));
	  //	c->vx=(ComMax/CoMass)*cos(a);
	  //	c->vy=(ComMax/CoMass)*sin(a);
		c->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2),
		get_config_int(NULL, "Comet_max", 2));
		add (c);
	}

	//asteroids code
	//Asteroids = get_config_int(NULL, "Asteroids", 0);
	Asteroid *astero;
	if (Asteroids > 0)
		for (int num = 0; num < Asteroids; num += 1)
		{
			add(astero = new(Asteroid));
		}
		else {
			NumMoons = get_config_int(NULL, "NPlanets", 2);
			Radius = get_config_int(NULL, "Radius", 2);
			if ((NumMoons*Radius+200)*2>3840) for (int i = 0; i < ((NumMoons*Radius+200)*2)/900; i += 1) add(new Asteroid());
			else for (int i = 0; i < 4; i += 1) add(new Asteroid());
		}
		


	// How to add a new (enemy) ship (no ship panel).
	// Create a new enemy player.
	// note, static means, the variable is kept in memory (I think? ) but is not accesible outside this subroutine.
	static TeamCode team_three = new_team();	// simply sets a (unique) number
	Ship *e;	// "WussieBot"

	for ( num = 0; num < NumStarbases; ++num )
	{
		e = create_ship(channel_none, "staba", "WussieBot", size/4, random(PI2), team_three);
		add(e->get_ship_phaser());
		// force the starbase in a fixed orbit:
		handler = new OrbitHandler_PLSY ( Centre,
			(SpaceLocation *)Centre, (SpaceLocation *)e,
			PlanetStarbaseEllipsW[num], PlanetStarbaseEllipsE[num],
			PlanetStarbaseEllipsAngvel[num], PlanetStarbaseEllipsOrientation[num]);
		game->add(handler);
	}


	// hmm, and what about a background !!! A nebula or so ?!

	//s = new spacebackground(0.0, 0.0);

	data = load_datafile("plbackground.dat");

	//BITMAP *backgrimage = get_data_bmp(data, "backgr_001");
	int Npics = 1;
	backgrimages = new SpaceSprite* [Npics];

	if(general_GetSprites(backgrimages, data,"backgr_%03d", Npics)==FALSE)
		error("File error, nebula pics.  Bailing out...");

	unload_datafile(data);


	double applyscale = 2.0;
	BigBackgr *nebula = new BigBackgr(backgrimages[0], MapSize/2, MapSize/2, applyscale);
	add(nebula);

	RadarMap *radar = new RadarMap(MapSize);
	add(radar);

}


Planets::~Planets()
{

	delete backgrimages;
	delete PlanetGrav, PlanetType, MoonGrav, MoonType, PlanetPics, MoonPics;

}




/*void Solar::init(Log *_log) {
	NormalGame::init(_log);
	log_file("Solar.ini");
	int j,m;
	j = get_config_int(NULL, "NPlanets", 2);
	m = get_config_int(NULL, "Radius", 2);
	if ((j*m+200)*2>3840) {
	width = (j*m+200)*2;
	height = (j*m+200)*2;
	}
	else {
	width = 3840;
	height = 3840;
	}
	return;
	}    */

REGISTER_GAME (Planets, "Melee in planetary system (INI)");



