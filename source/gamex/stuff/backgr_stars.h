/* $Id$ */ 
#ifndef __GAMEX_BACKGR_STARS__
#define __GAMEX_BACKGR_STARS__


// the star background ...

class StarBackgr : public SpaceLocation
{
	int Nstars;
	SpaceSprite	**starsprite;
	Vector2	*pos;
	double	*refdist;
public:
	void init(int N, Frame *frame);
	virtual void animate(Frame *space);
	virtual void calculate(); //advance the item frame_time milliseconds in time

	StarBackgr();
	~StarBackgr();
};



#endif // __GAMEX_BACKGR_STARS__
