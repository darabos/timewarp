// ************************************************************************** //
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
// ************************************************************************** //

#ifndef			MASKING_TABPANEL_H
#define			MASKING_TABPANEL_H

#include "dialog.h"
#include "panel.h"
#include "radiobutton.h"
#include <vector>

namespace MAS {
class TabButton : public RadioButton {
	protected:
		void MsgInitSkin();
		void Draw(Bitmap &canvas);
		bool MsgWantTab();
};

/**
	A compound widget for switching between several widgets (or dialogs).
	The currently active widget is displayed in the main panel area.
	Widgets can be selected by clicking buttons on top of the main panel.
*/
class TabPanel : public Dialog {
	protected:
		struct TabItem {
			Widget *widget;
			TabButton *button;
		};
		
		Panel box;
		std::vector<TabItem> tabArray;
		Widget *selectedWidget;
	
		void MsgInitSkin();
		
		/**
			Updates the size and position of all the widgets in the panel.
			This function places all the buttons along the top edge of the panel
			and resizes all the attached dialogs to fit inside the panel.
		*/
		void UpdateSize();
	
	public:
		TabPanel();
		~TabPanel();
	
		/**
			Attaches an exisiting widget (or dialog) to the tab panel.
			Widgets are always added to the end of the list. Pass the text
			you want to appear on the button that will select this widget.
		*/
		void Attach(Widget *w, const char *title);
	
		/**
			Removes the passed widget from the panel.
		*/
		void Detach(Widget *w);
	
		void HandleEvent(Widget &obj, int msg, int arg1=0, int arg2=0);
};
}

#endif		//MASKING_TABPANEL_H
