

// WARNING : routines in this file requires a byte alignment of 1 for the structs used.


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
REGISTER_FILE


#include "edit_trigger.h"


// trigger functions.

// gewoon als stomme test-functie:

struct trigger_func
{
	// the data block
	char *d;

	// by reference only ( no new mem is allocated for it? )
	void set_data(char *dnew);

	// should not be changed:

	virtual int basesize();
	virtual void make_datablock(char *argumentlist, char *d, int *N);

	// can (and should) be changed:

	virtual trigger_func *create() {return new trigger_func(); };
	virtual void argstr(char **arg, char **id = 0, char ***comments = 0);
	virtual void calculate();
};


void trigger_func::set_data(char *dnew)
{
	d = dnew;
}


void trigger_func::argstr(char **arg, char **id, char ***comments)
{
	// this string matches the contents of "args"
	// s = char*, a string (4 (pointer) + N (characters))
	// d = double (8)
	// i = integer (4)

	if (arg) *arg = "";
	if (id) *id = "basetype";
	if (comments) *comments = (0);	// should be null-terminated ?!
}

// argumentlist is separated by values 255 (-1).
// all arguments are in string style, to be interpreted by the trigger structure.
int trigger_func::basesize()
{
	char *a;
	argstr(&a);

	int i, L;
	
	L = 0;
	i = 0;

	while (a[i] != 0)
	{
		switch(a[i])
		{
		case 's':
			L += sizeof(char*);	// pointer size
			break;
		case 'i':
			L += sizeof(int);	// integer size
			break;
		case 'd':
			L += sizeof(double);	// double size
			break;
		default:
			tw_error("unknown trigger argstr");
		}

		++i;
	}

	return L;
}

// returns d and N: the datablock and its size (create an exterior data block...).
void trigger_func::make_datablock(char *argumentlist, char *dext, int *N)
{
	int L, Lblock;

	// the "internal" data block [d] should point at the "external" one [dext].
	set_data(dext);

	Lblock = basesize();

	char *a;//, *b, **c;
	argstr(&a);//, &b, &c);
	
	L = 0;
	
	int argpos = 0;
	int i = 0;

	while (a[i] != 0)
	{
		int k1, k2;
		k1 = argpos;	// first char
		while (argumentlist[argpos] != -1 && argumentlist[argpos] != 0)
		{
			++argpos;
		}

		k2 = argpos;	// separator
		++argpos;	// first char of the new argument

		char txt[512];
		int n;

		n = k2 - k1;
		if (n > 512)
			n = 512;

		strncpy(txt, &argumentlist[k1], n);
		txt[n] = 0;
		// now, txt is the textstring for the i-th argument
		// with strlen == n.

		switch(a[i])
		{
		case 's':

			// add a string (at the end of the block...)
			// first, set the pointer to that block
			(char*&)d[L] = &d[Lblock];
			L += sizeof(char*);	// pointer size

			// then, add the string at the end of that block ...
			strcpy(&d[Lblock], txt);	// this also adds a ending 0.
			Lblock += n+1;	//+1 because the 0 counts as 1 extra byte

			break;
		case 'i':

			(int&)d[L] = atoi(txt);
			L += sizeof(int);	// integer size

			break;
		case 'd':
			{
			//double &doh
			//double doh = atof(txt);
			//(double&) d[L] = doh;
			//char *txt = (char*) &doh;
			//double &doh2 = (double&) d[L];

			(double&) d[L] = atof(txt);
			L += sizeof(double);	// double size
			break;
			}
		default:
			tw_error("unknown trigger argstr");
		}

		++i;
	}

	// also, add the physical address of the block (provides a reference!).
	(char*&) d[Lblock] = &d[0];

	Lblock += sizeof(char*);

	*N = Lblock;
}

void trigger_func::calculate()
{
}




// All the triggers in a list ...

const int maxtriggers = 512;

struct TriggerList
{
	int N;

	TriggerList();
	~TriggerList();

	trigger_func *trigger[maxtriggers];

	void add(trigger_func *newtrigger);

	trigger_func *create(char *id);
};

TriggerList triggerlist;





TriggerList::TriggerList()
{
	N = 0;
}

TriggerList::~TriggerList()
{
	int i;
	for ( i = 0; i < N; ++i )
		delete trigger[i];
}

