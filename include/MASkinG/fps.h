////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#ifndef			MASKING_FPS_H
#define			MASKING_FPS_H

#include "label.h"

namespace MAS {
	
/**
A simple widget that measures and displays the current redraw frame rate. The frame rate is measured by 
calculating a running average of the frame rate. The widget listens to MSG_TICK and MSG_DRAW messages. 
The widget constantly requests to be redrawn which means it receives MSG_DRAW messages every drawing 
frame. The frequency of MSG_TICK message is constant and known. From the number of times the widget 
received a MSG_TICK and the number of times it received a MSG_DRAW the framerate can easily be calculated.
*/
class FPS : public Label {
	protected:
		/**
		Samples is an array of integer that holds the number of received MSG_DRAW messages for 
		every logic frame (i.e. for every MSG_TICK message), 
		*/
		int *samples;
		/**
		nSamples is the size of the samples array. 
		*/
		int nSamples;
		/**
		curSample is a pointer that 
		points to the last element of the samples array
		*/
		int curSample;
		/**
		frameCounter records the number of received 
		MSG_DRAW messages for the last frame.
		*/
		int frameCounter;
		/**
		framesPerSecond is the calculated avearage 
		framerate for the last second. 	
		*/
		int framesPerSecond;
	
	protected:
		// Message functions
		/**
		Resets the FPS counter when the dialog starts.
		*/
		void MsgStart();
		/**
		Records a MSG_TICK message and calculates the new framerate.
		*/
		void MsgTick();
		/**
		Initializes the text and sets the text background colour to the default skin background colour.
		*/
		void MsgInitSkin();
		/**
		Simply requests another redraw.
		*/
		void MsgIdle();
		/**
		Records a MSG_DRAW and renders the frame rate as text.
		*/
		void Draw(Bitmap &canvas);

	public:
		/** 
		default constructor.
		*/
		FPS();
		/** 
		destructor
		*/
		~FPS();

		/**
		Returns the current FPS (stands for 'Frames Per Second'). Actually the frame rate is a 
		running average over the last second.
		*/
		int Get();
		
};
}

#endif		//MASKING_FPS_H
