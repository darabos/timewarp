// **********************************************************
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
//            TextArea written by Martijn "amarillion" van Iersel             //
// **********************************************************
#ifndef TEXTAREA_H
#define TEXTAREA_H


#ifdef ALLEGRO_MSVC
#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
#endif

#include "scrollbox.h"
#include <vector>
#include <string>


namespace MAS {
/**
    A multiline text viewing widget. This is derived from ScrollBox, so it will show scrollbars if
    there is too much text to fit inthe widget area. 
    
    note: in the future this will be made into a full multiline text editing widget.
*/
class TextArea : public ScrollBox {
	private:
		std::vector<std::string> lines;
		void CalculateRange();
	
	protected:
		void MsgInitSkin();
                virtual void DrawClientArea(Bitmap &dest, int xofst, int yofst);

	public:
/**
        Default constructor.
*/
		TextArea();
/**
        Load a textfile to display in this textarea.
*/
		void LoadLinesFromFile(char *filename);
/**
         Set the contents of this TextArea.
*/
		void SetText(const char *text);
/**
        Append a new line to this textarea.
*/
		void AddText(const char *text);
};
}

#endif	//TEXTAREA_H
