

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE


#include "gamedialogue.h"
#include "edit/edit_dialogue.h"

//#include "allegttf.h"
//#include "allegro/internal/aintern.h"

#include "../other/ttf.h"
#include "../other/fontmorph.h"

#include "general/sprites.h"

static const int BlistLen = 128;


GameAliendialog::GameAliendialog()
:
GameBare()
{
	dialo = 0;
	racefile[0] = 0;
}


bool GameAliendialog::eol(char c)
{
	if (c == '\n' || c == 0)
		return true;
	else
		return false;
}

int GameAliendialog::Nlines(char *text)
{
	int i, n;

	i = 0;
	n = 0;

	while (text[i] != 0 )
	{
		if (eol(text[i]))
		{
			if (!eol(text[i+1]))	// skip empty lines ?
				++n;
		}

		++i;
	}

	++n;	// 0 also marks the end of a line.

	return n;
}

char *GameAliendialog::showline(char *text, int num)
{
	int i, n;

	i = 0;
	n = 0;

	while (text[i] != 0 && n < num )
	{
		if (eol(text[i]))
		{
			if (!eol(text[i+1]))	// skip empty lines ?
				++n;
		}

		++i;
	}

	int k;
	k = i;
	while ( !eol(text[k]))
		++k;

	if (k > 127)
		k = 127;
	strncpy(show_line, &text[i], k-i);
	show_line[k-i] = 0;

	return show_line;
}



void GameAliendialog::init_menu()
{
	T = new TWindow("gamex/interface/aliendialog", 0, 0, game_screen, true);
	maparea = 0;
}


void GameAliendialog::init()
{
	// the TTF library
//	antialias_init(0);


	// ttf font loading test
	FONT *ttf;

	int p;
	p = 24;
	char *fname;
	//fname = "fonts/jobbernole.ttf";
	fname = "fonts/lynx.ttf";
	ttf = load_ttf_font(fname, p, 0);

	if (!ttf)
	{
		tw_error("font isn't loaded");
	}
	

	// end of ttf test

	morph_font(ttf);

	if (!window) {
		window = new VideoWindow();
		window->preinit();
	}
	prepare();
	change_view("Hero"); 
	//view->window->locate(0,0,0,0,  0,1,0,1);

	size = Vector2(100,100);
	wininfo.init( Vector2(800,800), 800.0, view->frame );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.

	int i;
	for ( i = 0; i < maxbranches; ++i )
		Blist[i] = new char [BlistLen];

	
	scare_mouse();

	GameBare::init();

	double H = 2000;
	size = Vector2(H, H);
	prepare();

	ti = false;	// no render/tic time info needed...


	if (!dialo)
	{
		set_dialog("gamex/gamedata/races/earthling/colony.dialog");
		race_id = "earthling";
	}


	// initialize menu system.

	unscare_mouse();
	show_mouse(window->surface);


	// which font to use ... that depends on the screen resolution:
	i = 2;
	if (screen->w == 640)
		i = 4;
	if (screen->w == 800)
		i = 5;
	if (screen->w == 1024)
		i = 6;					// more pixels available for the same "real-life" size.

	FONT *usefont;
	//usefont = videosystem.get_font(i);
	usefont = ttf;



	//int tcol = makecol(255,255,128);

	A = new TextInfoArea(T, "A/", usefont, 0, 0);	// all text should fit on 1 window - no scroll needed !!
	A->passive = false;

	B = new TextList(T, "B/", usefont);

	
	T->tree_doneinit();

	// dialo should be defined BEFORE the game is initialized by a call to set_colony
	initBlist(dialo);
}


void GameAliendialog::quit()
{
	int i;
	for ( i = 0; i < maxbranches; ++i )
		delete Blist[i];


	scare_mouse();
	set_mouse_sprite(0);	// the default mouse sprite.
	show_mouse(screen);

	GameBare::quit();

	// stop the ttf
//	antialias_exit();
	//destroy_font(ttf);
}


