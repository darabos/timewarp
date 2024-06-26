/* $Id$ */ 
/* modified by orz for Star Control: TimeWarp 
aa_rotate_sprite rotated around a different point than rotate_sprite
and screwed up when the angle was a multiple of 90 degrees.  
Both were fixed, but the fix was really half-assed, so the problem 
might come up again.  */

/*
 * aarot.c --- anti-aliased rotation for Allegro
 *
 * This file is gift-ware.  This file is given to you freely
 * as a gift.  You may use, modify, redistribute, and generally hack
 * it about in any way you like, and you do not have to give anyone
 * anything in return.
 *
 * I do not accept any responsibility for any effects, adverse or
 * otherwise, that this code may have on just about anything that
 * you can think of.  Use it at your own risk.
 *
 * Copyright (C) 1998, 1999  Michael Bukin
 */

#include <allegro.h>

#include "aastr.h"
#include "aautil.h"

/*
 * Engine of anti-aliased rotation.
 */
void
_aa_rotate_bitmap (BITMAP *_src, BITMAP *_dst, int _x, int _y, long _angle,
		long _scalex, long _scaley, int _mode)
{
	int sw, sh, dw, dh;
	fixed fx0, fy0, fux, fuy, fvx, fvy;
	fixed fdw, fdh, fsinangle, fcosangle;
	struct {
		int dx, dy;
		int sx, sy;
	} point[4], *lpoint1, *lpoint2, *rpoint1, *rpoint2;
	int ledge[4], redge[4], lindex, rindex;
	int xbeg, xend, ybeg, yend;
	int sx, sy, dx, dy, dsx, dsy;
	int ldx, lsx, lsy, *lsc;
	int rdx, rsx, rsy, *rsc;
	int ldxinc, ldxdd, ldxi1, ldxi2;
	int rdxinc, rdxdd, rdxi1, rdxi2;
	int lscinc, lscdd, lsci1, lsci2;
	int rscinc, rscdd, rsci1, rsci2;
	int sxinc, sxdd, sxi1, sxi2;
	int syinc, sydd, syi1, syi2;
	unsigned long num;
	ADD_TYPE *add;
	PUT_TYPE *put;

	if (_dst->clip)
	{
		xbeg = _dst->cl;
		xend = _dst->cr;
		ybeg = _dst->ct;
		yend = _dst->cb;
	}
	else
	{
		xbeg = 0;
		xend = _dst->w;
		ybeg = 0;
		yend = _dst->h;
	}

	if ((xbeg >= xend) || (ybeg >= yend))
		return;

	/* Convert angle to [0, 256) range.  */
	_angle &= itofix (256) - 1;

	/* Width and height of source and destination.  */
	sw = _src->w;
	sh = _src->h;
	fdw = fmul (ABS (_scalex), itofix (sw));
	fdh = fmul (ABS (_scaley), itofix (sh));
	dw = fixtoi (fdw);
	dh = fixtoi (fdh);
	if ((dw <= 0) || (dh <= 0))
		return;

	fdw /= 2;
	fdh /= 2;

	/* Center of destination.  */
	fx0 = itofix (_x);
	fy0 = itofix (_y);

	fsinangle = fsin (_angle);
	fcosangle = fcos (_angle);

	/* Map source (half) edges onto destination.  */
	fux = fmul (fdw, fcosangle);
	fuy = fmul (fdw, fsinangle);
	fvx = -fmul (fdh, fsinangle);
	fvy = fmul (fdh, fcosangle);

	/* Coordinates of corners in destination.  */
	point[0].dx = fixtoi (fx0 - fux - fvx);
	point[1].dx = fixtoi (fx0 + fux - fvx);
	point[2].dx = fixtoi (fx0 - fux + fvx);
	point[3].dx = fixtoi (fx0 + fux + fvx);
	point[0].dy = fixtoi (fy0 - fuy - fvy);
	point[1].dy = fixtoi (fy0 + fuy - fvy);
	point[2].dy = fixtoi (fy0 - fuy + fvy);
	point[3].dy = fixtoi (fy0 + fuy + fvy);

	sw <<= aa_BITS;
	dsx = sw / dw;
	if ((unsigned int)dsx < aa_SIZE)
		dsx = aa_SIZE;
	sw -= dsx;

	sh <<= aa_BITS;
	dsy = sh / dh;
	if ((unsigned int)dsy < aa_SIZE)
		dsy = aa_SIZE;
	sh -= dsy;

	num = dsx * dsy;

	/* Avoid overflow.  */
	if (num > aa_MAX_NUM)
	{
		if ((unsigned int)dsx > aa_MAX_SIZE)
			dsx = aa_MAX_SIZE;
		if ((unsigned int)dsy > aa_MAX_SIZE)
			dsy = aa_MAX_SIZE;
		num = dsx * dsy;
	}

	_aa.total = num;
	_aa.inverse = 1 + (0xffffffffUL /  _aa.total );

	/* Coordinates of corners in source.  */
	if (_scalex < 0)
	{
		point[0].sx = sw;
		point[1].sx = 0;
		point[2].sx = sw;
		point[3].sx = 0;
	}
	else
	{
		point[0].sx = 0;
		point[1].sx = sw;
		point[2].sx = 0;
		point[3].sx = sw;
	}
	if (_scaley < 0)
	{
		point[0].sy = sh;
		point[1].sy = sh;
		point[2].sy = 0;
		point[3].sy = 0;
	}
	else
	{
		point[0].sy = 0;
		point[1].sy = 0;
		point[2].sy = sh;
		point[3].sy = sh;
	}

	/* Sort left and right edges.  */
	if ((_angle < itofix (32)) || (_angle >= itofix (128 + 64 + 32)))
	{
		if (point[0].dy < point[1].dy)
		{
			ledge[0] = 0;
			ledge[1] = 2;
			ledge[2] = 3;
			redge[0] = 0;
			redge[1] = 1;
			redge[2] = 3;
		}
		else if (point[0].dy > point[1].dy)
		{
			ledge[0] = 1;
			ledge[1] = 0;
			ledge[2] = 2;
			redge[0] = 1;
			redge[1] = 3;
			redge[2] = 2;
		}
		else
		{
			ledge[0] = 0;
			ledge[1] = 2;
			ledge[2] = 3;
			redge[0] = 1;
			redge[1] = 3;
			redge[2] = 2;
		}
	}
	else if (_angle < itofix (64 + 32))
	{
		if (point[0].dy < point[2].dy)
		{
			ledge[0] = 0;
			ledge[1] = 2;
			ledge[2] = 3;
			redge[0] = 0;
			redge[1] = 1;
			redge[2] = 3;
		}
		else if (point[0].dy > point[2].dy)
		{
			ledge[0] = 2;
			ledge[1] = 3;
			ledge[2] = 1;
			redge[0] = 2;
			redge[1] = 0;
			redge[2] = 1;
		}
		else
		{
			ledge[0] = 2;
			ledge[1] = 3;
			ledge[2] = 1;
			redge[0] = 0;
			redge[1] = 1;
			redge[2] = 3;
		}
	}
	else if (_angle < itofix (128 + 32))
	{
		if (point[2].dy < point[3].dy)
		{
			ledge[0] = 2;
			ledge[1] = 3;
			ledge[2] = 1;
			redge[0] = 2;
			redge[1] = 0;
			redge[2] = 1;
		}
		else if (point[2].dy > point[3].dy)
		{
			ledge[0] = 3;
			ledge[1] = 1;
			ledge[2] = 0;
			redge[0] = 3;
			redge[1] = 2;
			redge[2] = 0;
		}
		else
		{
			ledge[0] = 3;
			ledge[1] = 1;
			ledge[2] = 0;
			redge[0] = 2;
			redge[1] = 0;
			redge[2] = 1;
		}
	}
	else
	{
		if (point[1].dy < point[3].dy)
		{
			ledge[0] = 1;
			ledge[1] = 0;
			ledge[2] = 2;
			redge[0] = 1;
			redge[1] = 3;
			redge[2] = 2;
		}
		else if (point[1].dy > point[3].dy)
		{
			ledge[0] = 3;
			ledge[1] = 1;
			ledge[2] = 0;
			redge[0] = 3;
			redge[1] = 2;
			redge[2] = 0;
		}
		else
		{
			ledge[0] = 1;
			ledge[1] = 0;
			ledge[2] = 2;
			redge[0] = 3;
			redge[1] = 2;
			redge[2] = 0;
		}
	}

	/* Remove wrong edges on bottom.  */
	if (point[ledge[0]].dy == point[ledge[1]].dy)
	{
		ledge[0] = ledge[1];
		ledge[1] = ledge[2];
	}
	if (point[ledge[1]].dy >= point[ledge[2]].dy)
		ledge[2] = -1;
	ledge[3] = -1;

	if (point[redge[0]].dy == point[redge[1]].dy)
	{
		redge[0] = redge[1];
		redge[1] = redge[2];
	}
	if (point[redge[1]].dy >= point[redge[2]].dy)
		redge[2] = -1;
	redge[3] = -1;

	/* Completely clipped by y?  */
	if ((point[ledge[0]].dy >= yend)
		|| ((ledge[2] == -1) && (point[ledge[1]].dy < ybeg))
		|| (point[ledge[2]].dy < ybeg))
		return;

	/* Color manipulation routines.  */
	add = get_aa_add_function(_src, _mode);
	put = get_aa_put_function(_dst, _mode);
	if (!add || !put) return;

	lindex = 1;
	rindex = 1;
	lpoint1 = &point[ledge[0]];
	lpoint2 = &point[ledge[1]];
	rpoint1 = &point[redge[0]];
	rpoint2 = &point[redge[1]];

	dy = lpoint1->dy;
	if (ledge[2] == -1)
	{
		if (point[ledge[1]].dy < yend)
		yend = point[ledge[1]].dy + 1;
	}
	else if (point[ledge[2]].dy < yend)
		yend = point[ledge[2]].dy + 1;

	ldx = lpoint1->dx;
	aa_PREPARE (ldxinc, ldxdd, ldxi1, ldxi2,
		lpoint2->dx - lpoint1->dx, lpoint2->dy - lpoint1->dy);

	lsx = lpoint1->sx;
	lsy = lpoint1->sy;
	if (lpoint1->sx != lpoint2->sx)
	{
		lsc = &lsx;
		aa_PREPARE (lscinc, lscdd, lsci1, lsci2,
			lpoint2->sx - lpoint1->sx, lpoint2->dy - lpoint1->dy);
	}
	else
	{
		lsc = &lsy;
		aa_PREPARE (lscinc, lscdd, lsci1, lsci2,
			lpoint2->sy - lpoint1->sy, lpoint2->dy - lpoint1->dy);
	}

	rdx = rpoint1->dx;
	aa_PREPARE (rdxinc, rdxdd, rdxi1, rdxi2,
		rpoint2->dx - rpoint1->dx, rpoint2->dy - rpoint1->dy);

	rsx = rpoint1->sx;
	rsy = rpoint1->sy;
	if (rpoint1->sx != rpoint2->sx)
	{
		rsc = &rsx;
		aa_PREPARE (rscinc, rscdd, rsci1, rsci2,
			rpoint2->sx - rpoint1->sx, rpoint2->dy - rpoint1->dy);
	}
	else
	{
		rsc = &rsy;
		aa_PREPARE (rscinc, rscdd, rsci1, rsci2,
			rpoint2->sy - rpoint1->sy, rpoint2->dy - rpoint1->dy);
    }

	/* Skip region clipped on top.  */
	while (dy < ybeg)
	{
		dy++;

		if (dy > lpoint2->dy)
		{
			if (ledge[++lindex] == -1)
				return;
			lpoint1 = lpoint2;
			lpoint2 = &point[ledge[lindex]];

			if (lpoint1->sx != lpoint2->sx)
			{
				lsc = &lsx;
				aa_PREPARE (lscinc, lscdd, lsci1, lsci2,
					lpoint2->sx - lpoint1->sx, lpoint2->dy - lpoint1->dy);
			}
			else
			{
				lsc = &lsy;
				aa_PREPARE (lscinc, lscdd, lsci1, lsci2,
					lpoint2->sy - lpoint1->sy, lpoint2->dy - lpoint1->dy);
			}
			aa_PREPARE (ldxinc, ldxdd, ldxi1, ldxi2,
				lpoint2->dx - lpoint1->dx, lpoint2->dy - lpoint1->dy);
		}
		aa_ADVANCE (*lsc, lscinc, lscdd, lsci1, lsci2);
		aa_ADVANCE (ldx, ldxinc, ldxdd, ldxi1, ldxi2);

		if (dy > rpoint2->dy)
		{
			if (redge[++rindex] == -1)
				return;
			rpoint1 = rpoint2;
			rpoint2 = &point[redge[rindex]];

			if (rpoint1->sx != rpoint2->sx)
			{
				rsc = &rsx;
				aa_PREPARE (rscinc, rscdd, rsci1, rsci2,
					rpoint2->sx - rpoint1->sx, rpoint2->dy - rpoint1->dy);
			}
			else
			{
				rsc = &rsy;
				aa_PREPARE (rscinc, rscdd, rsci1, rsci2,
					rpoint2->sy - rpoint1->sy, rpoint2->dy - rpoint1->dy);
			}
			aa_PREPARE (rdxinc, rdxdd, rdxi1, rdxi2,
				rpoint2->dx - rpoint1->dx, rpoint2->dy - rpoint1->dy);
		}
		aa_ADVANCE (*rsc, rscinc, rscdd, rsci1, rsci2);
		aa_ADVANCE (rdx, rdxinc, rdxdd, rdxi1, rdxi2);
	}

	/* Stretch lines.  */
	while (dy < yend)
	{
		unsigned long daddr = bmp_write_line (_dst, dy);
		_aa.y = dy;

		if ((ldx < xend) && (rdx >= xbeg))
		{
			int curxend;

			aa_PREPARE (sxinc, sxdd, sxi1, sxi2,
				rsx - lsx, rdx - ldx);
			aa_PREPARE (syinc, sydd, syi1, syi2,
				rsy - lsy, rdx - ldx);

			for (sx = lsx, sy = lsy, dx = ldx; dx < xbeg; dx++)
			{
				aa_ADVANCE (sx, sxinc, sxdd, sxi1, sxi2);
				aa_ADVANCE (sy, syinc, sydd, syi1, syi2);
			}

			curxend = (rdx < xend) ? (rdx + 1) : xend;
			for (; dx < curxend; dx++)
			{
				(*add) (_src, sx, sx + dsx, sy, sy + dsy);
				AA_PUT_PIXEL(put, daddr, dx);

				aa_ADVANCE (sx, sxinc, sxdd, sxi1, sxi2);
				aa_ADVANCE (sy, syinc, sydd, syi1, syi2);
			}
		}

		dy++;

		if (dy > lpoint2->dy)
		{
			if (ledge[++lindex] == -1)
				return;
			lpoint1 = lpoint2;
			lpoint2 = &point[ledge[lindex]];

			if (lpoint1->sx != lpoint2->sx)
			{
				lsc = &lsx;
				aa_PREPARE (lscinc, lscdd, lsci1, lsci2,
				lpoint2->sx - lpoint1->sx, lpoint2->dy - lpoint1->dy);
			}
			else
			{
				lsc = &lsy;
				aa_PREPARE (lscinc, lscdd, lsci1, lsci2,
					lpoint2->sy - lpoint1->sy, lpoint2->dy - lpoint1->dy);
			}
			aa_PREPARE (ldxinc, ldxdd, ldxi1, ldxi2,
				lpoint2->dx - lpoint1->dx, lpoint2->dy - lpoint1->dy);
		}
		aa_ADVANCE (*lsc, lscinc, lscdd, lsci1, lsci2);
		aa_ADVANCE (ldx, ldxinc, ldxdd, ldxi1, ldxi2);

		if (dy > rpoint2->dy)
		{
			if (redge[++rindex] == -1)
				return;
			rpoint1 = rpoint2;
			rpoint2 = &point[redge[rindex]];

			if (rpoint1->sx != rpoint2->sx)
			{
				rsc = &rsx;
				aa_PREPARE (rscinc, rscdd, rsci1, rsci2,
				rpoint2->sx - rpoint1->sx, rpoint2->dy - rpoint1->dy);
			}
			else
			{
				rsc = &rsy;
				aa_PREPARE (rscinc, rscdd, rsci1, rsci2,
					rpoint2->sy - rpoint1->sy, rpoint2->dy - rpoint1->dy);
			}
			aa_PREPARE (rdxinc, rdxdd, rdxi1, rdxi2,
				rpoint2->dx - rpoint1->dx, rpoint2->dy - rpoint1->dy);
		}
		aa_ADVANCE (*rsc, rscinc, rscdd, rsci1, rsci2);
		aa_ADVANCE (rdx, rdxinc, rdxdd, rdxi1, rdxi2);
	}
	bmp_unwrite_line(_dst); //this helps if dest is a video bitmap
}

