
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
REGISTER_FILE


#include "edit_dialogue.h"



// create a dialogue structure

Dialo::Dialo()
{
	Nbranches = 0;
	Ntriggers = 0;
	mother = 0;
}

Dialo::~Dialo()
{
}

void Dialo::read(FileStore *fs)
{

	// in case it's a new file, then start with some basic information
	if (!mother && fs->size() == 0)
	{
		strcpy(T, "noconvoyet");
		
		strcpy(racepic, "noraceyet");
		version = 1;
		mother = 0;
		// branches of text (= new question list) spawned by the answer.
		Nbranches = 0;
		Ntriggers = 0;
		// initial state
		state = 1;

		return;
	}

	fs->read(&version);

	fs->read(&state);

	fs->read(&Nbranches);

	fs->read(&Ntriggers);
	fs->read(trigger, Ntriggers);

	fs->initstring(T, sizeof(T));	// does _not_ create a new char array, only copies info
	fs->initstring(racepic, sizeof(racepic));

	// also, read all the branches.
	int i;
	for (i = 0; i < Nbranches; ++i)
	{
		branch[i] = new Dialo();
		branch[i]->mother = this;
		branch[i]->read(fs);
	}

}

void Dialo::write(FileStore *fs)
{
	fs->write(version);

	fs->write(state);

	fs->write(Nbranches);

	fs->write(Ntriggers);
	fs->write(trigger, Ntriggers);

	fs->writestring(T);
	fs->writestring(racepic);

	// also, write all the branches.
	int i;
	for (i = 0; i < Nbranches; ++i)
		branch[i]->write(fs);

}



void Dialo::enable()
{
	state = true;
}

void Dialo::disable()
{
	state = false;
}


void Dialo::calculate()
{
}

void Dialo::addbranch(Dialo *d)
{
	if (Nbranches >= maxbranches)
		return;

	++Nbranches;

	branch[Nbranches-1] = d;//new Dialo();
}


void Dialo::rembranch(int index)
{
	if (index < 0 || index > Nbranches-1)
		return;

	delete branch[index];

	--Nbranches;
	if (Nbranches < 0)
		Nbranches = 0;


	int i;
	for ( i = index; i < Nbranches; ++i )
	{
		branch[i] = branch[i+1];
	}

}



Dialo dialo;

void GameDialogue::init()
{
	int i;

	scare_mouse();

	GameBare::init();

	double H = 2000;
	size = Vector2(H, H);
	prepare();


	// find somewhere a list of all the available race-dialogue bitmaps

	FILE *f;
	char txt[128];

	f = fopen( "gamex/dialogue/racepics.txt", "rt" );

	Nracepiclist = 0;
	while (Nracepiclist < 128 && fgets(txt, 128, f))
	{
		int k = strlen(txt);
		if (txt[k-1] == '\n')
		{
			--k;
			txt[k] = 0;
		}

		racepiclist[Nracepiclist] = new char [k+1];
		strcpy(racepiclist[Nracepiclist], txt);
		++Nracepiclist;
	}


	for ( i = 0; i < maxbranches; ++i )
		Blist[i] = new char [32];

	// initialize dialogue data:
	// question

	firstdialo = new Dialo();

	fs = new FileStore("gamex/dialogue/test.dialogue2");
	firstdialo->read(fs);

	dialo = firstdialo;


	// initialize menu system.

	unscare_mouse();
	show_mouse(window->surface);


	// which font to use ... that depends on the screen resolution:
	i = 2;
	if (screen->w == 640)
		i = 3;
	if (screen->w == 800)
		i = 4;
	if (screen->w == 1024)
		i = 5;					// more pixels available for the same "real-life" size.

	FONT *usefont = videosystem.get_font(i);

	view->frame->prepare();





	R = new TWindow("gamex/interface/dialogeditor", 10, 10, view->frame->surface);


	


	int tcol = makecol(255,255,128);

	//strcpy(Qtext, "a\n b\n c\n d\n e\n f\n g\n h\n i");
	T = new TextEditBox(R, "text/", usefont, dialo->T, 256);
	T->set_textcolor(tcol);


	B = new TextList(R, "branches/", usefont);

	
	refresh = new Button(R, "refresh_", 0);

	Bplus = new Button(R, "addbranch_", 0);
//	Bmin = new Button(R, "Bmin", 0);

	bprev = new Button(R, "prevbranch_", 0);
//	bnext = new Button(R, "next", 0);

	dialostatus = new SwitchButton(R, "state_", 0);

	// add a text-button for the race pic, and a popup for the race list selection ?!

	raceselect = new TextButton(R, "race_", usefont);
	raceselect->set_text(dialo->racepic, makecol(255,255,0));
	raceselect->passive = false;

	popupraceselect = new PopupList(raceselect, "gamex/interface/dialogeditor/raceselect",
		"text/", -20, -20, usefont, 0);
	popupraceselect->tbl->set_optionlist(racepiclist, Nracepiclist, makecol(255,255,128));
	popupraceselect->hide();


//	winman = new WindowManager;
//	winman->add(R);
	R->add(popupraceselect);

	R->tree_doneinit();

	initeditor(dialo);

}