void TriggerList::add(trigger_func *newtrigger)
{
	trigger[N] = newtrigger;
	++N;
}

trigger_func *TriggerList::create(char *id)
{
	// create the trigger with this id.
	int i;
	for ( i = 0; i < N; ++i )
	{
		char *triggerid;

		trigger[i]->argstr(0, &triggerid, 0);

		if (strcmp(id, triggerid) == 0)
			return trigger[i]->create();
	}

	// if the ID wasn't found:
	return 0;
}



// ----------------- a test-trigger ---------------
	
struct trigger_addstar : public trigger_func
{
	
	struct args
	{
		char *s;
		double d;
		int i;
	} *arg;

	virtual trigger_func *create() { return new trigger_addstar(); };
	virtual void argstr(char **arg, char **id = 0, char ***comments = 0);
	virtual void calculate();

};


void trigger_addstar::argstr(char **arg, char **id, char ***comments)
{
	if (arg) *arg = "sdi";
	if (id) *id = "addstar";
	if (comments) *comments = ("string","double","integer",0);	// should be null-terminated ?!
}


void trigger_addstar::calculate()
{
	// WARNING : this requires a byte alignment of 1 for structs.
	arg = (args*) d;

//	/* only for testing:
	clear_to_color(screen, makecol(0,0,0));

	char txt[512];
	sprintf(txt, "[%s] [%f] [%i]", arg->s, arg->d, arg->i );
	textout(screen, videosystem.get_font(4), txt, 100, 100, makecol(255,255,255));

	readkey();
//	*/
}


// trigger list
// ... declared as a global, for testing only ...
trigger_func *trigger[64];




void addchar(char *txt, char c)
{
	int L;
	L = strlen(txt);
	txt[L] = c;
	txt[L+1] = 0;
}

void GameTriggerEdit::init()
{
	int i;

	scare_mouse();

	GameBare::init();

	double H = 2000;
	size = Vector2(H, H);
	prepare();



	dstore.fread("gamex/triggers/test.triggers", 1000000);	// 1 MB min size.

	// the version
	// (can be used for rewriting stuff if changes are made).
	int currentversion = 1;
	if (currentversion != dstore.getversion())
	{
		// you should rewrite it !!
	}
	// update the version !?
	dstore.setversion(currentversion);





	// initialize menu system.

	unscare_mouse();
	show_mouse(window->surface);


	// which font to use ... that depends on the screen resolution:
	i = 2;
	if (screen->w == 640)
		i = 2;
	if (screen->w == 800)
		i = 3;
	if (screen->w == 1024)
		i = 4;					// more pixels available for the same "real-life" size.

	FONT *usefont = videosystem.get_font(i);

	view->frame->prepare();




//	AreaReserve *R;

//	R = new AreaReserve("gamex/interface/dialogeditor", 10, 10, view->frame->surface);


	// initialize the trigger list:
	triggerlist.add(new trigger_addstar());
	

	char *txt;

	txt = new char [512];

	strcpy(txt, "hi there !!");
//	txt[strlen(txt)] = -1;
//	txt[strlen(txt)+1] = 0;

	addchar(txt, -1);
	strcat(txt, "12.34");
//	txt[strlen(txt)] = -1;
//	txt[strlen(txt)+1] = 0;

	addchar(txt, -1);
	strcat(txt, "12345");
//	txt[strlen(txt)] = 0;

	//trigger_addstar ta;
	trigger[0] = triggerlist.create("addstar"); //ta.create();
	if (!trigger[0])
	{
		tw_error("Trigger could not be created");
	}

	char d[512];	// should be big enough to contain all the required data for that trigger.
	int N;

	trigger[0]->make_datablock(txt, d, &N);

	trigger[0]->calculate();


	winman = new WindowManager;
//	winman->add(R);

}


void GameTriggerEdit::quit()
{
	
	delete trigger[0];

	scare_mouse();
	set_mouse_sprite(0);	// the default mouse sprite.
	show_mouse(screen);	// ????????
	//show_mouse();
	GameBare::quit();

	// write changes to disk
	dstore.fwrite();
}



void GameTriggerEdit::calculate()
{
	if (next)
		return;

	GameBare::calculate();


	FULL_REDRAW = true;
	winman->calculate();

}


void GameTriggerEdit::animate(Frame *frame)
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



