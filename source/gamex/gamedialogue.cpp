

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

#include "../other/configrw.h"


static const int BlistLen = 128;


static GameAliendialog *dgame;


/** \brief add a ship to the player fleet
*/

lua_func(addship)
{
	arg_string(s);

	playerinfo.fleet.add(s);
	
}
lua_ret


/** \brief toggle state of a certain node in a active dialogue.
id = the node identifier
*/


lua_func(enable)
{
	Dialo *root = dgame->firstdialo;

	arg_string(id);

	Dialo *d;
	d = root->findnode(id);
	
	if (d)
		d->enable();

}
lua_ret

/*
lua_func(disable)
{
	arg_string(id);
}
lua_ret
*/









GameAliendialog::GameAliendialog()
:
GameBare()
{
	dialo = 0;
	racefile[0] = 0;
	dialofilename[0] = 0;
}


bool GameAliendialog::eol(char c)
{
	if (c == '\n' || c == 0)
		return true;
	
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

		if (text[i] == '<')
		{
			if (strncmp(&text[i], "<lua>", 5) == 0)
			{
				if (i > 0)
					if (text[i-1] == '\n')
						--n;		// you don't need to show the <lua> remark

				break;
			}
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
	dgame = this;

	// the TTF library
//	antialias_init(0);

    L = lua_init();

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
		set_dialog("races/earthling/colony.dialog");
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

	lua_close(L);

	// save the dialog node states
	FILE *outfile = getsavefile( replext(dialofilename, "bin") );
	if (outfile)
	{
		firstdialo->save_state(outfile);
		fclose(outfile);
	}

	// stop the ttf
	// or, should this be part of the twgui code ??
	//destroy_font(ttf);

	delete fs;
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

		Dialo *dialo_done = dialo;
		
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
					// and there is other stuff to do ... if the dialog has some
					// commands, then we'll need to execute those ...
					exec_commands(dialo_done);
					// do this before initblist, because of ... well, state change
					// otherwise, a node which has been altered, doesn't show up correctly
					// in the option-list (cause altering it after initblist makes the
					// alteration take no effect --> must be done before initblist).

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
					
					char *txt = showline(dialo->T, showline_num);
					A->set_textinfo( txt, strlen(txt) );


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
	strcpy(fname, "races/");
	strcat(fname, rc->race->id);
	strcat(fname, "/");
	strcat(fname, rc->dialogname);


	set_dialog(fname);

	race_id = rc->race->id;
}


void GameAliendialog::set_dialog(char *fname)
{
	// read the root branch !!
	strcpy(dialofilename, fname);
	fs = new FileStore(construct_loadname(fname));
	firstdialo = new Dialo();

	firstdialo->read(fs);

	dialo = firstdialo;


	// load/overwrite the dialog node states
	FILE *inpfile = getloadfile( replext(fname, "bin") );
	if (inpfile)
	{
		firstdialo->read_state(inpfile);
		fclose(inpfile);
	}

}


char playername[64];
char shipname[64];

/** \brief Stores relation between an look-up string and a value-string (a simple map system),
which is used to replace stuff in dialogues with strings stored in the game (eg ship name and
player name).
*/

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


/** \brief Scans for special <identifiers> (fgnameptr) in the text, and replaces those with game-content.

Currently the following commands are recognized:
<player>  inserts player name
<ship>    inserts ship name
*/
void GameAliendialog::preptext(char *t)
{

	int N = 2;
	fgNamePtr ptr[2];
	
	ptr[0].set("<player>", playerinfo.playername);
	ptr[1].set("<ship>", playerinfo.shipname);

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




/** \brief Check for commands at the end of the dialog text of the current dialog record,
which have to be executed after the record is read by the player.

*/

void GameAliendialog::exec_commands(Dialo *d)
{

	// find the start of commands
	char *txt = strstr(d->T, "<lua>");

	if (!txt)
		return;

	// all lines following <lua> are assumed to be lua-commands.

	//char *com = "addship(\"spael\");";

	txt += strlen("<lua>");

	lua_dochunk(L, &txt);
}

