

#include <allegro.h>
#include <stdio.h>
#include <string.h>


#include "../melee.h"
//#include "../../melee/mview.h"

REGISTER_FILE

#include "twbutton.h"
#include "twwindow.h"

#include "twmenuexamples.h"






// make use of this textinfo box, in some "controlled" menu space:

PopupTextInfo::PopupTextInfo(EmptyButton *creator, char *ident, char *id2, int axshift, int ayshift,
							 FONT *afont, char *atext, int aNchar)
:
PopupT(creator, ident, axshift, ayshift)
{
	// add a text list box
	tia = new TextInfoArea(this, id2, afont, atext, aNchar);//, &scroll);
	//tia->set_textinfo(atext, aNchar);

	char s[128];
	strcpy(s, id2);
	strcat(s, "close_");
	closebutton = new Button(this, s);

	exclusive = true;

}

PopupTextInfo::~PopupTextInfo()
{
	if (tia)
		delete tia;
}


void PopupTextInfo::check_end()
{
	if (closebutton->flag.left_mouse_press)
		close(-1);
}





PopupTextInfo_toggle::PopupTextInfo_toggle(EmptyButton *creator, char *ident, char *id2, int axshift, int ayshift,
	FONT *afont, char *atext, int aNchar)
:
PopupTextInfo(creator, ident, id2, axshift, ayshift, afont, atext, aNchar)
{
};



void PopupTextInfo_toggle::calculate()
{
	PopupTextInfo::calculate();
}

//void PopupTextInfo_toggle::check_end()
//{
//};






// This popup is invoked by a push-button somewhere. I'm not going to create some
// special push button type for that, I can just as well let this (new) class
// access the state of that general button type.


// (well, the following one isn't using a button, the one below is...)
PopupList::PopupList(BITMAP *outputscreen, char *ident, char *id2, int axshift, int ayshift,
				FONT *afont, char **aoptionslist)
:
PopupT(ident, axshift, ayshift, outputscreen)
{
	tbl = new TextList(this, id2, afont);
	tbl->set_optionlist(aoptionslist, makecol(0,0,0));

	exclusive = true;
}


PopupList::PopupList(EmptyButton *atrigger, char *ident, char *id2, int axshift, int ayshift,
						FONT *afont, char **aoptionslist)
:
PopupT(atrigger, ident, axshift, ayshift)
{
	//optionslist = aoptionslist;

	// add a text list box
	tbl = new TextList(this, id2, afont);
	tbl->set_optionlist(aoptionslist, makecol(0,0,0));

	exclusive = true;
}


PopupList::~PopupList()
{

	// delete the list
	if (tbl)
		delete tbl;
}


// this calls close with return value
void PopupList::check_end()
{
	PopupT::check_end();		// this is also a way to end (without choosing anything)

	if (tbl->selected)
	{
		tbl->selected = false;
		close(tbl->getk());
	}
}










PopupFleetSelection::PopupFleetSelection(char *ident, int axshift, int ayshift,
						char *datafilename, BITMAP *outputscreen,
						BITMAP **alistIcon, double ascale, FONT *afont )
:
Popup(ident, axshift, ayshift, outputscreen)
{

	icons = new MatrixIcons(this, "icon_", KEY_ENTER);

	// this auto-configures the data as well ...
	icons->set_iconinfo(alistIcon, ascale);

	info = new TextButton(this, "info_", afont);
	oncerandom = new Button(this, "random_", KEY_R);
	alwaysrandom = new Button(this, "alwaysrandom_", KEY_A);

}




PopupFleetSelection::~PopupFleetSelection()
{
	if (icons)
		delete icons;
	if (info)
		delete info;
	if (oncerandom)
		delete oncerandom;
	if (alwaysrandom)
		delete alwaysrandom;
}

// this calls close with return value
// (the general close is ok, it closes and hides the menu).
void PopupFleetSelection::check_end()
{
	if (icons->selected)
		close(icons->getk());
}



void PopupFleetSelection::newscan(BITMAP **alistIcon, double ascale, char *txt)
{
	Popup::newscan();
	icons->set_iconinfo(alistIcon, ascale);
	info->set_text(txt, makecol(255,255,0));
}




/*
// let the "main" routine determine whether to stop or not ;)
void PopupFleetSelection::check_end()
{
	if (icons->selected)
		close(icons->getk());
}
*/







// here, buttons are initialized
PopupYN::PopupYN(char *ident, int xcenter, int ycenter,
							BITMAP *outputscreen,
								bool inherited)
:
Popup(ident,
			xcenter, ycenter,
			outputscreen)
{
	yes = new Button(this, "yes_",KEY_Y);

	no = new Button(this, "no_", KEY_N);

	exclusive = true;
}

PopupYN::~PopupYN()
{
}

void PopupYN::check_end()
{
	if (yes->flag.left_mouse_press)
		close(1);
	if (no->flag.left_mouse_press)
		close(0);
}







// here, buttons are initialized
PopupOk::PopupOk(char *ident, int xcenter, int ycenter,
							BITMAP *outputscreen,
								bool inherited)
:
Popup(ident,
			xcenter, ycenter,
			outputscreen)
{
	ok = new Button(this, "ok_", KEY_ENTER);

	exclusive = true;
}

PopupOk::~PopupOk()
{
}

void PopupOk::check_end()
{
	if (ok->flag.left_mouse_press)
		close(1);
}




