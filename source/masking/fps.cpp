////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/fps.h"
#include "../include/MASkinG/settings.h"


MAS::FPS::FPS()
	:MAS::Label(),
	samples(NULL),
	nSamples(0),
	curSample(0),
	frameCounter(0),
	framesPerSecond(0)
{
	ClearFlag(D_AUTOSIZE);
}


MAS::FPS::~FPS() {
	if (samples) {
		delete [] samples;
		samples = NULL;
	}
	nSamples = 0;
	frameCounter = 0;
}


void MAS::FPS::MsgStart() {
	if (samples) {
		delete [] samples;
		samples = NULL;
	}
	
	nSamples = MAS::Settings::logicFrameRate;
	samples = new int[nSamples];
	for (int i=0; i<nSamples; i++) {
		samples[i] = 1;
	}
	
	curSample = 0;
	frameCounter = 0;
	framesPerSecond = MAS::Settings::logicFrameRate;
	
	Label::MsgStart();
}


void MAS::FPS::MsgTick() {
	curSample++;
	curSample %= nSamples;
	framesPerSecond -= samples[curSample];
	framesPerSecond += frameCounter;
	samples[curSample] = frameCounter;
	frameCounter = 0;
	Redraw();
	
	Label::MsgTick();
}


void MAS::FPS::Draw(Bitmap &canvas) {
	++frameCounter;
	SetNumber(framesPerSecond);
	Label::Draw(canvas);
}


void MAS::FPS::MsgIdle() {
	Redraw();
	Widget::MsgIdle();
}


void MAS::FPS::MsgInitSkin() {
	SetNumber(80000);
	Label::MsgInitSkin();
	SetTextMode(skin->c_back);
}


int MAS::FPS::Get() {
	return framesPerSecond;
}
