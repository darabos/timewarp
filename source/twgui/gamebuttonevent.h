
#ifndef __GAMEBUTTONEVENT_H__
#define __GAMEBUTTONEVENT_H__


template <class G>
class BEvent : public ButtonEvent
{
public:
	typedef void (G::* ftest) ();

	G *g;
	ftest mainpress, mainhold, specialpress, specialhold;

	BEvent(G *gg, ftest m1, ftest m2);
	BEvent(G *gg, ftest m1, ftest m2, ftest s1, ftest s2);
	
	
	virtual void handle_main_press();
	virtual void handle_main_hold();
	virtual void handle_special_press();
	virtual void handle_special_hold();
};

template <class G>
BEvent<G>::BEvent<G>(G *gg, ftest m1, ftest m2)
{
	g = gg;
	mainpress = m1;
	mainhold = m2;
}

template <class G>
BEvent<G>::BEvent<G>(G *gg, ftest m1, ftest m2, ftest s1, ftest s2)
{
	g = gg;
	mainpress = m1;
	mainhold = m2;
	specialpress = s1;
	specialhold = s2;
}


template <class G>
void BEvent<G>::handle_main_press()
{
	if (mainpress)
		(g->*mainpress) ();
}


template <class G>
void BEvent<G>::handle_main_hold()
{
	if (mainpress)
		(g->*mainhold) ();
}


template <class G>
void BEvent<G>::handle_special_press()
{
	if (specialpress)
		(g->*specialpress) ();
}


template <class G>
void BEvent<G>::handle_special_hold()
{
	if (specialpress)
		(g->*specialhold) ();
}



#endif // __GAMEBUTTONEVENT_H__
