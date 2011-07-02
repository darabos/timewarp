/* $Id$ */ 
/*

  The following ini settings are required in server.ini, under [Planet]

PlanetDimension = 3       ; 2 = standard 2d sprite, 3 = Tau's 3D planet idea.
PlanetUsespec = 1         ; 1 = uses spec and shading, 0 = uses only shading.


*/

#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#include "../util/pmask.h"
#include "../util/aastr.h"

#include <stdio.h>
#include <string.h>

#include "planet3d.h"
#include "../melee/mlog.h"


SpaceSprite::SpaceSprite(BITMAP *image, int _attributes)
{

	if (_attributes == -1) _attributes = string_to_sprite_attributes(NULL);
	general_attributes = _attributes;

	int i;
	BITMAP *bmp;
	count = 1;

	references = 0;
	sbitmap = 0;

	bpp = 32;//videosystem.bpp;

	smask = new PMASK*			[count];
	sbitmap = new BITMAP*		[count];

	bmp = create_bitmap_ex ( bpp, image->w, image->h);
	convert_bitmap(image, bmp, general_attributes & MASKED);
	
	i = 0;
	color_correct_bitmap(bmp, general_attributes & MASKED);
	smask[i] = create_allegro_pmask(bmp);
	sbitmap[i] = bmp;

	if (!sbitmap[0])
	{
		throw("Basic sprite shape expected, but doesn't exist");
	}
}





/*
class Vector3D
{
public:
	double x, y, z;

	Vector3D(double ax, double ay, double az);

	double dot(Vector3D v);
	Vector3D cross(Vector3D b);

	void normalize();

	Vector3D &operator= (_Ignore_Me *nothing) {x = y = z = 0; return *this;}
};

inline Vector3D operator+ (Vector3D a, Vector3D b) { return Vector3D(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3D operator- (Vector3D a, Vector3D b) { return Vector3D(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3D operator* (double b, Vector3D a) { return Vector3D(a.x*b, a.y*b, a.z*b); }


Vector3D::Vector3D(double ax, double ay, double az)
{
	x = ax;
	y = ay;
	z = az;
}

double Vector3D::dot(Vector3D v)
{
	return x*v.x + y*v.y + z*v.z;
}

Vector3D Vector3D::cross(Vector3D b)
{
	return Vector3D(y*b.z - z*b.y, -x*b.z + z*b.x, x*b.y - y*b.x);
}

void Vector3D::normalize()
{
	double r;
	r = sqrt(x*x + y*y + z*z);
	x /= r;
	y /= r;
	z /= r;
}
*/





// something extra is needed ...

