// ************************************************************************** //
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
// ************************************************************************** //

#ifndef			MASKING_GROUPBOX_H
#define			MASKING_GROUPBOX_H

#include "panel.h"
#include "label.h"
#include "dialog.h"

namespace MAS {
/**
	Similar to a Panel except that it can also have a title.
*/
class GroupBox : public Dialog {
	protected:
		/**
			The underlaying panel widget.
		*/
		Panel box;
		/**
			The underlaying title label.
		*/
		Label title;
	
	protected:
		/**
			Initialized the groupbox with the default bitmap, fonts and colours as defined by the skin.
		*/
		void MsgInitSkin();
		/**
			Updates the size of the box and the title.
			It resizes the panel to fit into the box and positions the title
			text according to the title alignment setting. The title can be
			aligned left or right or can be centered and can be placed at an
			arbitrary offset from either the left or right edge of the box or
			from the centre.
		*/
		void UpdateSize();

	public:
		GroupBox();

		/**
			 ets the title text of the groupbox.
		*/
		void SetTitle(const char *title);
};
}

#endif		//MASKING_GROUPBOX_H
