


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
REGISTER_FILE

#include "../../melee/mview.h"
#include "../../melee/mcbodies.h"
#include "../../frame.h"

#include "tredit.h"


#include "trtest.h"
#include "trcontrol.h"
#include "trvarsetting.h"
#include "trmath.h"
#include "trgamesetting.h"
#include "trships.h"

#include "../../melee/mgame.h"
#include "../../twgui/twgui.h"

//#include "gamex/projectx.h"




action_startfunction *trmain;

WindowManager *winman;

TextButton **b;
TextList *listarea;
TextEditBox *editarea;

Button *line_del, *run_game;

AreaReserve *Amain, *Alist, *Aedit;

action_func		**action_ptr;

const int maxedittxt = 512;
char edittxt[maxedittxt];
int edit_var_num, edit_var_value;
char edit_var_type;

char **idlist, **ivarlist, **svarlist;


// in order to expand several subblocks (for, if) into a linear structure, use :
const int max_lines = 10000;
int line_N;	// now many lines are there ..
action_func *line_func[max_lines];
char		line_level[max_lines];



class GameTriggerEdit : public Game
{
public:

	enum {T_MENU, T_GAME};
	int gtype;

	int Nx, Ny, dx, dy;
	int	changelocation;
	action_func	*changefunc_ptr;

	virtual void init(Log *_log);
	virtual void quit(const char *s);
	//virtual bool handle_key(int k);

	void calculate_menu();
	void calculate_game();
	void animate_menu(Frame *frame);
	void animate_game(Frame *frame);
	void init_game();


	virtual void calculate();
	virtual void animate(Frame *frame);

	void update_buttons();

	char **get_id_list(char vartype);

};



void GameTriggerEdit::init(Log *_log)
{
	int i;


	Game::init(_log);

	scare_mouse();

	double H = 2000;
	size = Vector2(H, H);
	prepare();


	// initialize the existing action types :

	/*
//	action_id.add(new action_startfunction() );		// don't add: it's an init function for the main program action: initialized by the main program
//	action_id.add(new action_subr() );				// don't add it; it's an init function and a variable (doh)
	action_id.add(new action_if() );
	action_id.add(new action_for() );
	action_id.add(new action_setI() );
	action_id.add(new action_convID() );
	action_id.add(new action_addI() );
	action_id.add(new action_subI() );
	action_id.add(new action_mulI() );
	action_id.add(new action_divI() );

	action_id.add(new action_mapsize() );
	action_id.add(new action_playership() );
	action_id.add(new action_AIship() );
	action_id.add(new action_newteam() );
	*/

	action_id.init();

	// create the main program action:
	trmain = new action_startfunction();
	trmain->define();
	//trmain->define_treat();

	action_vars.read_vars("gamedata/trvariables.bin");
	trmain->read("gamedata/tractions.bin");

	// initialize this ?
//	FILE *f;
//	f = fopen("data/trtest/test.trigger", "rb");
	// first init variables
	// ..
	// then init the main program
//	if (f)
//		trmain->init_block(f);



	// initialize menu system.

	unscare_mouse();
	show_mouse(window->surface);


	// which font to use ... that depends on the screen resolution:
	i = 1;
	if (screen->w == 640)
		i = 1;
	if (screen->w == 800)
		i = 2;
	if (screen->w == 1024)
		i = 3;					// more pixels available for the same "real-life" size.

	FONT *usefont = videosystem.get_font(i);

	view->frame->prepare();





	Amain = new AreaReserve("interfaces/gtriggers/main2", 0, 0, view->frame->surface);

	
	// create a bunch of buttons ...
	int ix, iy;

	Nx = 8;
	Ny = 10;

	dx = 80;
	dy = 40;

	b = new TextButton* [Nx*Ny];

	action_ptr = new action_func* [Ny];

	for ( iy = 0; iy < Ny; ++iy )
	{
		for ( ix = 0; ix < Nx; ++ix )
		{

			int k;
			k = iy*Nx+ix;
			b[k] = new TextButton(Amain, "button", (ix+0.5)*dx, (iy+0.5)*dy, usefont);		
			b[k]->set_text("", makecol(0,0,0));
		}
	}

	line_del = new Button(Amain, "linedel", 0, 0, 0);

	run_game = new Button(Amain, "rungame", 750, 0, 0);


	// also, define a popup window to display a list of strings:

	Alist = new AreaReserve("interfaces/gtriggers/popup_list", 0, 0, view->frame->surface);
	Alist->hide();

	listarea = new TextList(Alist, "text", -1, -1, usefont);


	Aedit = new AreaReserve("interfaces/gtriggers/edit_box", 0, 0, view->frame->surface);
	Aedit->hide();
	
	editarea = new TextEditBox(Aedit, "text", 0, 0, usefont, edittxt, maxedittxt);
	editarea->set_textcolor(makecol(0,0,0));



	// the window manager.

	winman = new WindowManager;
	winman->add(Amain);
	winman->add(Alist);
	winman->add(Aedit);

	// prepare some lists ... for use in the popup window ...
	// first of all, a list of the function ids: to select a new function.

	

	idlist = new char* [action_id.N+1];

	for ( i = 0; i < action_id.N; ++i )
		idlist[i] = action_id.action[i]->id;
	idlist[i] = 0;


	ivarlist = new char* [max_action_vars+1];
	// don't use nI, cause that's 0 in the start (or can be).

	for ( i = 0; i < max_action_vars; ++i )
		ivarlist[i] = action_vars.intvarid[i];
	ivarlist[i] = 0;


	svarlist = new char* [max_action_vars+1];
	// don't use nI, cause that's 0 in the start (or can be).

	for ( i = 0; i < max_action_vars; ++i )
		svarlist[i] = action_vars.strvarid[i];
	svarlist[i] = 0;


	update_buttons();

	edit_var_num = -1;
	edit_var_value = -1;
	changefunc_ptr = 0;

	// start always in menu mode.
	gtype = T_MENU;
}