// call this in void NormalGame::init_objects() { 
// instead of the creation of a 2D planet.
Planet *create_planet( Vector2 position )
{
	 

	// first, check the server.ini settings to see which type of planet you want
	// to create: a 2D planet, or a 3D planet ?!
	//
	// add this line to server.ini, under [Planets]
	// PlanetDimension = 3       ; 2 = standard 2d sprite, 3 = Tau's 3D planet idea.

	set_config_file ("server.ini");


	int planetD, PlanetUsespec;
	planetD = get_config_int("Planet", "PlanetDimension", 2);
	PlanetUsespec = get_config_int("Planet", "PlanetUsespec", 1);

	// this is necessary: cause the planets have different sizes.
	share(-1, &planetD);
	share_update();
	

	// check if the 3D data file exists
	if (!exists("planet3d.dat"))
		planetD = 2;


	if (planetD == 2)
	{
		// create a 2D planet
		Planet *planet = new Planet (position, meleedata.planetSprite, tw_random(meleedata.planetSprite->frames()));
		game->add (planet);
		return planet;

	} else if ( planetD == 3 ) {

		SpaceSprite *color_map, *spec_map, *dummy;
		DATAFILE *rawdata = load_datafile("planet3d.dat");
		
		if(!rawdata) throw("Error loading planet3D data");
		
		
		// select one of the planets (at random) ...
		int planetlist[32]; 
		int Nplanets = 0;
		int k = 0;

		char name[128];

		while (rawdata[k].type != DAT_END)
		{
			if (rawdata[k].type == DAT_BITMAP )
			{

				// check the name of this object:

				strncpy(name, get_datafile_property(&rawdata[k], DAT_NAME), 128);
				
				// check if it is a _color_bmp (and not a _spec_bmp)
				if (strcmp(&name[strlen(name)-strlen("COLOR_BMP")], "COLOR_BMP") == 0)
				{
					planetlist[Nplanets] = k;
					++Nplanets;
				}
			}
			
			++k;
		}

		if (Nplanets == 0)
			throw("No planet data !!");
		
		k = planetlist[tw_random(Nplanets)];
		
		color_map  = new SpaceSprite(&rawdata[k], 1, SpaceSprite::MASKED );

		// first, determine the spec map name from the color name, by
		// replacing the last few characters.
		strncpy(name, get_datafile_property(&rawdata[k], DAT_NAME), 128);
		name[strlen(name)-strlen("COLOR_BMP")] = 0;
		strcat(name, "SPEC_BMP");

		// load the data file part containing this name; 
		DATAFILE *datapart;
		datapart = find_datafile_object(rawdata, name);
		if (!(datapart && datapart->type == DAT_BITMAP))
		{
			throw("Cannot find the specular map of the planet, or not a bmp");
		}
		// create a sprite
		spec_map = new SpaceSprite(datapart, 1);
		
		unload_datafile(rawdata);
		
		int mapW = (int)color_map->size().x;
		int mapH = (int)color_map->size().y;

		if (spec_map->size().x != mapW || spec_map->size().y != mapH )
		{
			throw("color map and spec map have different sizes !!");
		}
		
		
		// the image is already correct on the y-axis (since the maps are cylindrical projections).
		
		int planet_radius = 100;
		int image_size = 2 * planet_radius;
		
		
		BITMAP *image32bit = create_bitmap_ex(32, image_size, image_size);
		clear_to_color(image32bit, makecol32(255,0,255));
		circlefill(	image32bit,
			image32bit->w/2, image32bit->h/2,
			image32bit->w/2 - 2, makecol(255,255,255));
		
	    dummy  = new SpaceSprite(image32bit, SpaceSprite::MASKED);
		
		double tilt, spin, sun_hangle, sun_vangle;
		tilt = tw_random(-40.0, 40.0);
		spin = tw_random(-30.0, 30.0);
		sun_vangle = tw_random(-20.0, 60.0);
		sun_hangle = tw_random(-45.0, 45.0);
		Planet3D *planet = new Planet3D(position, color_map, spec_map, dummy, planet_radius,
											PlanetUsespec, spin, 
											tilt, sun_vangle, sun_hangle, 1.0, 1.0, 1.0);
		game->add (planet);
		return planet;

	} else {
		throw("Unknown planet dimension in create_planet()");
	}

	return 0;

	// create a 3D planet
}




Planet3D::Planet3D( Vector2 opos, SpaceSprite *color_map, SpaceSprite *spec_map,
						SpaceSprite *ObjectSprite,
						int planet_radius, int aPlanetUsespec,
						double turn_rate, double tilt, 
						double sun_vertangle,	// positive is pointing down
						double sun_horizangle,	// oriented along x
						double sun_r, double sun_g, double sun_b,
						bool invcolor)
