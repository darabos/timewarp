
#include "../util/t_rarray.h"

struct MouseState;
struct TrugOrder;
class TrugControl;
class TrugView;
class TrugGUI;
class TrugPlayer;
class TrugGame;


struct TrugOrder {
	int type;
	int x, y, z;
};

class TrugControl : public ControlWussie {
	public:
	TrugControl ( const char *name, int channel );
	void next_order();
	int think();
	TrugOrder order;
	TrugPlayer *player;
};

struct MouseState {
	int x, y, z;
	int buttons;
};

class TrugWidget : public Presence {
public:
	int channel;
	TrugWidget ( );
	virtual SpaceLocation *get_focus();
	virtual void next_focus();
	virtual void gui_stuff ( Frame *frame, VideoWindow *region, int frame_type, int region_type );
	virtual void animate ( Frame *frame );
};

class TrugShipList : public TrugWidget {
	virtual void gui_stuff ( Frame *frame, VideoWindow *region, int frame_type, int region_type );
};

class TrugGUI : public Presence {
public:
	class TrugPlayer *player;
	ShipPanel *panel;
	VideoWindow *window;
	VideoWindow *subwindow;
	virtual void calculate ( );
	virtual void animate ( Frame *space );
	virtual bool handle_key ( int key );
	virtual void handle_mouse ( MouseState *now );
	MouseState prev;
	TrugGUI ( TrugPlayer *p );
};

extern TrugGUI *truggy;

class TrugPlayer : public Presence {
	public:
	TrugPlayer (int channel);
	virtual ~TrugPlayer ();
	int channel;
	Control *manual;
	TrugControl *overriden;
	TeamCode team;
	virtual void calculate();
	R_Array<int> ship_roots;
	R_Array<int> ship_codes;
};

enum {
	ORDER_FREELANCE = 0,
	ORDER_PLAYDEAD  ,
	ORDER_ATTACK    ,     //x = target serial#
	ORDER_MOVE      ,     //x = x coordinate, y = y coordinate
	ORDER_GUARD     ,     //x = x coorinate, y = y coordinate
	ORDER_OVERRIDE  ,
	ORDER_DEFAULT = ORDER_PLAYDEAD
};

class ViewTrug : public View {
	double f;
	double max, min;
	public:
	virtual void calculate(Game *game);
	virtual void init(View *old);
};

class TrugOrderEvent : public GameEvent
{
	int player;
	int ship;
	TrugOrder order;
};

class TrugEvent : public GameEvent
{
	int serial;
};

class TrugGame : public Game {
	public:
	//over-riding functions from Physics
	virtual void play_sound (SAMPLE *sample, SpaceLocation *source, int vol = 255, int freq = 1000);

	//over-riding functions from Game
	virtual void preinit();
	virtual void prepare();
	virtual void init(Log *_log) ;
	virtual void calculate ();
	virtual ~TrugGame();
	virtual bool handle_key(int k);

	//somewhere in between
	virtual Ship *create_ship(const char *id, TrugPlayer *c, Vector2 pos, double angle) ;

	virtual TrugPlayer* add_player ( int channel );


	R_Ref_Array<TrugPlayer*> players;

	TrugGUI *gui;
};