char **GameTriggerEdit::get_id_list(char vartype)
{
	switch (vartype)
	{
	case 'I':
		return ivarlist;
		break;
	case 'S':
		return svarlist;
		break;
	}

	return 0;
}


void GameTriggerEdit::quit(const char *s)
{
	// write the vars to disk ...

	action_vars.write_vars("gamedata/trvariables.bin");
	trmain->write("gamedata/tractions.bin");

	delete idlist;
	delete ivarlist;

	unscare_mouse();
	unscare_mouse();
	//show_mouse(screen);
	Game::quit(s);
}



void GameTriggerEdit::calculate_menu()
{
	Game::calculate();


	// check if a button has been pressed:

	int ix, iy;
	for ( iy = 0; iy < Ny; ++iy )
	{
		for ( ix = 0; ix < Nx; ++ix )
		{

			int k;
			k = iy*Nx+ix;

			TextButton *tb;
			tb = b[k];
			if (tb->flag.left_mouse_press)
			{
				changelocation = ix;
				changefunc_ptr = action_ptr[iy];

				// first column: select a function from the action-list.
				if (ix == 0)
				{
					Alist->show();
					Amain->disable();
					listarea->set_optionlist(idlist, makecol(0,0,0));
				}

				if (ix % 2 == 1 && changefunc_ptr && ix/2 < changefunc_ptr->Narg)
				{
					//listarea->set_optionlist(ivarlist, makecol(0,0,0));
					edit_var_type = changefunc_ptr->vartype[ix/2];
					listarea->set_optionlist(get_id_list(edit_var_type), makecol(0,0,0));

					/*
					switch (changefunc_ptr->vartype[(ix-1)/2])
					{
					case 'I':
						listarea->set_optionlist(ivarlist, makecol(0,0,0));
						break;
					case 'S':
						listarea->set_optionlist(svarlist, makecol(0,0,0));
						break;
					}
					*/

					Alist->show();
					Amain->disable();
				}

			}
	
			
			// if you press right on a "variable" part, then you define a constant ...
			// a "constant" has some pre-defined name, and the thing you enter, is the value!!
			// (should be converted from string to the required value type).
			if (tb->flag.right_mouse_press)
			{
				changelocation = ix;
				changefunc_ptr = action_ptr[iy];

				// create a new variable (you don't need a unique name for a const)
				// (assume type I (=int) for now)
				// well, except if this position already has a constant --> overwrite it
				if (ix > 0 && ix % 2 == 1 && changefunc_ptr && ix/2 < changefunc_ptr->Narg)
				{
					int ipos;
					ipos = changelocation/2;
					edit_var_type = changefunc_ptr->vartype[ipos];

					char *id;
					// CHANGE THIS (and other stuff)
					k = changefunc_ptr->varindex[ipos];
					id = action_vars.get_id(edit_var_type, k);

					if (k < 0 || strcmp(id, "constant") != 0)
					{
						//k = action_vars.nI;
						//++action_vars.nI;

						k = action_vars.newvar(edit_var_type, "constant");
						--k;	// now points to the last element...
					}
					
					//action_vars.set_id(edit_var_type, k, "constant"); useless ;)
					//action_vars.intvarid[edit_var_num][var_id_len-1] = 0;
					
					changefunc_ptr->varindex[ipos] = k;
					// now the only thing left is, to define its value
					
					Amain->disable();
					Aedit->show();
					edit_var_value = k;
					
				}
			}


		}
	}







	// check if the list has been pressed ...

	if (listarea->flag.left_mouse_press)
	{
		if (changelocation == 0)
		{
			// add an action type, corresponding to this row
			// (you cannot change it - you've to delete it if you want to get rid of it)
			int k;
			action_func *a;
			
			k = listarea->scroll.yselect;

			a = action_id.create(listarea->optionlist[k]);
			a->define();
			//a->define_treat();
			//a->insert(trmain->main_ptr, trmain->main_ptr->next);
			if (changefunc_ptr)
			{
				if (changefunc_ptr == trmain->main_ptr)
					trmain->main_ptr = a;

				a->insert(changefunc_ptr->prev, changefunc_ptr);
			}
			else
				trmain->append(a);

			Alist->hide();
			Amain->show();

		} else {

			// define or change variable reference.
			int k, m;
			k = listarea->scroll.yselect;
			m = action_vars.N(edit_var_type);
			if (k == -1 || k >= m )			//action_vars.nI)
			{
				//k = action_vars.nI;
				//++action_vars.nI;
				k = action_vars.newvar(edit_var_type, "");	// a zero string.
				--k;
			}

			// first check, if this variable already exists; if not, define its id
			// (but not its value ? no, it's a variable !!)
			// assume an "int", for now ...
			if (action_vars.get_id(edit_var_type, k)[0] == 0 && changefunc_ptr)		//.intvarid[k][0]
			{
				Alist->hide();
				Aedit->show();

				edit_var_num = k;
				//edit_var_type = changefunc_ptr->vartype[changelocation/2]; already defined

			} else {
				Alist->hide();
				Amain->show();
			}
			
			if (changefunc_ptr)
				changefunc_ptr->varindex[changelocation/2] = k;
		}

		// change the button text as well ? or, update all the buttons ?
		update_buttons();

		
	}






	// check if a variable has been entered

	if (editarea->flag.right_mouse_press)
	{
		if (edit_var_num >= 0)
		{
		
			//strncpy(action_vars.intvarid[edit_var_num], edittxt, var_id_len-1);
			
			action_vars.set_id(edit_var_type, edit_var_num, edittxt);

			//action_vars.intvarid[edit_var_num][var_id_len-1] = 0; not needed
			//action_vars.intvar[edit_var_num] = 0; not needed
			edit_var_num = -1;

		}

		if (edit_var_value >= 0)
		{

			// assume an integer value for now
			//action_vars.intvar[edit_var_value] = atoi(edittxt);
			action_vars.set_val(edit_var_type, edit_var_value, edittxt);

			edit_var_value = -1;

		}

		Aedit->hide();
		Amain->enable();

		update_buttons();
	}


	// position and use the linedel button

	iy = int(mouse_y-0.5*dy) / dy;
	line_del->x = 750;
	line_del->y = (iy + 0.5) * dy;

	if (line_del->flag.left_mouse_press)
	{
		// delete the contents of this line.
		// except if it's the 1st line of the series...

		if (action_ptr[iy])
		{
			// the main code block should always point to a valid line
			if (action_ptr[iy] == trmain->main_ptr)
				trmain->main_ptr = action_ptr[iy]->next;

			delete action_ptr[iy];
		}

		update_buttons();
	}


	if (run_game->flag.left_mouse_press)
	{
		init_game();

		Amain->disable();
	}



	FULL_REDRAW = true;
	winman->calculate();

}