:
Planet( opos, ObjectSprite, 0 )
{

	PlanetUsespec = aPlanetUsespec;

	sprite_index = 0;

	spin = turn_rate;


	map = color_map;
	
	dummy = ObjectSprite;

	mapW = (int)map->size().x;
	mapH = (int)map->size().y;

	image_size = planet_radius*2;
	visual_size = image_size / 2 - 1;

	image32bit = create_bitmap_ex(32, image_size, image_size);
	clear_to_color(image32bit, makecol32(255,0,255));

	// there are 4 chars containing the colors
	// for a map of size W, H
	// and I keep an extra copy of the map (for the rotation extrapolates on there).
	color_map_linear = new unsigned char [4*mapW*mapH * 2];
	spec_map_linear = new unsigned char [4*mapW*mapH * 2];


	int ccc;
	int i,j;

	for (i = 0; i < mapW; i++)
		for (j = 0; j < mapH; j++)
		{
			ccc=getpixel(map->get_bitmap(0), i, j);

			int spec;
			if (spec_map)
				spec = getpixel(spec_map->get_bitmap(0), i, j);
			else
				spec = makecol(200,200,200);

			unsigned char r, g, b;
			unsigned char sr, sg, sb;

			// AT THIS MOMENT, it's a good moment to find out in which way the
			// videocard interprets the colors ... as rgb, or as bgr ??!!

			// map coordinate
			int k = (2*mapW*mapH) - (j+1)*(2*mapW) + i;
			if (!invcolor)
			{
				r = getr32(ccc);
				g = getg32(ccc);
				b = getb32(ccc);
			} else {
				b = getr32(ccc);
				g = getg32(ccc);
				r = getb32(ccc);
			}

			// filter the colors by the reference sun
			r = (unsigned char)(sun_r * r);
			g = (unsigned char)(sun_g * g);
			b = (unsigned char)(sun_b * b);

			// for testing:
//			r = 200;
//			g = 200;
//			b = 200;

//			color_map_linear[4*k] = b;
//			color_map_linear[4*k+1] = g;
//			color_map_linear[4*k+2] = r;
//			color_map_linear[4*k+3] = 128;
			*((int*) &color_map_linear[4*k]) = makecol32(r,g,b);

			// repeat this for the spec map

			sr = (unsigned char)(sun_r * getr32(spec));
			sg = (unsigned char)(sun_g * getg32(spec));
			sb = (unsigned char)(sun_b * getb32(spec));

			*((int*) &spec_map_linear[4*k]) = makecol32(sr,sg,sb);


			// keep an extra copy in memory.
			k += mapW;
			*((int*) &color_map_linear[4*k]) = makecol32(r,g,b);

			*((int*) &spec_map_linear[4*k]) = makecol32(sr,sg,sb);

		}

	delete map;
	if (spec_map)
		delete spec_map;
	map = 0;
	spec_map = 0;
	color_map = 0;

	theta = tilt * ANGLE_RATIO;
	fi =  0;// should always be 0 !!//125* ANGLE_RATIO;
	rad = 0;

	double L, D, d;
	double focus;
	
	double th=theta;

	base_map = (base_map_type*)malloc(sizeof(base_map_type)*image_size*image_size);

	base_map_linear = new unsigned int [image_size*image_size];
	base_shade_linear = new unsigned int [image_size*image_size];
	base_spec_linear = new unsigned int [image_size*image_size];

	base_map_and_shade_resorted = new unsigned int [2*image_size*image_size];

	double fi_s = sun_horizangle*ANGLE_RATIO;
	double th_s = sun_vertangle * ANGLE_RATIO; //sun position

	
	double ts[3][3];
	double tm[3][3];


	tm[0][0] = cos(th)*cos(fi);
	tm[0][1] = sin(th);
	tm[0][2] = -cos(th)*sin(fi);
	tm[1][0] = -sin(th)*cos(fi);
	tm[1][1] = cos(th);
	tm[1][2] = sin(th)*sin(fi);
	tm[2][0] = sin(fi);
	tm[2][1] = 0;
	tm[2][2] = cos(fi);

	ts[0][0] = cos(fi_s)*cos(th_s);
	ts[0][1] = -sin(fi_s);
	ts[0][2] = cos(fi_s)*sin(th_s);
	ts[1][0] = sin(fi_s)*cos(th_s);
	ts[1][1] = cos(fi_s);
	ts[1][2] = sin(fi_s)*sin(th_s);
	ts[2][0] = -sin(th_s);
	ts[2][1] = 0;
	ts[2][2] = cos(th_s);


	// x, y is in the drawing area;
	// z points outside the screen.

	Vector3D observer(0,0,1);
	Vector3D sunshine(cos(fi_s)*cos(th_s), sin(fi_s)*cos(th_s), -sin(th_s));
	sunshine.normalize();

	L = 10;
	D = L*L - 1;
	d = sqrt(D);

	double xx,yy,zz, rr, ff, tg2;
//	double xf, yf, zf;//final;
	double xs,ys,zs;//sun
	double lon, lat;

	double min_light = 0.2;
	double max_light = 1.0;

	focus = (visual_size) * d;//tan(view_angle/2);


	for (i=-image_size/2; i<image_size/2; i++)
	{

		int k = i + image_size/2;
		if (k >= image_size)	continue;
		jmin[k] = 100000;
		jmax[k] = 0;

		for (j=-image_size/2; j<image_size/2; j++)
		{
			if ((i*i+j*j) < visual_size*visual_size)
			{
				//-int k = i + image_size/2;
				int m = j + image_size/2;
				if (m >= image_size)	continue;

				if (m < jmin[k])
					jmin[k] = m;

				if (m > jmax[k])
					jmax[k] = m;


				
				ff = (i*i+j*j);
				tg2 = ff / (focus*focus);

				zz = sqrt(L*L - D*(1+tg2)) / (1 + tg2);
				rr = sqrt(1 - zz*zz);

				if (ff > 0) {
					xx = rr * i / sqrt(ff);
					yy = rr * j / sqrt(ff); }
				else {
					xx = 0; yy = 0; }
					
//				xx = i/double(visual_size);
//				yy = j/double(visual_size);
//				zz = sqrt(1 - xx*xx - yy*yy);

				xs = xx; ys = yy; zs = zz;
				// = position of the visible sphere point in 3D space ?

				Vector3D surfacenormal(xs, ys, zs);
				surfacenormal.normalize();
				// the point that is facing you ;)

				// now, calculate the light intensity.
				// two parts: how much light does a part of the earth receive,
				// and how much does it radiate out:

				double I, J;
				I = -1 * sunshine.dot(surfacenormal);	// incoming sunshine is in opposite direction from outgoing surface normal
//				I *= surfacenormal.dot(observer);		// this is directional shading... shouldn't be here.
				if (I < 0)
					I = 0;
				I = min_light + I*(max_light-min_light);

/*
				I = -1 * sunshine.dot(surfacenormal);
//				I *= surfacenormal.dot(observer);
> That does not look right. Surface illumination intensity should be simply
> (sunshine dot surfacenormal) - that is, the first line only.  The second
> "assumes" that the diffused light varies depending on the angle of
> observation, which is generally NOT the case (for flat surfaces)!  Things
> like, say, bright sand with some (scarce) really _dark_ grass growing on
it
> may have such properties (but it will also be affected by (sunshie dot
> surfacenormal)^2 due to shadows from the source).  On most Earth surfaces
> (but water) it is generally safe to assume "pure" diffusive reflection (it
> will be most realistic, a.k.a less erroneous).

I think I've made a mistake.

Here's what I probably did wrong: I was assuming there's a set of
finite flat surfaces convering the sphere. When you rotate one of
these surfaces, the area you see becomes smaller, hence, you
see less light.

What I forgot is that, a curved surface has rotated surfaces,
but has a lot of them ... so I should integrate over a few of
them... enough to fill the square pixel area.

Thus you're right, at least approximately (since the sphere
is not a flat surface).

(resolution is good enough that the curvature makes little
difference, I think).

*/

				// and, what about specular, i.e., reflections of light towards
				// the observer?
				Vector3D reflection(0,0,0);	// the reflected sun-light direction
				reflection = -1*(sunshine.cross(surfacenormal)).cross(surfacenormal) - sunshine.dot(surfacenormal) * surfacenormal;
				J = reflection.dot(observer);
				if (J < 0)
					J = 0;
				J *= J;	// to make it more pronounced :)
				J *= J;
				J *= J;
				J *= J;
				J *= 0.8;
/*

(a) > reflection = -1*(sunshine.cross(surfacenormal)).cross(surfacenormal) -
(b) > sunshine.dot(surfacenormal) * surfacenormal;
(c) > J = reflection.dot(observer);

a) calculates the light-vector component perpendicular to the surface normal
It shouldn't be normalized.

b) is the (inverted) light-vector component parallel to the surface normal

c) is the cos(angle) of the reflected ray towards the observer; in
principle,
only rays where angle=0 can be observed for a reflection;
I've used J to the power (something) to make a kinda near-delta pulse
filter instead.

*/
				
				
//				J = 0;
				
				
				
				xx = tm[0][0] * xs + tm[0][1] * ys + tm[0][2] * zs;
				yy = tm[1][0] * xs + tm[1][1] * ys + tm[1][2] * zs;
				zz = tm[2][0] * xs + tm[2][2] * zs;
				// = sphere point rotated wrt observer (for plotting) ?
				// it's rotated backwards I think, so that it'll point to
				// the lat,lon with the surface property.
					
//				xf = ts[0][0] * xs + ts[0][1] * ys + ts[0][2] * zs;
//				yf = ts[1][0] * xs + ts[1][1] * ys + ts[1][2] * zs;
//				zf = ts[2][0] * xs + ts[2][2] * zs;
				// = sphere point rotated wrt sun (for light) ?

				// sun is to the right ?
				// observer is perp. to the screen ?


				if (fabs(zz) > 1e-10) {
					//orz: I changed it from fabsl() to fabs() for compatibility reasons
					if (zz > 0) lon = atan(xx/zz);
					else lon = PI+atan(xx/zz); }
				else
					if (xx > 0) lon = PI/2;
					else lon = -PI/2;

				if (yy*yy < 1-1e-10)
					lat = atan(yy/sqrt(1-yy*yy));
				else {
					if (yy > 0) lat = PI/2;
					else lat = -PI/2; }

	

				double mx = mapW * lon / (2*PI);
				while (mx >= mapW) mx -= mapW;
				while (mx < 0) mx += mapW;
				double my = mapH * (1 - lat/(PI/2)) / 2.0;
				while (my >= mapH) my -= mapH;
				while (my < 0) my += mapH;

				base_map[(i+image_size/2)*image_size+j+image_size/2].lat = my;
				base_map[(i+image_size/2)*image_size+j+image_size/2].lon = mx;


				/*
				double ll;
				if (zf <= 0) ll = 0;
				else	ll = zf;
				double l1 = min_light + ll*(max_light-min_light);

				if (zf >= 0) {
					ll*=ll;
					ll*=ll;
					ll*=ll;
					ll*=ll;
					ll*=ll;
					ll*=0.8;
				}
				else ll = 0;
				*/


				base_map[(i+image_size/2)*image_size+j+image_size/2].diff = I;//l1;
				base_map[(i+image_size/2)*image_size+j+image_size/2].spec = J;//ll;
			}
			else {
				base_map[(i+image_size/2)*image_size+j+image_size/2].lat = 0;
				base_map[(i+image_size/2)*image_size+j+image_size/2].lon = 0;
				base_map[(i+image_size/2)*image_size+j+image_size/2].diff = 0;
				base_map[(i+image_size/2)*image_size+j+image_size/2].spec = 0;
			}

			int k = (i+image_size/2) * image_size + j + image_size/2;
			if (k >= image_size*image_size)
				k = image_size*image_size - 1;

			int mx2, my2;
			mx2 = (int)base_map[k].lon;
			if (mx2 >= mapW) mx2 -= mapW;
			my2 = (int)base_map[k].lat;
			if (my2 >= mapH) my2 -= mapH;

			base_map_linear[k] = mx2 + my2 * 2*mapW;	// twice cause there's an extra copy
														// of the map colors


			// so that the shading doesn't become excessive.

			unsigned char shading;
			int shading_toomuch;
			shading_toomuch = int(base_map[k].diff * 255);
			if (shading_toomuch > 255)
				shading = 255;
			else
				shading = shading_toomuch;
			// 255 means, brightness is almost unaffected.

			base_shade_linear[k] = shading;


			base_spec_linear[k] = int(base_map[k].spec * 255);

		}
	}


	// re-sort some arrays:
	unsigned int *resort = base_map_and_shade_resorted;
	for ( j = 0; j < image_size; ++j )
	{
		for ( i = 0; i < image_size; ++i )
		{
			int k = i*image_size + j;
			*resort = base_map_linear[k];
			++resort;

			unsigned char s1 = base_shade_linear[k];
			unsigned char s2 = base_spec_linear[k];
			*resort = s1 | s2 << 8;
			++resort;

		}
	}

	draw_reserve = 0;


	delete base_map;
	base_map = 0;

}

