
/*
Problems:
1. Q/A perturbed. Need way to distinguish between them
2. how to select another branch ?
*/

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE


#include "gamedialogue.h"
#include "edit/edit_dialogue.h"





int GameAliendialog::Nlines(char *text)
{
	int i, n;

	i = 0;
	n = 0;

	while (text[i] != 0 )
	{
		if (text[i] == '\n')
		{
			++n;
		}

		++i;
	}

	return n;
}

char *GameAliendialog::showline(char *text, int num)
{
	int i, n;

	i = 0;
	n = 0;

	while (text[i] != 0 && n < num )
	{
		if (text[i] == '\n')
		{
			++n;
		}

		++i;
	}

	int k;
	k = i;
	while ( text[k] != 0 && text[k] != '\n')
		++k;

	if (k > 127)
		k = 127;
	strncpy(show_line, &text[i], k-i);
	show_line[k-i] = 0;

	return show_line;
}


void GameAliendialog::init()
{
	scare_mouse();

	GameBare::init();

	double H = 2000;
	size = Vector2(H, H);
	prepare();


	int i;
	for ( i = 0; i < maxbranches; ++i )
		Blist[i] = new char [128];


	// read the root branch !!
	fs = new FileStore("gamex/dialogue/test.dialogue2");
	firstdialo = new Dialo();

	firstdialo->read(fs);

	dialo = firstdialo;
	


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

	FONT *usefont = videosystem.get_font(i);

	view->frame->prepare();


	// the background for this race is :
	
	strcpy(racefile, "");
	//strcpy(racefile, "gamex/dialogue/");
	//strcat(racefile, dialo.racepic);
	//racepic = load_bitmap(racefile, 0);


	AreaReserve *R;

	R = new AreaReserve("gamex/interface/aliendialog", 10, 10, view->frame->surface);


	


	int tcol = makecol(255,255,128);

	// ???
	showline_num = 0;
	showline_Nlines = Nlines(dialo->T);
	//A = new TextEditBox(R, "A", -1, -1, usefont, showline(dialo.A, showline_num), 0);
	A = new TextInfoArea(R, "A", -1, -1, usefont);	// all text should fit on 1 window - no scroll needed !!
	//A->set_textcolor(tcol);

	// ????
	//A->changebackgr(racefile);


	B = new TextList(R, "B", -1, -1, usefont);

	
	winman = new WindowManager;
	winman->add(R);

	initBlist(dialo);

}


void GameAliendialog::quit()
{
	int i;
	for ( i = 0; i < maxbranches; ++i )
		delete Blist[i];


	scare_mouse();
	set_mouse_sprite(0);	// the default mouse sprite.
	show_mouse(screen);	// ????????
	//show_mouse();

	GameBare::quit();
}


// this should be named "initeditor"
void GameAliendialog::initBlist(Dialo *dialo)
{
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
			
			if (d->state)
			{
				strncpy(Blist[k], d->T, 127);
				Blist[k][127] = 0;
				//branchindex[k] = d->n;
				dialo_selected[k] = d;		// ???
				++k;
			}
			
		}		
	} else 
		Blist[0][0] = 0;

	Nactivebranches = k;
	B->set_optionlist(Blist, Nactivebranches, makecol(255,255,255));

//	A->text_reset();
	showline_num = 0;
	char *txt = showline(dialo->T, showline_num);
	A->set_textinfo( txt, strlen(txt) );

	showline_Nlines = Nlines(dialo->T);


	char tmp[128];
	strcpy(tmp, "gamex/dialogue/");
	strcat(tmp, dialo->racepic);
	if (strcmp(tmp, racefile) != 0)
	{
		strcpy(racefile, tmp);
		A->changebackgr(racefile);
	}
}



void GameAliendialog::calculate()
{
	if (next)
		return;

	GameBare::calculate();


	FULL_REDRAW = true;
	winman->calculate();



	if (A->flag.left_mouse_press)
	{

		// next line to show
		if (showline_num < showline_Nlines)
		{
			++showline_num;
			char *txt = showline(dialo->T, showline_num);
			A->set_textinfo( txt, strlen(txt) );

		} else {

			// change state, and go back to parent
			dialo->state = 0;
			//dialo.write(&dstore);	// you've to store in the temp-mem blocks.
			
			//dialo.read(dialo.mother, &dstore);
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
					dialo->state = 0;

					dialo = dialo->mother;

					initBlist(dialo);
				}
			}
		}


	}

	
	// go forth to the currently selected branch

	//if (bnext->flag.left_mouse_press)
	if (B->flag.left_mouse_press)
	{
		// load the data from the child branch.
		if (dialo->Nbranches > 0)
		{
			int i;
			i = B->scroll.yselect;
			if (i >= 0 && i < Nactivebranches)	// shouldn't be necessary ?!
			{
				//int k = branchindex[i];

				//dialo.read(k, &dstore);
				dialo = dialo_selected[i];

				// So, now you've loaded the question; what you want is the next step,
				// namely the alien answer.
				if (dialo->Nbranches > 0)
				{
					int i = 0;

					// For now, it defaults to branch 0
					dialo = dialo->branch[i];
				}

				initBlist(dialo);
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
	winman->setscreen(view->frame->surface);
	winman->animate();
	show_mouse(view->frame->surface);
	scare_mouse();
}