// this should be named "initeditor"
void GameAliendialog::initBlist(Dialo *dialo)
{
	// make small changes, to change e.g. captain name or ship name to the correct on
	preptext(dialo->T);

	Dialo *d;

	int k;
	k = 0;

	// find its branches :
	if (dialo->Nbranches > 0)
	{
		//char Q[256];
		//char A[2048];
		
		int i;

		// only show "active" branches
		for ( i = 0; i < dialo->Nbranches; ++i )
		{
			//d->read(dialo->branch[i], &dstore);
			d = dialo->branch[i];

			preptext(d->T);
			
			if (d->state)
			{
				strncpy(Blist[k], d->T, BlistLen-1);
				Blist[k][BlistLen-1] = 0;
				//branchindex[k] = d->n;
				dialo_selected[k] = d;		// ???
				++k;
			}
			
		}		
	} else 
		Blist[0][0] = 0;

	Nactivebranches = k;
	int col;
	//col = makecol(255,255,255);
	col = -1;	// for color-fonts.
	B->set_optionlist(Blist, Nactivebranches, col);
	//set_palette(default_palette);

//	A->text_reset();
	showline_num = 0;
	char *txt = showline(dialo->T, showline_num);
	A->set_textinfo( txt, strlen(txt) );
	A->text_color = -1;	// so that font-colors are used.

	showline_Nlines = Nlines(dialo->T);


	char tmp[128];
	strcpy(tmp, "gamex/gamedata/races/");
	//strcpy(tmp, race_id);
	strcat(tmp, dialo->racepic);
	if (strcmp(tmp, racefile) != 0)
	{
		strcpy(racefile, tmp);
		A->changebackgr(racefile);
	}
}



void GameAliendialog::calculate()
{
	if (!dialo)		// if there's no dialogue defined... then it's no use to continue
	{
		tw_error("Dialog is missing!");
		quit();
		return;
	}

	if (next)
		return;

	wininfo.center( 0.5*size );
	
	GameBare::calculate();


	if (A->flag.left_mouse_press || keyhandler.keyhit[KEY_SPACE])
	{

		// next line to show
		if (showline_num < showline_Nlines)
		{
			++showline_num;
			char *txt = showline(dialo->T, showline_num);
			A->set_textinfo( txt, strlen(txt) );
		}
	}

	// this can also happen, if you return from a child node after the last
	// question has been answered ... in that case, suddenly all subbranches
	// are "done" and then this node has to collapse as well.
	if (showline_num == showline_Nlines)
	{
		
		// change state, and go back to parent
		// hmm, no, only do this if all sub-questions are "done"
		
		dialo->check_state();
		
		if (!dialo->state)
		{
			
			if (dialo->mother)
			{
				dialo = dialo->mother;
				// the parent's parent is the real on you've to get (since doing once
				// goes back to the question node, going back one further goes to
				// the alien-talk node.
				if (dialo->mother)
				{
					// disable the question node, and go back
					// (a bit simple ... should really only be done after checking that
					// all sub-questions are disabled already - but this is a simple
					// test case).
					dialo->check_state();
					
					dialo = dialo->mother;
					
					initBlist(dialo);
					
					// since you return, that means that the previous
					// node has already been "done"
					showline_num = showline_Nlines;
				}
			}
		}
	}



	
	// go forth to the currently selected branch
	// but, you should only enable this, if the "alien" text is completely read

	if (B->flag.left_mouse_press && showline_num == showline_Nlines)
	{
		// load the data from the child branch.
		if (dialo->Nbranches > 0)
		{
			int i;
			i = B->gety();//scroll.yselect;
			if (i >= 0 && i < Nactivebranches)	// shouldn't be necessary ?!
			{
				//int k = branchindex[i];

				//dialo.read(k, &dstore);
				dialo = dialo_selected[i];

				// So, now you've loaded the question; what you want is the next step,
				// namely the alien answer.
				if (dialo->Nbranches > 0)
				{
					//int i = 0;

					// For now, it defaults to branch 0
					dialo = dialo->branch[dialo->get_branch()];
				} else {

					// generate a default "answer" which tells you there's a missing link
					Dialo *d;
					d = new Dialo();
					d->init_default();	// this also sets mother=0
					d->mother = dialo;


					dialo = d;
				}

				initBlist(dialo);

				showline_num = 0;
			}
		}
	}

}



