#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "../ais.h"
#include "../id.h"

#include "../frame.h"
#include "../gui.h"

#include "mnet1.h"
#include "mview.h" //remove this
#include "mfleet.h"



/* 
adding lag handlers to mcontrol.cpp
They're not finished yet
they're in mcontrol.cpp because that's really the 
only thing likely to use them
*/
/*
#include "mlog.h"

class SimpleLagHandler : public Presence {
	public:
	virtual void preinit();
	virtual void init ( Game *game, int channel, int size, void *default_value );
	~SimpleLagHandler();
	virtual void change_latency ( int new_lag_frames ) ;
	virtual void post_item ( void *item );
	virtual void recv_item ( void *space );
	virtual void predict_item (int frames, void *space );
	void *data;
	void *default_value;
	unsigned int *transmit_bits;
	Log *log;
	int channel;
	short int item_size;
	short int lag_frames;
	};

void SimpleLagHandler::preinit() {
	data = NULL;
	default_value = NULL;
	log = NULL;
	item_size = 0;
	lag_frames = 0;
}
void SimpleLagHandler::init ( Game *game, int channel, int size, void *default_value ) {
	this->log = game->log;
	this->lag_frames = game->lag_frames;
	this->channel = channel + Game::_channel_buffered;
	this->item_size = size;
	this->default_value = malloc( item_size );
	memcpy(this->default_value, default_value, item_size);
	data = malloc ( item_size * lag_frames );
	return;
}



/*
	int lag = game->lag_frames;
	if (sent & 1) {
		source->recieve();
		}
	if (source && source->exists()) {
		if (send()) sent |= (1 << lag);
		}
	else source = NULL;
	sent = sent << 1;
	if (!source && (sent == 0)) die();


	if (already == lag) {
		game->log_short(channel + Game::_channel_buffered, keys);
		oldkeys = keys;
		}
	else if (already < lag) {
		keys = intel_ordering_short(keys);
		game->log->buffer(channel + Game::_channel_buffered, &keys, sizeof(KeyCode));
		keys = oldkeys;
		already += 1;
		}
	else {
		game->log->unbuffer(channel + Game::_channel_buffered, &oldkeys, sizeof(KeyCode));
		oldkeys = intel_ordering_short(oldkeys);
		already -= 1;
		}
*/









enum {
	ai_index_none = 0,
	ai_index_human,
	ai_index_moron,
	ai_index_wussie,
	ai_index_vegetable,
	ai_index_end
};

const char num_controls = 4;
static char *gcc_sucks_dick[num_controls + 2] = 
		{
	"none", 
	"Human", 
	"MoronBot", 
	"WussieBot", 
	"VegetableBot", 
	NULL};
char **control_name = gcc_sucks_dick;



void animate_target(Frame *frame, SpaceLocation *t, int dx, int dy, int r, int c) {
	STACKTRACE
/*	double x, y;
	r = int( r * space_zoom/2);
	x = t->normal_x() - space_x;
	while (x < -X_MAX/2) x += X_MAX;
	x *= space_zoom;
	x += dx;
	if ((x > -r) && (x < (frame->surface->w+r))) {
		y = t->normal_y() - space_y;
		while (y < -Y_MAX/2) y += Y_MAX;
		y *= space_zoom;
		y += dy;
		if ((y > -r) && (y < (frame->surface->h+r))) {
			r /= 2;
			vline (frame->surface, (int)x, (int)(y)-r*2, (int)(y)-r, c);
			vline (frame->surface, (int)x, (int)(y)+r*2, (int)(y)+r, c);
			hline (frame->surface, (int)(x)-r*2, (int)y, (int)(x)-r, c);
			hline (frame->surface, (int)(x)+r*2, (int)y, (int)(x)+r, c);
			circle (frame->surface, (int)x, int(y), (r*3)>>1, c);
			frame->add_box((int)(x-r*2), (int)(y-r*2), (int)(r*4)+1, (int)(r*4)+1);
			}
		}*/
	return;
	}





int control2number(const char *name) {STACKTRACE
	if (!name) return 0;
	for (int i = 0; i < num_controls+1; i += 1) {
		if (!strcmp(name, control_name[i])) return i;
		}
	return 0;
	}

Control *getController(const char *type, const char *name, int channel) {STACKTRACE
	switch (control2number(type)) {
		case  ai_index_human:     return new ControlHuman(name, channel);
		case  ai_index_moron:     return new ControlMoron(name, channel);
		case  ai_index_wussie:    return new ControlWussie(name, channel);
		case  ai_index_vegetable: return new ControlVegetable(name, channel);
		}
	return NULL;
	}


int Control::rand() {
	if (channel == Game::channel_none) return random();
	return (::rand() ^ ((::rand() << 12) + (::rand() <<24))) & 0x7fffffff;
	}