void Planet3D::calculate()
{
	 

	Planet::calculate();

	sprite_index = 0;

	normalize(theta, 360*ANGLE_RATIO);

	rad += spin * (frame_time/1000.0);
	while (rad >= 360 ) rad -= 360;
	while (rad < 0) rad += 360;		// in case of anti-spin

}

void Planet3D::animate_pre()
{
	 


//	if (!space_view->in_view(pos, size)) return;
	
	//dummy->unlock();
	BITMAP *tmp = dummy->get_bitmap(0);
	//dummy->lock();
	//clear_to_color(tmp, makecol(255,0,255));

	int i,j;

	double dl = mapW*rad/360.0;
	while (dl >= mapW) dl -= mapW;

	unsigned int* imageptr = (unsigned int*) image32bit->dat;

	unsigned int *base_sorted = base_map_and_shade_resorted;

	//unsigned int d_shift = unsigned int(dl);
	unsigned int d_shift = (unsigned int)dl; //changed 7/1/2003 Culture20

	unsigned char *color_map_linear2 = &color_map_linear[d_shift << 2];	// a shift by dl int's
	unsigned char *spec_map_linear2 = &spec_map_linear[d_shift << 2];	// a shift by dl int's

	for (i=0; i<image_size; i++)
	{
		// NOTE: this loop can be altered such that it won't iterate over "empty" space
		// by adding boundaries jmin[i] and jmax[i]
		//for (j=0; j<image_size; j++)

		if (jmin[i] >= jmax[i])
		{
			imageptr += image_size;
			base_sorted += 2 * image_size;
			continue;
		}
		

		imageptr += jmin[i];	// cause it must start at position jmin

		//int m = i + jmin[i]*image_size;
		base_sorted += 2*jmin[i];


		for (j = jmin[i]; j <= jmax[i]; j++)
		{


			unsigned char *col;
			unsigned char *speccol;
			unsigned short int specshade;

		#if (defined (_MSC_VER))// && defined (__ia32__))
		//#ifdef _MSC_VER
			// for better efficiency, I'll rewrite this stuff in assembler code:

			unsigned short int colorshade;

			if (!PlanetUsespec)
			_asm
			{
//			col = &color_map_linear2[(*base_sorted) << 2];

			mov edx, base_sorted
			mov eax, dword ptr [edx]	// eax = (*base_sorted)
			shl eax, 2

			mov edx, color_map_linear2
			add edx, eax
			mov col, edx

//			++base_sorted;
			add base_sorted, 4
//			shade = *base_sorted;

			// store the shade in bx:
			mov edx, base_sorted;
			mov bl, byte ptr [edx]
			xor bh, bh


//			result = 0;
			xor ecx, ecx		// ecx = result

			// blue
//			e = (*col) * shade;
//			e &= 0x0FF00;
//			result |= e;
//			result <<= 8;
			
			mov edx, col

			mov al, byte ptr [edx]
			xor ah, ah

//			mov bl, shade
//			xor bh, bh

			mul bx
			or  ch, ah

			shl ecx, 8
			

			
			// ++col
			inc col	
			mov edx, col	// this is already present in edx

			mov al, byte ptr [edx]
			xor ah, ah

			// multiply with the shade
			mul bx
			or  ch, ah




			// red
//			++col;
//			e = (*col) * shade;
//			e >>= 8;
//			result |= e;

			inc col
			mov edx, col

			mov al, byte ptr [edx]
			xor ah, ah

			// multiply with the shade
			mul bx
			or  cl, ah

			mov edx, imageptr
			mov dword ptr [edx], ecx

			//++imageptr;
			add edx, 4
			mov imageptr, edx
			

			// Note that, since shade<256 and color<256, the result of the multiplication
			// will always fit within a short-int
			// And, shifting that resulting value to the right by one byte, automatically
			// generates a value <256.

			
			//++base_sorted;
			add base_sorted, 4
			
			}
			else
			_asm
			{

			//col = &color_map_linear2[(*base_sorted) << 2];
			//speccol = &color_spec_linear2[(*base_sorted) << 2];

			mov edx, base_sorted
			mov eax, dword ptr [edx]	// eax = (*base_sorted)
			shl eax, 2

			mov edx, color_map_linear2
			add edx, eax
			mov col, edx

			mov edx, spec_map_linear2
			add edx, eax
			mov speccol, edx

//			++base_sorted;
			add base_sorted, 4
//			shade = *base_sorted;

			// store the shade
			mov edx, base_sorted
			mov al, byte ptr [edx]
			xor ah, ah
			mov colorshade, ax

			// store the specshade (is the next byte)
			mov edx, base_sorted
			inc edx
			mov al, byte ptr [edx]
			xor ah, ah
			mov specshade, ax

//			result = 0;
			xor ecx, ecx		// ecx = result

//			// blue
//			e = (*col) * shade;
//			e += (*speccol) * specshade;
//			if (e & 0x010000)
//				e = 0x0FF00;			// truncation.
//			e &= 0x0FF00;
//			result |= e;
//			result <<= 8;
			
			xor eax, eax
			mov edx, col
			mov al, byte ptr [edx]
			mul colorshade

			mov bx, ax
			xor ah, ah
			mov edx, speccol
			mov al, byte ptr [edx]	// THIS CAN CRASH !! (dunno why)
			mul specshade
			add ax, bx

			jnc ignore1		// in case of overflow, you substitute a "max" value
				mov ah, 0x0FF
ignore1:

			or  ch, ah
			shl ecx, 8
			

			// green
//			++col;
//			++speccol;
//			e = (*col) * shade;
//			e += (*speccol) * specshade;
//			if (e & 0x010000)
//				e = 0x0FF00;			// truncation.
//			e &= 0x0FF00;
//			result |= e;
			
			inc speccol
			inc col	

			xor eax, eax
			mov edx, col	// this is already present in edx
			mov al, byte ptr [edx]
			mul colorshade


			// the specular light:

			mov bx, ax
			xor ah, ah
			mov edx, speccol
			mov al, byte ptr [edx]
			mul specshade
			add ax, bx

			jnc ignore2
				mov ah, 0x0FF
ignore2:

			or  ch, ah


//			// red
//			++col;
//			++speccol;
//			e = (*col) * shade;
//			e += (*speccol) * specshade;
//			if (e & 0x010000)
//				e = 0x0FF00;			// truncation.
//			e &= 0x0FF00;
//			e >>= 8;
//			result |= e;

			inc speccol
			inc col	

			xor eax, eax
			mov edx, col	// this is already present in edx
			mov al, byte ptr [edx]
			mul colorshade



			// the specular light:
			mov bx, ax
			xor ah, ah
			mov edx, speccol
			mov al, byte ptr [edx]
			mul specshade
			add ax, bx

			jnc ignore3
				mov ah, 0x0FF
ignore3:
			or  cl, ah





			mov edx, imageptr
			mov dword ptr [edx], ecx

			//++imageptr;
			add edx, 4
			mov imageptr, edx
			

			// Note that, since shade<256 and color<256, the result of the multiplication
			// will always fit within a short-int
			// And, shifting that resulting value to the right by one byte, automatically
			// generates a value <256.

			
			//++base_sorted;
			add base_sorted, 4

			}
		#else
			unsigned short int shade;
			unsigned int result, e;

			if (!PlanetUsespec)
			{
				col = &color_map_linear2[(*base_sorted) << 2];
				
				++base_sorted;
				
				shade = *((unsigned char*) base_sorted);
				
				result = 0;
				
				// red
				e = (*col) * shade;
				e &= 0x0FF00;
				result |= e;
				result <<= 8;
				
				// green
				++col;
				e = (*col) * shade;
				e &= 0x0FF00;
				result |= e;
				
				
				// blue
				++col;
				e = (*col) * shade;
				e >>= 8;
				result |= e;
				
				*imageptr = result;
				
				++imageptr;
				
				
				// Note that, since shade<256 and color<256, the result of the multiplication
				// will always fit within a short-int
				// And, shifting that resulting value to the right by one byte, automatically
				// generates a value <256.
				
				++base_sorted;

			} else {

				col = &color_map_linear2[(*base_sorted) << 2];
				speccol = &spec_map_linear2[(*base_sorted) << 2];
				
				++base_sorted;

				shade = *((unsigned char*) base_sorted);
				specshade = *((unsigned char*) base_sorted + 1);
				
				result = 0;
				
				// red
				e = (*col) * shade;
				e += (*speccol) * specshade;
				if (e & 0x010000)
					e = 0x0FF00;			// truncation.
				e &= 0x0FF00;
				result |= e;
				result <<= 8;
				
				// green
				++col;
				++speccol;
				e = (*col) * shade;
				e += (*speccol) * specshade;
				if (e & 0x010000)
					e = 0x0FF00;			// truncation.
				e &= 0x0FF00;
				result |= e;
				
				// blue
				++col;
				++speccol;
				e = (*col) * shade;
				e += (*speccol) * specshade;
				if (e & 0x010000)
					e = 0x0FF00;			// truncation.
				e &= 0x0FF00;
				e >>= 8;
				result |= e;

				*imageptr = result;

				++imageptr;
				++base_sorted;
			}
		#endif
			

		}

		int memjump = image_size - jmax[i] - 1;
		imageptr += memjump;

		base_sorted += 2*memjump;
	}

	// blit can convert color depths (I think?)
	blit(image32bit, tmp, 0, 0, 0, 0, image_size, image_size);
}
	

void Planet3D::animate( Frame* space )
{
	 

	animate_pre();

	dummy->animate(pos, 0, space);
}





Planet3D::~Planet3D()
{
	delete dummy;

	destroy_bitmap(image32bit);


	delete base_map_linear;
	delete base_shade_linear;
	delete base_map_and_shade_resorted;
	delete color_map_linear;
}