void GameAliendialog::animate(Frame *frame)
{
	if (next)
		return;

	GameBare::animate(frame);


	//show_mouse(frame->surface);
	//R->tree_setscreen(view->frame->surface);
	//R->tree_animate();
	//show_mouse(view->frame->surface);
	//scare_mouse();
}



void GameAliendialog::set_colony(RaceColony *rc)
{
	if (rc->dialogname[0] == 0)		// no filename defined.
		return;

	// load the dialog for this colony ?!

	char fname [512];
	strcpy(fname, "gamex/gamedata/races/");
	strcat(fname, rc->race->id);
	strcat(fname, "/");
	strcat(fname, rc->dialogname);


	set_dialog(fname);

	race_id = rc->race->id;
}


void GameAliendialog::set_dialog(char *fname)
{
	// read the root branch !!
	fs = new FileStore(fname);
	firstdialo = new Dialo();

	firstdialo->read(fs);

	dialo = firstdialo;
}


char playername[64];
char shipname[64];

struct fgNamePtr
{
	char id[64];
	char *ptr;

	void set(char *newid, char *newptr);
};

void fgNamePtr::set(char *newid, char *newptr)
{
	strcpy(id, newid);
	ptr = newptr;
}


// NOTE: this does not check for end-of-text...
void replace_text(char *txt, char *rem, char *ins)
{
	int L;
	L = strlen(txt) + 1;	// include 0.

	int k;
	k = strlen(ins) - strlen(rem);
	if (k > 0)
		memmove(&txt[k], &txt[0], L-k);
	if (k < 0)
		memmove(&txt[0], &txt[-k], L+k);

	memcpy(&txt[0], ins, strlen(ins));
}


void GameAliendialog::preptext(char *t)
{
	strcpy(playername, "zelnick");
	strcpy(shipname, "titanic");
	// <player> insert player name there
	// <ship> insert ship name there


	int N = 2;
	fgNamePtr ptr[2];
	
	ptr[0].set("<player>", playername);
	ptr[1].set("<ship>", shipname);

	int i = 0;
	while ( t[i] )
	{
		char *id;
		id = &t[i];

		int k;
		for ( k = 0; k < N; ++k )
		{
			if (strncmp(id, ptr[k].id, strlen(ptr[k].id)) == 0)
			{
				// succesful comparison; replace the string
				replace_text(id, ptr[k].id, ptr[k].ptr);
			}
		}

		++i;
	}
}














