#ifndef __GAMEX_SPACE_BODY__
#define __GAMEX_SPACE_BODY__


// ellips of planet orbit
void drawellips(Frame *f, Vector2 center, double R, double b, int col);

// planet shading
void shadowpaintcircle(SpaceSprite *spr, double fi_s);

const int ID_SOLAR_BODY = 0x098a398f5;

class SolarBody : public SpaceObject
{
public:
	Vector2 stayhere;
	double sunangle;
	SolarBody(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite, Vector2 sunpos,
					int bodynum,
					Vector2 Ec, double ER, double Eb, int Ecol);
	virtual void animate(Frame *f);
	virtual void calculate();

	int solar_body_num;

	Vector2 ellipscenter;
	double ellipsR, ellipsb;
	int ellipscol;
};


#endif