void GameTriggerEdit::animate_menu(Frame *frame)
{

	Game::animate(frame);


	//show_mouse(frame->surface);
	winman->setscreen(view->frame->surface);
	winman->animate();
	show_mouse(view->frame->surface);
	scare_mouse();
}


void GameTriggerEdit::animate_game(Frame *frame)
{
	Game::animate(frame);
}


void GameTriggerEdit::animate(Frame *frame)
{

	switch (gtype)
	{
	case T_MENU:
		animate_menu(frame);
		break;
	case T_GAME:
		animate_game(frame);
		break;
	}

}






void GameTriggerEdit::update_buttons()
{
	action_func *a;
	a = trmain->main_ptr;

	if (!a)
	{
		int iy;
		for ( iy = 0; iy < Ny; ++iy )
			action_ptr[iy] = 0;
		return;
	}






	// create a (new) array of pointers to the subsequent action (blocks):


//	int level, N, line_max;
//
//	line_max = 100;
//	level = 0;
//	a->addme(line_func, line_level, N, level, line_max);










	int yselect = 0;
	int i;

	// skip scrolled part
	i = 0;
	while (i < yselect && a != 0)
	{
		++i;
		a = a->next;
	}

	if (!a)
		return;

	i = 0;
	for ( i = 0; i < Ny; ++i )
	{
		if (!a)
			action_ptr[i] = 0;
		else
		{
			action_ptr[i] = a;
			a = a->next;
		}
	}


	int ix, iy;
	for ( iy = 0; iy < Ny; ++iy )
	{
		if (!action_ptr[iy])
		{
			for ( ix = 0; ix < Nx; ++ix )
			{
				int k;
				k = iy*Nx+ix;

				// make sure that unused lines look empty.
				b[k]->set_text(0, makecol(0,0,0));
			}

			break;
		}

		for ( ix = 0; ix < Nx; ++ix )
		{

			int k;
			k = iy*Nx+ix;

			int j;

			j = ix / 2;

			if (ix % 2 == 0)
			{
				b[k]->set_text(action_ptr[iy]->descr[j], makecol(0,0,0));

			} else {

				int index;
				index = action_ptr[iy]->varindex[j];

				if (index >= 0)
				{
					char itype;
					itype = action_ptr[iy]->vartype[j];
					char *txt, txtvalue[512];

					txt = action_vars.get_id(itype, index); //.intvarid[index];

					// if the text is a variable, then show the variable name;
					// if it's of type "constant", then show the value
					if (strcmp(txt, "constant") == 0)
					{
						//int k;
						//k = action_vars.intvar[index];
						//itoa(k, txtvalue, 10);	// base-10

						action_vars.get_val(itype, index, txtvalue);
						txt = txtvalue;
					}

					b[k]->set_text(txt, makecol(0,0,0));
				}
				else if (j < action_ptr[iy]->Narg)
					b[k]->set_text("<undefined>", makecol(0,0,0));

				else
					b[k]->set_text(0, makecol(0,0,0));
			}
		}

	}

}



void GameTriggerEdit::calculate_game()
{
	Game::calculate();
}

void GameTriggerEdit::calculate()
{
	switch (gtype)
	{
	case T_MENU:
		calculate_menu();
		break;
	case T_GAME:
		calculate_game();
		break;
	}
}




void GameTriggerEdit::init_game()
{
	gtype = T_GAME;

	prepare();

	Stars *stars = new Stars();
	add(stars);

	// right ... this one is important !!
	// this initializes the game using the trigger actions; it does 1 calculate
	// of the main_ptr ... yeah :)

	trmain->calculate();

	// this is all that is needed ... I think ;)
}





REGISTER_GAME(GameTriggerEdit, "Miaauuuw")

