
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "melee.h"
REGISTER_FILE


#include "edit_dialogue.h"




static int current_dialog_version = 0;

// create a dialogue structure

Dialo::Dialo()
{
	Nbranches = 0;
	Ntriggers = 0;
	mother = 0;
	id[0] = 0;
}

Dialo::~Dialo()
{
	// if you delete this thing, you also need to delete all the nodes following it.
	discard_branches();
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

	// depending on the version, you can tweak the stuff below, in case you're
	// dealing with an important old dialog ...
	if (version != current_dialog_version)
	{
		tw_error("version conversion is not yet supported");
	}

	fs->read(&state);

	fs->read(&Nbranches);

	fs->read(&Ntriggers);
	fs->read(trigger, Ntriggers);

	fs->initstring(T, sizeof(T));	// does _not_ create a new char array, only copies info
	fs->initstring(racepic, sizeof(racepic));

	fs->initstring(id, sizeof(id));

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
	// write the new version.
	version = current_dialog_version;
	fs->write(version);

	fs->write(state);

	fs->write(Nbranches);

	fs->write(Ntriggers);
	fs->write(trigger, Ntriggers);

	fs->writestring(T);
	fs->writestring(racepic);

	fs->writestring(id);

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


// remove a branch, and all the branches following it...
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


// discards all subnodes.
void Dialo::discard_branches()
{
	int i;
	for (i = 0; i < Nbranches; ++i)
	{
		delete branch[i];
	}
}


//Dialo dialo;


void GameDialogue::init_menu()
{
	T = new TWindow("gamex/interface/dialogeditor", 0, 0, game_screen);
	maparea = 0;
}


void GameDialogue::init_dialog(char *fname)
{	
	if (fs)
		delete fs;

	fs = new FileStore(fname);

	if (firstdialo)
		delete firstdialo;

	firstdialo = new Dialo();

	firstdialo->read(fs);

	dialo = firstdialo;

	// initialize the interface so that you can edit and navigate the dialog tree
	initeditor(dialo);
}


void GameDialogue::save_dialog()
{
	fs->seek(0);		// you've to rewind the file first ...
	firstdialo->write(fs);
}


void GameDialogue::init()
{
	fs = 0;
	dialo = 0;
	firstdialo = 0;

	int i;

	if (!window) {
		window = new VideoWindow();
		window->preinit();
	}
	change_view("Hero"); 

	wininfo.init( Vector2(800,800), 800.0, view->frame );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.

	scare_mouse();

	GameBare::init();

	size = Vector2(100,100);
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


	


	int tcol = makecol(255,255,128);

	Tedit = new TextEditBox(T, "text/", usefont, 0, 0);
	Tedit->set_textcolor(tcol);


	Tlist = new TextList(T, "branches/", usefont);

	
	bload = new Button(T, "load_", 0);

	bsave = new Button(T, "save_", 0);

	Bplus = new Button(T, "addbranch_", 0);

	bprev = new Button(T, "prevbranch_", 0);

	dialostatus = new SwitchButton(T, "state_", 0);

	// add a text-button for the race pic, and a popup for the race list selection ?!

	raceselect = new TextButton(T, "race_", usefont);
	raceselect->passive = false;

	nodeid = new TextEditBox(T, "nodeid_", usefont, 0, 0);
	nodeid->set_textcolor(tcol);

	popupraceselect = new PopupList(raceselect, "gamex/interface/dialogeditor/raceselect",
		"text/", -20, -20, usefont, 0);
	popupraceselect->tbl->set_optionlist(racepiclist, Nracepiclist, makecol(255,255,128));
	popupraceselect->hide();

	fb = new FileBrowser(bload, "gamex/interface/filebrowser", 0, 0, usefont);
	fb->tbl->text_color = makecol(255,255,0);
	fb->set_dir("gamex/gamedata/races");
	fb->set_ext("dialog");


	T->add(popupraceselect);
	T->add(fb);

	T->tree_doneinit();

	
	// initialize dialogue data

	char fname[128];
	strcpy(fname, "gamex/gamedata/races/");
	strcat(fname, "earthling");		// the race name
	strcat(fname, "/");
	strcat(fname, "test.dialog");	// the dialog name

	init_dialog(fname);


	// no need for tic-info.
	ti = false;
}


void GameDialogue::quit()
{
	// disable auto-save ? Cause you got a save-button for that...
//	save_dialog();

	if (fs)
		delete fs;

	if (firstdialo)
		delete firstdialo;

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
			d = dialo->branch[i];
			
			strncpy(Blist[i], d->T, 31);
			Blist[i][31] = 0;
			
		}		
	} else 
		Blist[0][0] = 0;

	Tlist->set_optionlist(Blist, dialo->Nbranches, makecol(255,255,255));


	dialostatus->state = dialo->state;

	Tedit->text_reset(dialo->T, Ntext);

	raceselect->set_text(dialo->racepic, makecol(255,255,0));

	nodeid->text_reset(dialo->id, sizeof(dialo->id));
}


void GameDialogue::calculate()
{
	if (next)
		return;

	wininfo.center(0.5*size);

	GameBare::calculate();


	// create a new branch ...

	if (Bplus->flag.left_mouse_press)
	{

		Dialo *d = new Dialo();

		// default values:
		strcpy(d->T, "new");
		d->mother = dialo;	// the new branch has the old dialog as its parent.
		d->Nbranches = 0;
		d->Ntriggers = 0;
		d->state = dialo->state;				// inherit info
		strcpy(d->racepic, dialo->racepic);	// inherit info
		// well, this creates a new dialogue thingy :)

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
			dialo = dialo->mother;
			initeditor(dialo);
		}
	}


	
	// go forth to the currently selected branch

	if (Tlist->flag.left_mouse_press)
	{
		// load the data from the child branch.
		if (dialo->Nbranches > 0)
		{
			int i;
			i = Tlist->gety();//scroll.yselect;
			if (i >= 0 && i < dialo->Nbranches)	// shouldn't be necessary ?!
			{
				dialo = dialo->branch[i];
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


	// check if a new file is selected
	// then you load a existing file ...
	if (fb->ready())
	{
		init_dialog(fb->fname);
	}

	if (bsave->flag.left_mouse_press)
	{
		save_dialog();
	}

}


void GameDialogue::animate(Frame *frame)
{
	if (next)
		return;

	GameBare::animate(frame);
}



