
#include "lightning.h"




Lightning::~Lightning()
{
	destroy_bitmap(lightningbmp);
}

bool Lightning::visible() const
{
	return (sparktime > 0);
}

void Lightning::update(double time)
{
	if (sparktime > 0)
		sparktime -= time;
	else
		sparktime = 0;
}

void Lightning::reset()
{
	sparktime = maxsparktime;
}

void Lightning::locate(Vector2 newpos)
{
	lightningrelpos = newpos + 0.5 * Vector2(lightningbmp->w,lightningbmp->h);
}

void Lightning::init(BITMAP *ashpbmp, Vector2 alightningrelpos, int amaxsparktime, int aRmax)
{
	shpbmp = ashpbmp;

	maxsparktime = amaxsparktime;
	reset();
	
	Rmax = aRmax;
	lightningrelpos = alightningrelpos;

	int bpp = bitmap_color_depth(shpbmp);
	lightningbmp = create_bitmap_ex(bpp, shpbmp->w, shpbmp->h);
	clear_to_color(lightningbmp, makeacol(0,0,0,255));
}


void Lightning::draw(Frame *space, Vector2 plot_pos, Vector2 plot_size)
{
	 

	Vector2 P, S;

	//return;
	//clear_to_color(lightningbmp, makeacol(0,0,0,255));
	//circlefill(lightningbmp, P.x, P.y, 10, makeacol(255,0,0,200));

	// first, let the image grow fainter
	BITMAP *b;
	b = lightningbmp;
	int ix, iy;
	int iw, ih;
	iw = b->w;
	ih = b->h;

	P = lightningrelpos;

	// assume it's a 32 bit image ...
	unsigned char *a;

	for ( iy = 0; iy < ih; ++iy )
	{
		a = (unsigned char*) b->line[iy];
		a += 0;		// red = the 3rd color (in my case).
		
		for ( ix = 0; ix < iw; ++ix )
		{
			// reduce colors
			int i;
			for ( i = 0; i < 3; ++i )
			{
				if (*a > 5)
					(*a) -= 5;
				else
					(*a) = 0;

				++a;
			}

			++a;
		}
	}

	if (sparktime > 0)
	{
		// create some kind of lightning now ? How ?
		int i, N;
		
		N = 5 + rand() % 5;	//graphics

		double ang, R;
		ang = (rand() % 360) * PI/180;	//graphics
		
		for ( i = 0; i < N; ++i )
		{
			double dx, dy;
			//dx = (random(double(iw)) - 0.5*iw) / N;
			//dy = (random(double(ih)) - 0.5*ih) / N;

			ang += (rand() % 180 - 90) * PI/180;	//graphics
			R = (rand() % int(Rmax+1)) / N;	//graphics
			dx = R * cos(ang);
			dy = R * sin(ang);
			
			
			int j, M;
			M = 10;
			for ( j = 0; j < M; ++j )
			{
				P.x += dx / M;
				P.y += dy / M;
				
				int col;
				int re, gr, bl;
				
				col = getpixel(shpbmp, iround(P.x), iround(P.y));
				
				re = getr(col);
				gr = getg(col);
				bl = getb(col);
				
				if ( !(re == 255 && gr == 0 && bl == 255))
				{
					int k;
					k = iround(128*(0.5 + 0.5*sparktime/maxsparktime));
					
				//	re = 0;
				//	bl = 0;
				//	if ( k > 96)
				//		re = k + 127 / (i+1);
				//	else
				//		bl = k + 127 / (i+1);

					double c, f;
					//f = (0.5 + (0.5*sparktime)/maxsparktime);
					f = sparktime / maxsparktime;
					c = 255;// / (i+1);
					re = iround(f * c);
					bl = iround((1-f) * c);
					
					putpixel(b, iround(P.x), iround(P.y), makeacol(re,0,bl,255));
				}
			}
			
		}
	}




	// target position
	ix = iround(plot_pos.x);
	iy = iround(plot_pos.y);
	// target size
	iw = iround(plot_size.x);
	ih = iround(plot_size.y);


	
	int aa;
	aa = aa_get_mode();
	aa_set_mode(aa | AA_ALPHA);
	aa_stretch_blit(lightningbmp, space->surface,
		0, 0,lightningbmp->w,lightningbmp->h,
		ix, iy, iw, ih);
	aa_set_mode(aa);
	
	space->add_box(ix, iy, iw, ih);


}

