#ifndef __GAMEX_DIALOGUE__
#define __GAMEX_DIALOGUE__


#include "gameproject.h"
#include "edit/edit_dialogue.h"
#include "gamedata.h"

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
	char show_line[2048];
	char *showline(char *text, int num);
	int Nlines(char *text);

	void preptext(char *t);

	bool eol(char c);

public:
	GameAliendialog();

	char *race_id;
	void set_colony(RaceColony *rc);
	void set_dialog(char *fname);
};


#endif // __GAMEX_DIALOGUE__


