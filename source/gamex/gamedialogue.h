#ifndef __GAMEX_DIALOGUE__
#define __GAMEX_DIALOGUE__


#include "gameproject.h"
#include "edit/edit_dialogue.h"

#include "edit/disk_stuff.h"

#include "../twgui/twgui.h"


class GameAliendialog : public GameBare
{
	FileStore *fs;

	Dialo *dialo, *firstdialo, *dialo_selected[32];

	char *Blist[maxbranches];
	int branchindex[maxbranches];
	int Nactivebranches;

	virtual void init();
	virtual void init_menu();
	virtual void quit();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	void initBlist(Dialo *dialo);


	//TWindow *R;

	TextInfoArea *A;
	TextList	*B;

	char racefile[128];
	

	int showline_num, showline_Nlines;
	char show_line[256];
	char *showline(char *text, int num);
	int Nlines(char *text);
};


#endif


