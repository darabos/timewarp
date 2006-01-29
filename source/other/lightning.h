
#ifndef  __LIGHTNING_
#define __LIGHTNING_

#include "../ship.h"
#include "../scp.h"
#include "../util/aastr.h"
#include "../melee/mview.h"
#include "../frame.h"

#include <stdlib.h>


class Lightning
{
	//void calc_lightning(BITMAP *shpbmp);
	Vector2 lightningrelpos;
	double maxsparktime, Rmax;
	BITMAP *lightningbmp, *shpbmp;
	//Vector2 sparkpos;
	int  sparktime;

public:
	~Lightning();

	void init(BITMAP *ashpbmp, Vector2 alightningrelpos, int amaxsparktime, int aRmax);

	void update(double amount);
	void reset();
	void locate(Vector2 newpos);

	bool visible() const;

	void draw(Frame *space, Vector2 plot_pos, Vector2 plot_size);
};


#endif
