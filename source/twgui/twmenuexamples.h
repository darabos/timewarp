/* $Id$ */ 
/*
Placed in public domain by Rob Devilee, 2004. Share and enjoy!
*/

#ifndef __TWMENUEXAMPLES_H__
#define __TWMENUEXAMPLES_H__

#include "twpopup.h"
#include "twgui.h"


class PopupTextInfo : public PopupT
{
public:
	TextInfoArea	*tia;
	::Button			*closebutton;

	PopupTextInfo(EmptyButton *creator, char *ident, char *id2, int axshift, int ayshift,
					FONT *afont, char *atext, int aNchar);
	virtual ~PopupTextInfo();
	virtual void check_end();
};



// the info screen, can be toggled on/off, by pressing the trigger-button.
// (uhm, well, that didn't work easily, so now it's simply turned on all the time).
class PopupTextInfo_toggle : public PopupTextInfo
{
public:

	PopupTextInfo_toggle(EmptyButton *creator, char *ident, char *id2, int axshift, int ayshift,
		FONT *afont, char *atext, int aNchar);

//	~PopupTextInfo();

	virtual void calculate();
//	virtual void check_end();

};




// A popup-list consists of several components, which you've to initialize:
// (most of it is automatically initialized from the shape of the menu screen-
// the ::Button images should be present on it as well)
class PopupList : public PopupT
{
public:

	int xshift, yshift;

//	EmptyButton		*trigger;	// this controls the on/off of the menu

//	Button			*left, *right, *up, *down;
//	scrollpos_str	*scroll_control;
	TextList	*tbl;
//	ScrollBar		*scrollvert, *scrollhor;

//	char			**optionslist;

	// just a "list" item ... not really a popup thing
	PopupList(BITMAP *outputscreen, char *ident, char *id2, int axshift, int ayshift,
				FONT *afont, char **aaoptionslist);
	
	// origin relative to the creators' position
	PopupList(EmptyButton *creator, char *ident, char *id2, int axshift, int ayshift,
				FONT *afont, char **aaoptionslist);
	virtual ~PopupList();

	virtual void check_end();
	//virtual void calculate();
	//virtual void handle_focus_loss();

};









class PopupFleetSelection : public Popup
{
public:
	MatrixIcons		*icons;
	TextButton		*info;
	::Button			*oncerandom, *alwaysrandom;

	PopupFleetSelection(EmptyButton *creator, char *ident, int axshift, int ayshift, char *datafilename,
				BITMAP **alistIcon, double ascale, FONT *afont );

	PopupFleetSelection(char *ident, int axshift, int ayshift,
								char *datafilename, BITMAP *outputscreen,
								BITMAP **alistIcon, double ascale, FONT *afont );

	virtual ~PopupFleetSelection();

	//virtual void close(int areturnstatus);
	virtual void check_end();

	virtual void newscan(BITMAP **alistIcon, double ascale, char *txt);

	//virtual void init_components();
	
};








class PopupOk : public Popup
{
	::Button		*ok;
public:
	PopupOk(char *ident, int xcenter, int ycenter,
						//char *datafilename,
						BITMAP *outputscreen,
							bool inherited = false);
	virtual ~PopupOk();
	virtual void check_end();
};


class PopupYN : public Popup
{
	::Button		*yes, *no;
public:
	PopupYN(char *ident, int xcenter, int ycenter,
						//char *datafilename,
						BITMAP *outputscreen,
							bool inherited = false);
	virtual ~PopupYN();
	virtual void check_end();

};





class FileBrowser : public PopupList
{
public:
	bool selection;
	::Button *downdir;

	char dir[512];		// max filename length
	char fname[512];
	int fattr[2048];	// can handle max 2048 files.

	char required_ext[64];
	void set_ext(char *ext);

	FileBrowser(EmptyButton *creator, char *ident, int axshift, int ayshift, FONT *afont);
	virtual ~FileBrowser();

	virtual void calculate();

	void reset_dirlist();

	virtual void check_end();

	void set_dir(char *newdir);
};





enum valuetypes {vtype_int=0, vtype_float=1};

class ValueStr
{
public:
	char format[16];
	char descr[64];
	double min, max, value;
	valuetypes type;	// int or float

	void set(valuetypes atype, char *adescr, double amin, double amax);
	double getval();
};


class ValueEdit : public EmptyButton
{
public:
	int Nmax, isel;
	bool do_init;
	ValueStr **values;

	double barpos;
	char	edit_text[512];

	TextButton *info;
	TextEditBox *edit;
	ScrollControl scroll;

	ValueEdit(TWindow *menu, char *identbranch, FONT *afont, int Nmax);
	virtual ~ValueEdit();

	void edit_update();

	virtual void calculate();
};


#endif // __TWMENUEXAMPLES_H__

