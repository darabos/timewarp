
#ifndef __TWMENUEXAMPLES__
#define __TWMENUEXAMPLES__

#include "twpopup.h"
#include "twgui.h"


class PopupTextInfo : public PopupT
{
public:
	TextInfoArea	*tia;
	Button			*closebutton;

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
// the button images should be present on it as well)
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
	Button			*oncerandom, *alwaysrandom;

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
	Button		*ok;
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
	Button		*yes, *no;
public:
	PopupYN(char *ident, int xcenter, int ycenter,
						//char *datafilename,
						BITMAP *outputscreen,
							bool inherited = false);
	virtual ~PopupYN();
	virtual void check_end();

};





#endif