void Control::setup() {}
void Control::select_ship(Ship* ship_pointer, const char* ship_name) {STACKTRACE
	ship = ship_pointer;
	if (ship) {
		ship->control = this;
		if ((channel != Game::channel_none) && (already != 0) && (already != game->lag_frames)) 
			tw_error ("Control - known error");
		}
	target_stuff() ;
	return;
	}
void Control::load(const char* inifile, const char* inisection) {
	return;
	}
void Control::save(const char* inifile, const char* inisection) {
	return;
	}
SpaceLocation *Control::get_focus() {STACKTRACE
	if (ship) return ship->get_focus();
	else return NULL;
	}


#define SELECT_DIALOG_LIST      0
#define SELECT_DIALOG_TITLE     1
#define SELECT_DIALOG_SHIP      2
#define SELECT_DIALOG_RANDOM    3
#define SELECT_DIALOG_ARANDOM   4
char selectPlayer[18];
char selectTitleString[100];

int my_list_proc( int msg, DIALOG* d, int c );
int my_bitmap_proc( int msg, DIALOG* d, int c ){
	if( msg == MSG_END && d->dp ){
		destroy_bitmap( (BITMAP*)d->dp );
		d->dp = NULL;
	}

	if( msg != MSG_DRAW || d->dp ) return d_bitmap_proc( msg, d, c );
	return D_O_K;
}