/*
 * Anti-aliased bitmap rotation with scaling.
 */
void
aa_rotate_scaled_bitmap (BITMAP *_src, BITMAP *_dst, int _x, int _y, fixed _angle,
			 fixed _scalex, fixed _scaley)
{
  _aa_rotate_bitmap (_src, _dst, _x, _y, _angle, _scalex, _scaley, 
	  aa_get_mode());
}

/*
 * Anti-aliased bitmap rotation with scaling (masked).
 */
void
aa_rotate_scaled_sprite (BITMAP *_dst, BITMAP *_src, int _x, int _y, fixed _angle,
			 fixed _scalex, fixed _scaley)
{
  //_aa_rotate_bitmap (_src, _dst, _x, _y, _angle, _scalex, _scaley, 1);
	if ((_angle+16384) & 0x003f8000)
		_aa_rotate_bitmap (
			_src, _dst, 
			_x + ((_src->w*_scalex)>>17), _y + ((_src->h*_scaley)>>17), 
			_angle, 
			_scalex-256, 
			_scaley-256, 
			AA_MASKED | aa_get_mode());
	else 
		rotate_scaled_sprite(_dst, _src, _x, _y, _angle, _scalex);//hope they're the same
	return;
}

/*
 * Anti-aliased bitmap rotation.
 */
void
aa_rotate_bitmap (BITMAP *_src, BITMAP *_dst, int _x, int _y, fixed _angle)
{
  _aa_rotate_bitmap (_src, _dst, _x, _y, _angle, itofix (1), itofix (1), 0);
}

/*
 * Anti-aliased bitmap rotation (masked).
 */
void
aa_rotate_sprite (BITMAP *_dst, BITMAP *_src, int _x, int _y, fixed _angle)
{
//		_aa_rotate_bitmap (_src, _dst, _x, _y, _angle, itofix (1), itofix (1), 1);
//seems to be buggy, changing it to this:
	if ((_angle+16384) & 0x003f8000)
		_aa_rotate_bitmap (
			_src, _dst, 
			_x + _src->w/2, 
			_y + _src->h/2, 
			_angle, 
			itofix (1), 
			itofix (1), 
			AA_MASKED | aa_get_mode());
	else 
		rotate_sprite(_dst, _src, _x, _y, _angle);
	return;
}

/*
 * aarot.c ends here
 */