/*

#include "freetype/freetype.h"


void cleanup (FT_Library engine, FT_Face face)
{
	if (engine)
		FT_Done_FreeType(engine);
}


// copy ttf bitmap onto an allegro bitmap
void my_draw_bitmap( FT_Bitmap *src, BITMAP *dest, int dx, int dy )
{
	if (src->rows == 0 || src->width == 0)
		return;

	int ix, iy;

	if (src->pixel_mode == ft_pixel_mode_mono)
	{

		// dunno if this actually works ?
		int ix, iy;
		for ( iy = 0; iy < src->rows; ++iy )
		{
			unsigned char *p = src->buffer + iy * src->pitch;
			
			for ( ix = 0; ix < src->width; ++ix )
			{
				unsigned char v;
				v = p[ix >> 8];
				
				unsigned char mask;
				mask = 1 << (ix & 0x0F);
				
				if (v & mask)
					putpixel(dest, ix+dx, iy+dy, 255);
			}
		}
		
	} else if ( src->pixel_mode == ft_pixel_mode_grays )
	{
		for ( iy = 0; iy < src->rows; ++iy )
		{
			unsigned char *p = src->buffer + iy * src->pitch;
			
			for ( ix = 0; ix < src->width; ++ix )
			{
				unsigned char v;
				v = p[ix];
				
				//if (v != 0)
				if (v > 128)
					putpixel(dest, ix+dx, iy+dy, 255);
			}
		}
		
	} else {
		tw_error("unsupported bitmap format");
	}


}



FONT* load_ttf_font (AL_CONST char* filename,
                     AL_CONST int points,
                     AL_CONST int smooth)
{

	int points_w, points_h, begin, end;
	begin = 32;	// space
	end = 128;	// ?
	points_w = points;
	points_h = points;


	FT_Library           engine = 0;
	FT_Face              face = 0;
	
	
	FT_Error error;

	// initialize the library
	error = FT_Init_FreeType(&engine);
	if(error)
		return 0;
		
	// load a font
	error = FT_New_Face(engine, filename, 0, &face);
	if(error)
	{
		cleanup(engine, face);
		return 0;
	}

	// set font size in pixels
	error = FT_Set_Pixel_Sizes(face, points_w, points_h);
	if(error)
	{
		cleanup(engine, face);
		return 0;
	}


	// in case there's no unicode (ascii) charmap
	if (!face->charmap)
	{
		if (!face->num_charmaps)
		{
			cleanup(engine, face);
			return 0;
		}

		FT_Set_Charmap(face, face->charmaps[0]);	// just pick the first one .
	}

	if (!face->charmap)
	{
		cleanup(engine, face);
		return 0;
	}
	
	
	int c;
	
	AL_CONST int num = end - begin + 1;
	
	struct FONT *f;
	struct FONT_COLOR_DATA *fcd;
	
	// Allocate and setup the Allegro font
	// (copied from allegttf)
	f = (struct FONT*)calloc(1,sizeof(struct FONT));
	fcd = (struct FONT_COLOR_DATA*)calloc(1,sizeof(struct FONT_COLOR_DATA));
	fcd->begin = begin;
	fcd->end = end;
	fcd->bitmaps = (BITMAP**)calloc(num,sizeof(BITMAP*));
	fcd->next = NULL;
	f->data = (void*)fcd;
	f->vtable = font_vtable_color;
	


	FT_GlyphSlot  slot = face->glyph;  // a small shortcut
	int           pen_x, pen_y;
	
	pen_x = 0;
	pen_y = 0;

	int ymin, ymax, yorigin;
	
	ymin = face->bbox.yMin;		// the bounding box for all chars in this font.
	ymax = face->bbox.yMax;		// this actually defines the origin position...
	yorigin = points_h * double(ymax) / double(ymax - ymin);
	
	//char c;
	for ( c = begin; c <= end; ++c )
	{
		// load glyph image into the slot (erase previous one)
		
		error = FT_Load_Char( face, c, FT_LOAD_RENDER );
		if (error)
			continue;
		
		// now, draw to our target surface
		BITMAP *bmp = 0;
		int w, h;

		w = slot->metrics.horiAdvance / 64;
		if (!w)
			w = 1;

		h = points_h;
		
		int dx, dy;

		dx = slot->bitmap_left;
		dy = yorigin - slot->bitmap_top;

		if (w && h)
		{
			bmp = create_bitmap_ex(8, w, h);
			clear_to_color(bmp, 0);
			
			my_draw_bitmap( &slot->bitmap, bmp, dx, dy );
		}

		fcd->bitmaps[c-begin] = bmp;
	}

	
	// set the font height
	f->height = points_h;
	
	// clean up the font stuff
	cleanup(engine, face);
	
	return f;
   }
   
   */