static DIALOG selectDialog[] = {
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
	{ my_list_proc,      5,     5,   280,  400,  255,  0,    0,    D_EXIT,  0,    0,    fleetListboxGetter, NULL, NULL },
	{ d_textbox_proc,    300,  10,   240,  80,   255,  0,    0,    0,       0,    0,    selectTitleString, NULL, NULL },
	{ d_button_proc,     330, 120,   180,  40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Select ship", NULL, NULL },
	{ d_button_proc,     330, 180,   180,  40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Random selection", NULL, NULL },
	{ d_button_proc,     330, 240,   180,  40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Always random", NULL, NULL },
	{ my_bitmap_proc,    388, 300,   64,  100,   255,  0,    0,    D_EXIT,  0,    0,    NULL, NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
	};

int my_list_proc( int msg, DIALOG* d, int c ){
	int old_d1 = d->d1;
	Fleet *fleet = (Fleet*)d->dp3;
	int ret = d_list_proc2( msg, d, c );
	if( d->d1 != old_d1 || msg == MSG_START ){
		ShipType* type = fleet->ship[d->d1];

		BITMAP* panel = NULL;
		DATAFILE* data = load_datafile_object( type->data->file, "SHIP_P00_PCX" );

		if( data ){
			BITMAP* bmp = (BITMAP*)data->dat;
			panel = create_bitmap_ex( bitmap_color_depth(screen), bmp->w, bmp->h );
			blit( bmp, panel, 0, 0, 0, 0, bmp->w, bmp->h );
			unload_datafile_object( data );
			data = load_datafile_object( type->data->file, "SHIP_P01_PCX" );
			bmp = (BITMAP*)data->dat;
			blit( bmp, panel, 0, 0, 4, 65, bmp->w, bmp->h );
			unload_datafile_object( data );
			color_correct_bitmap( panel, 0 );
		}

		if( selectDialog[5].dp ) destroy_bitmap( (BITMAP*)selectDialog[5].dp );
		selectDialog[5].dp = panel;
		scare_mouse();
		SEND_MESSAGE( &selectDialog[5], MSG_DRAW, 0 );
		unscare_mouse();
	}
	return ret;
}
int Control::choose_ship(VideoWindow *window, char * prompt, Fleet *fleet) {STACKTRACE
	int ret = -1, slot = 0;
	if (fleet->size == 0) tw_error ("Empty fleet! (prompt:%s)", prompt);
	selectDialog[SELECT_DIALOG_LIST].dp3 = fleet;
	sprintf(selectTitleString, "%s", prompt);
	slot = -1;
	if (!always_random) {
		while (key[KEY_ENTER] || key[KEY_SPACE]) poll_keyboard();
		ret = tw_do_dialog(window, selectDialog, SELECT_DIALOG_LIST);
		}
	if ((ret == SELECT_DIALOG_SHIP) || (ret == SELECT_DIALOG_LIST))
		slot = selectDialog[SELECT_DIALOG_LIST].d1;
	if ((ret == SELECT_DIALOG_ARANDOM) || (ret == -1)) always_random = 1;
	return(slot);
	}
void Control::set_target(int i) {STACKTRACE
	if (i >= game->num_targets) tw_error("oscar hamburger!!!!!!!!!");
	if (i == -1) {
		index = i;
		target = NULL;
		return;
		}
	if (!valid_target(game->target[i])) tw_error("oscer hambuger");
	index = i;
	target = game->target[index];
	return;
	}
void Control::target_stuff() {STACKTRACE
	if (index == -1) {
		if (game->num_targets) {
			index = random() % game->num_targets;
			target = game->target[index];
			goto validate;
			}
		else {
			goto done;
			}
		}
blah:
	if (index >= game->num_targets) {
		if (game->num_targets) {
			index -= 1;
			if (index < 0) index = 0;
			goto blah;
			}
		else {
			index = -1;
			target = NULL;
			goto change;
			}
		}
	if (target == game->target[index]) {
		goto done;
		}
	else goto search;
search:
	int o;
	o = index;
	for (index = 0; index < game->num_targets; index += 1) {
		if (game->target[index] == target) {
			goto done;
			}
		}
	index = o;
validate:
	if (!ship) {
		goto done;
		}
	int start;
	start = index;
	while (!valid_target(game->target[index])) {
		index = (index + 1) % game->num_targets;
		if (index == start) {
			index = -1;
			target = NULL;
			goto change;
			}
		}
	target = game->target[index];
change:
done:
	return;
	}
void Control::calculate() {STACKTRACE

	target_stuff();

	if (ship) {
		if (!ship->exists()) {
			//message.print(5000, 12, "Ship died in frame %d", game->frame_number);
			select_ship( NULL, NULL);
			}
		else keys = think();
		}
	
	if (!ship) keys = 0;

	//THIS WILL CRASH
	//IF A NETWORKED SHIP IS DESTROYED IN THE FIRST FEW FRAMES OF IT'S LIFE
	//!!!!!!!!
	if (channel != Game::channel_none) {
		int lag = game->lag_frames;
		if (!ship) {
			lag = 0;
			if (!already) {
				if (temporary) this->die();
				return;
				}
			}
		_prediction_keys[_prediction_keys_index] = keys;
		_prediction_keys_index = (_prediction_keys_index + 1) & (_prediction_keys_size-1);
		if (already == lag) {
			game->log_short(channel + Game::_channel_buffered, keys);
			}
		else if (already < lag) {
			keys = intel_ordering_short(keys);
			game->log->buffer(channel + Game::_channel_buffered, &keys, sizeof(KeyCode));
			keys = intel_ordering_short(keys);
			already += 1;
			}
		else {
			game->log->unbuffer(channel + Game::_channel_buffered, &keys, sizeof(KeyCode));
			keys = intel_ordering_short(keys);
			already -= 1;
			}
		}

	return;
	}
int Control::think() {STACKTRACE
	return 0;
	}
char *Control::getDescription() {STACKTRACE
	return iname;
	}
Control::Control(const char *name, int _channel) : ship(NULL), 
	target(NULL), index(-1), _prediction_keys(NULL), 
	always_random(0), channel(_channel), 
	already(0), temporary(false), target_sign_color(255)
	{STACKTRACE
	id |= ID_CONTROL;
	attributes |= ATTRIB_SYNCHED;
	if (channel != Game::channel_none) {
		attributes |= ATTRIB_LOGGED;
		_prediction_keys = new KeyCode[_prediction_keys_size];
		_prediction_keys_index = 0;
	}
	iname = strdup(name);
	}
Control::~Control() { STACKTRACE
	if (_prediction_keys) delete _prediction_keys;
	}
bool Control::valid_target(SpaceObject *t) {
	if (!ship) tw_error("Control::valid_target - !ship");
	if (t->sameTeam(ship)) return false;
	if (!t->exists()) return false;
	return true;
	}
void Control::animate(Frame *space) {STACKTRACE
	if (!ship) return;
	if (!target || target->isInvisible()) return;
	if (!(attributes & ATTRIB_ACTIVE_FOCUS)) return;
	if (game->num_targets < 3) return;
	if (target_sign_color == 255) return;

	int i = target_sign_color;
	int col = target_sign_color;
	animate_target(space, target, (i%3)*2-2, ((i/3)%3)*2-2, 140 + i, pallete_color[col]);
	}
/*void Control::add_target(SpaceObject *killit) {
	if (!killit) return;
	if (!killit->exists()) return;
	for (int i = 0; i < num_targets; i += 1) {
		if (target[i] == killit) return;
		}
	num_targets += 1;
	target = (SpaceObject**) realloc(target, sizeof(SpaceObject *) * num_targets);
	target[num_targets-1] = killit;
	if (active_target == -1) set_target(0);
	return;
	}
void Control::remove_target(SpaceObject *killit) {
	if (!killit) return;
	for (int i = 0; i < num_targets; i += 1) {
		if (target[i] == killit) {
			num_targets -= 1;
			target[i] = target[num_targets];
			target = (SpaceObject**)realloc(target, num_targets * sizeof(SpaceObject *));
			if (active_target == i) {
				if (num_targets) set_target(random() % num_targets);
				else set_target(-1);
				return;
				}
			else if (active_target == num_targets) active_target = i;
			}
		}
	return;
	}*/
