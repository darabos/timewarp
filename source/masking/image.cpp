////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/image.h"

MAS::Image::Image()
	:Widget(),
	allocated(false),
	masked(false),
	bg(-1)
{
	SetFlag(D_AUTOSIZE);
}


MAS::Image::~Image() {
	bmp.Destroy();
}


void MAS::Image::SetMasked(bool masked) {
	this->masked = masked;
	Redraw();
}


void MAS::Image::SetBackgroundColor(const Color& bg) {
	this->bg = bg;
	Redraw();
}


void MAS::Image::Setup(int x, int y, int w, int h, int key, int flags, int i, bool masked, const Color& bg) {
	Widget::Setup(x, y, w, h, key, flags);
	SetBitmap(i);
	this->masked = masked;
	this->bg = bg;
}


void MAS::Image::Setup(int x, int y, int w, int h, int key, int flags, const char *file, bool masked, const Color& bg) {
	Widget::Setup(x, y, w, h, key, flags);
	SetBitmap(file);
	this->masked = masked;
	this->bg = bg;
}


void MAS::Image::Setup(int x, int y, int w, int h, int key, int flags, const Bitmap& bmp, bool masked, const Color& bg) {
	Widget::Setup(x, y, w, h, key, flags);
	SetBitmap(bmp);
	this->masked = masked;
	this->bg = bg;
}


void MAS::Image::Draw(Bitmap &canvas) {
	if (!bmp && bitmap < 0) {
		return;
	}

	Bitmap bmp2 = bmp ? bmp : skin->GetBitmap(bitmap);
	if (!bmp2) {
		return;
	}

	int xx = MAX((w() - bmp2.w())/2, 0);
	int yy = MAX((h() - bmp2.h())/2, 0);
	
	if (masked) {
		if (bg) canvas.Clear(bg);
		bmp2.MaskedBlit(canvas, 0, 0, xx, yy, w(), h());
	}
	else {
		bmp2.Blit(canvas, 0, 0, xx, yy, w(), h());
	}
}


void MAS::Image::SetBitmap(const Bitmap& bmp) {
	this->bmp.Set(bmp, true);
	UpdateSize();
}


void MAS::Image::SetBitmap(const char *filename) {
	bmp.Load(filename);
	UpdateSize();
}


void MAS::Image::SetBitmap(int i) {
	bmp.Destroy();
	Widget::SetBitmap(i);
	UpdateSize();
}


MAS::Bitmap &MAS::Image::GetBitmap() {
	return bmp;
}


void MAS::Image::UpdateSize() {
	if ((!bmp && bitmap < 0) || !TestFlag(D_AUTOSIZE)) {
		return;
	}

	Bitmap bmp = this->bmp ? this->bmp : skin->GetBitmap(bitmap);
	
	if (bmp) {
		Resize(bmp.size());
	}
}
