
#ifndef __GAMEX_EDIT_DIALOGUE__
#define __GAMEX_EDIT_DIALOGUE__


#include "../gameproject.h"

#include "disk_stuff.h"

#include "../../twgui/twgui.h"
#include "../../twgui/twmenuexamples.h"

const int Ntext = 2048;
const int maxbranches = 16;
const int maxdialotriggers = 16;

struct Dialo
{
	int version;	// can be used to rewrite stuff (if ever needed)

	// for "refresh"
//	int filepos;
//	char *filename;
//	void refresh();

	char racepic[128];

	void read(FileStore *fs);
	void write(FileStore *fs);


	void enable();
	void disable();
	void calculate();

	void addbranch(Dialo *d);
	void rembranch(int index);
	//void copy(Dialo d);		// better use dialo.read.

	Dialo();
	~Dialo();

	char T[Ntext];	// text


	// text can also activate triggers ... but that's not supported yet ...
	int Ntriggers;
	int trigger[maxdialotriggers];


	int state;			// it may change state depending on whether it's used.

	int Nbranches;		// child dialogue items
	Dialo *branch[maxbranches];
	Dialo *mother;			// the mother
};



const int maxracepiclist = 1024;

class GameDialogue : public GameBare
{
	FileStore *fs;

	Dialo *dialo, *firstdialo;
	char *Blist[maxbranches];

	virtual void init();
	virtual void quit();
	//virtual bool handle_key(int k);
	virtual void init_menu();

	virtual void calculate();
	virtual void animate(Frame *frame);

	void initeditor(Dialo *dialo);


	int Nracepiclist;
	char *racepiclist[maxracepiclist];
	

	TextEditBox *Tedit;
	TextList	*Tlist;

	Button *refresh, *Bplus, *bprev;

	SwitchButton *dialostatus;

	TextButton *raceselect;
	PopupList *popupraceselect;
};


#endif


