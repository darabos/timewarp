// ************************************************************************** //
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
// ************************************************************************** //

#ifndef			MASKING_PANEL_H
#define			MASKING_PANEL_H

#include "widget.h"
#include "dialog.h"

namespace MAS {
/**
	The basic panel widget.
	This is an extremely simple widgets that just draws a panel. Good for
	vsiually separating groups of widgets. You can change the appearance
	of the panel with the SetBitmap() function to select different kinds
	of panels.
*/
class Panel : public Widget {
	protected:
		/**
			Draws a panel by tiling its bitmap over the entire canvas.
		*/
		void Draw(Bitmap &canvas);
		/**
			Initializes the panel with the selcted panel bitmap from the skin.
			The default for this panel is the "raised panel" bitmap.
		*/
		void MsgInitSkin();
};

/**
	The raised panel widget.
*/
class PanelRaised : public Panel {
	protected:
		/**
			Initializes the panel with the "raised panel" bitmap from the skin.
		*/
		void MsgInitSkin();
};

/**
	The sunken panel widget.
*/
class PanelSunken : public Panel {
	protected:
		/**
			Initializes the panel with the "sunken panel" bitmap from the skin.
		*/
		void MsgInitSkin();
};

/**
	The ridge panel widget.
*/
class PanelRidge : public Panel {
	protected:
		/**
			Initializes the panel with the "ridge panel" bitmap from the skin.
		*/
		void MsgInitSkin();
};

/**
	The groove panel widget.
*/
class PanelGroove : public Panel {
	protected:
		/**
			Initializes the panel with the "groove panel" bitmap from the skin.
		*/
		void MsgInitSkin();
};

/**
	The shadow box widget.
*/
class BoxShadow : public Panel {
	protected:
		/**
			Initializes the panel with the "shadow box" bitmap from the skin.
		*/
		void MsgInitSkin();
};
}

#endif		//MASKING_PANEL_H
