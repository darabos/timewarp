
#ifndef __GAMEX_EDIT_DIALOGUE__
#define __GAMEX_EDIT_DIALOGUE__


#include "../gameproject.h"

#include "disk_stuff.h"

#include "twgui/twgui.h"

const int Ntext = 2048;
const int maxbranches = 16;
const int maxdialotriggers = 16;

/** \brief The record of a dialogue-node. A dialogue consists of many of these
linked records.

*/

struct Dialo
{
	int version;	// can be used to rewrite stuff (if ever needed)

	// for "refresh"
//	int filepos;
//	char *filename;
//	void refresh();

	char racepic[128];

	void init_default();
	void read(FileStore *fs);
	void write(FileStore *fs);

	bool check_state();		// checks/changes state based on whether there are live branches or not
	int get_branch();		// retrieves the first "live" branch

	void enable();
	void disable();
	void calculate();

	void addbranch(Dialo *d);
	void rembranch(int index);
	//void copy(Dialo d);		// better use dialo.read.

	Dialo();
	~Dialo();

	char T[Ntext];	// text

	char id[64];


	// text can also activate triggers ... but that's not supported yet ...
	int Ntriggers;
	int trigger[maxdialotriggers];


	int state;			// it may change state depending on whether it's used.

	int Nbranches;		// child dialogue items
	Dialo *branch[maxbranches];
	Dialo *mother;			// the mother

	void discard_branches();

	Dialo *findnode(const char *id);	// finds the node with the specified id; returns 0 on failure.
	void save_state(FILE *f);
	void read_state(FILE *f);
};



const int maxracepiclist = 1024;

class GameDialogue : public GameBare
{
	FileStore *fs;

	Dialo *dialo, *firstdialo;
	char *Blist[maxbranches];

	FileBrowser *fb, *popupraceselect;

	virtual void init();
	virtual void quit();
	//virtual bool handle_key(int k);
	virtual void init_menu();

	virtual void calculate();
	virtual void animate(Frame *frame);

	void initeditor(Dialo *dialo);

	void init_dialog(char *fname);
	void save_dialog();

	//int Nracepiclist;
	//char *racepiclist[maxracepiclist];
	

	TextEditBox *Tedit, *nodeid;
	TextList	*Tlist;

	Button *bload, *bsave, *Bplus, *bprev;

	SwitchButton *dialostatus;

	TextButton *raceselect;
	//PopupList *popupraceselect;
};


#endif  // __GAMEX_EDIT_DIALOGUE__