void GameDialogue::quit()
{
	// update the changes in memory
	fs->seek(0);		// you've to rewind the file first ...
	dialo->write(fs);

	delete fs;

	int i;
	for ( i = 0; i < maxbranches; ++i )
		delete Blist[i];

	for ( i = 0; i < Nracepiclist; ++i)
	{
		delete racepiclist[i];
	}

	scare_mouse();
	set_mouse_sprite(0);	// the default mouse sprite.
	show_mouse(screen);	// ????????
	//show_mouse();
	GameBare::quit();

}


void GameDialogue::initeditor(Dialo *dialo)
{
	Dialo *d;

	// find its branches :
	if (dialo->Nbranches > 0)
	{
		
		int i;
		for ( i = 0; i < dialo->Nbranches; ++i )
		{
			//d.read(dialo.branch[i], &dstore);
			d = dialo->branch[i];
			
			strncpy(Blist[i], d->T, 31);
			Blist[i][31] = 0;
			
		}		
	} else 
		Blist[0][0] = 0;

	B->set_optionlist(Blist, dialo->Nbranches, makecol(255,255,255));


	dialostatus->state = dialo->state;

	T->text_reset(dialo->T);

	raceselect->set_text(dialo->racepic, makecol(255,255,0));
}


void GameDialogue::calculate()
{
	if (next)
		return;

	GameBare::calculate();


	FULL_REDRAW = true;
	R->tree_calculate();


	/*
	// well... save the dialogue when needed (not to disk, but in memory).

	if (refresh->flag.left_mouse_press)
	{
		//dialo.write();
		dialo.refresh(&dstore);


		initeditor(dialo);
	}
	*/


	// create a new branch ...

	if (Bplus->flag.left_mouse_press)
	{
		//dialo.write(&dstore);

		Dialo *d = new Dialo();

		// default values:
		strcpy(d->T, "new");
		d->mother = dialo;	// the new branch has the old dialog as its parent.
		d->Nbranches = 0;
		d->Ntriggers = 0;
		d->state = dialo->state;				// inherit info
		strcpy(d->racepic, dialo->racepic);	// inherit info
		// well, this creates a new dialogue thingy :)
		//d.write(&dstore);

		dialo->addbranch(d);

		// and then, switch to this new dialo !!
		dialo = d;

		initeditor(dialo);
	}


	// go back to the child's mother

	if (bprev->flag.left_mouse_press)
	{

		// load the data from the mother branch (if it's has a mother).
		if (dialo->mother)
		{
			//dialo->write(&dstore);

			//dialo->read(dialo.mother, &dstore);
	
			dialo = dialo->mother;
			initeditor(dialo);
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
			i = B->gety();//scroll.yselect;
			if (i >= 0 && i < dialo->Nbranches)	// shouldn't be necessary ?!
			{
				// write the current data.
				//dialo.write(&dstore);

				dialo = dialo->branch[i];

				//dialo.read(k, &dstore);

				initeditor(dialo);
			}
		}
	}
	


	if (dialostatus->flag.left_mouse_press)
	{
		dialo->state = dialostatus->state;
	}


	if (popupraceselect->returnvalueready)
	{
		int k = popupraceselect->getvalue();

		if (k >= 0 && k < Nracepiclist)
		{
			strcpy(dialo->racepic, racepiclist[k]);
			raceselect->set_text(dialo->racepic, makecol(255,255,128));
		}
	}


}


void GameDialogue::animate(Frame *frame)
{
	if (next)
		return;

	GameBare::animate(frame);


	//show_mouse(frame->surface);
	R->tree_setscreen(view->frame->surface);
	R->tree_animate();
	show_mouse(view->frame->surface);
	scare_mouse();
}



