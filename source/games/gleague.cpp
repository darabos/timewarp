#include <stdio.h>
#include <allegro.h>

#include "../melee.h"
#include "../id.h"
#include "../frame.h"
#include "../scp.h"

#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mship.h"
#include "../melee/mshot.h"
#include "../melee/mview.h"
#include "../melee/mcbodies.h"

#define LEAGUE_BALL_MASS         3
#define LEAGUE_BALL_KICK_SPEED   0.7
#define LEAGUE_BALL_SLOWDOWN     0.0005
#define LEAGUE_BALL_MIN_SPEED    0.005
#define LEAGUE_BALL_ALLY_FLAG    2000
#define LEAGUE_GOAL_LINE_DAMAGE  0.0005
#define LEAGUE_GOAL_LINE_REGEN   0.0005
#define LEAGUE_TEAM_COLOR_1      12
#define LEAGUE_TEAM_COLOR_2      10
#define LEAGUE_BALL_APAMUTI      50
#define LEAGUE_BALL_NOFIRE_DELAY 500
#define LEAGUE_BOUNDARY_COLOR    15
#define LEAGUE_BORDER            50

#define PANEL_WIDTH             64
#define PANEL_HEIGHT            80

#define ID_BALL                 0x10000003

class Ball : public SpaceObject {
	Ship*  latched;
	double rel_angle;
	double old_angle;
	double rel_dist;

	bool assembling;
	bool disassembling;
	int  step;

	int apamuti;
public:
	Ball( Vector2 opos, double oangle, SpaceSprite* osprite );
	virtual void inflict_damage( SpaceObject* other );
	virtual void calculate();
	virtual void death();
	virtual void kick();
	virtual void reassemble();
};

class GoalLine : public Laser {
	double _damage;
	double _length;
	double residualDamage;
public:
	GoalLine( Vector2 opos, double oangle, double olength, double odamage,
		int ocolor, double ores = 0.0 );
	virtual void calculate();
	virtual void inflict_damage( SpaceObject* other );
};

class FixedObject : public SpaceObject {
	Vector2 fixed_pos;
public:
	FixedObject( SpaceLocation* creator, Vector2 opos, double oangle,
		SpaceSprite* osprite );
	virtual void calculate();
	virtual void collide( SpaceObject* other );
};

class DisableFire : public SpaceLocation {
	int   time;
public:
	DisableFire( Ship* oship, int otime );
	virtual void animate( Frame* space );
	virtual void calculate();
};

class Boundary : public SpaceLine {
public:
	Boundary( Vector2 point1, Vector2 point2 );
	virtual void collide( SpaceObject* other );
};

class LeagueGame : public NormalGame {
	DATAFILE* old_melee;
	Ball* ball;

	public:
	int score[2];
	virtual void calculate();
	virtual void init(Log *_log);
	virtual void set_resolution (int screen_x, int screen_y);
	virtual void init_objects();
	virtual void animate(Frame *frame);
	virtual      ~LeagueGame();
};

void LeagueGame::init_objects() {
	STACKTRACE

	add( new Stars() );
	add( new Boundary( Vector2(LEAGUE_BORDER, LEAGUE_BORDER), Vector2(size.x/2, LEAGUE_BORDER) ));
	add( new Boundary( Vector2(size.x/2, LEAGUE_BORDER), Vector2(size.x-LEAGUE_BORDER, LEAGUE_BORDER) ));
	add( new Boundary( Vector2(size.x-LEAGUE_BORDER, LEAGUE_BORDER), Vector2(size.x-LEAGUE_BORDER, size.y/2) ));
	add( new Boundary( Vector2(size.x-LEAGUE_BORDER, size.y/2), Vector2(size.x-LEAGUE_BORDER, size.y-LEAGUE_BORDER) ));
	add( new Boundary( Vector2(size.x-LEAGUE_BORDER, size.y-LEAGUE_BORDER), Vector2(size.x/2, size.y-LEAGUE_BORDER) ));
	add( new Boundary( Vector2(size.x/2, size.y-LEAGUE_BORDER), Vector2(LEAGUE_BORDER, size.y-LEAGUE_BORDER) ));
	add( new Boundary( Vector2(LEAGUE_BORDER, size.y-LEAGUE_BORDER), Vector2(LEAGUE_BORDER, size.y/2) ));
	add( new Boundary( Vector2(LEAGUE_BORDER, size.y/2), Vector2(LEAGUE_BORDER, LEAGUE_BORDER) ));

	ball = new Ball( size/2, 0, asteroidSprite );
	add( ball );
	GoalLine* gl = new GoalLine( Vector2(2 * size.x / 5, size.y / 10), 0, size.x / 5, LEAGUE_GOAL_LINE_DAMAGE, pallete_color[LEAGUE_TEAM_COLOR_1] );
	add( gl );
	gl = new GoalLine( Vector2(3 * size.x / 5, size.y / 10), PI, size.x / 5, LEAGUE_GOAL_LINE_DAMAGE, pallete_color[LEAGUE_TEAM_COLOR_1], 0.5 );
	add( gl );
	gl = new GoalLine( Vector2(2 * size.x / 5, 9 * size.y / 10), 0, size.x / 5, LEAGUE_GOAL_LINE_DAMAGE, pallete_color[LEAGUE_TEAM_COLOR_2] );
	add( gl );
	gl = new GoalLine( Vector2(3 * size.x / 5, 9 * size.y / 10), PI, size.x / 5, LEAGUE_GOAL_LINE_DAMAGE, pallete_color[LEAGUE_TEAM_COLOR_2], 0.5 );
	add( gl );
	FixedObject* post = new FixedObject( NULL, Vector2(2 * size.x / 5 - planetSprite->size().x / 2, size.y / 10), 0, planetSprite );
	add( post );
	post = new FixedObject( NULL, Vector2(3 * size.x / 5 + planetSprite->size().x / 2, size.y / 10), 0, planetSprite );
	add( post );
	post = new FixedObject( NULL, Vector2(2 * size.x / 5 - planetSprite->size().x / 2, 9 * size.y / 10), 0, planetSprite );
	add( post );
	post = new FixedObject( NULL, Vector2(3 * size.x / 5 + planetSprite->width() / 2, 9 * size.y / 10), 0, planetSprite );
	add( post );

	size *= 2;
	prepare();
}

void LeagueGame::init( Log *_log ){
	STACKTRACE

	score[0] = score[1] = 0;
	// changed GEO. The data-file of this game is outdated. It works with
	// the default melee data file.
//	old_melee = melee;
//	melee = load_datafile( "league.dat" );
//	if( !melee ) error( "Error loading league data\n" );
	NormalGame::init( _log );
	turbo *= 1.414;
	player_team[2] = player_team[0];
}

LeagueGame::~LeagueGame(){
	// change GEO
//	if( melee && melee != old_melee ) unload_datafile( melee );
//	melee = old_melee;
}

void LeagueGame::set_resolution( int screen_x, int screen_y ){
	NormalGame::set_resolution( screen_x, screen_y );
	view->frame->set_background( 0, 32, 0 );
}

void LeagueGame::calculate(){
	NormalGame::calculate();
}

void LeagueGame::animate( Frame* frame ){
	STACKTRACE

	NormalGame::animate( frame );
	panelSprite->draw( 0, Vector2(PANEL_WIDTH, PANEL_HEIGHT), 0, frame );
	int colon_w = text_length( font, ":" );
	textprintf( frame->surface, font, (PANEL_WIDTH - colon_w) / 2, 45, pallete_color[15], ":" );
	char buf[100];
	sprintf( buf, "%d", score[0] );
	int team_1_w = text_length( font, buf );
	textprintf( frame->surface, font, (PANEL_WIDTH - colon_w) / 2 - team_1_w, 45, pallete_color[LEAGUE_TEAM_COLOR_2], buf );
	sprintf( buf, "%d", score[1] );
	textprintf( frame->surface, font, (PANEL_WIDTH + colon_w) / 2, 45, pallete_color[LEAGUE_TEAM_COLOR_1], buf );
}

Ball::Ball( Vector2 opos, double oangle, SpaceSprite* osprite ):
SpaceObject( NULL, opos, oangle, osprite ),
latched( NULL ),
assembling( false ), disassembling( false ),
apamuti( 0 ){
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
	mass = LEAGUE_BALL_MASS;
	id = ID_BALL;
}

void Ball::inflict_damage( SpaceObject* other ){
	STACKTRACE

	if( !other->isShip() || apamuti ) return;
	apamuti = LEAGUE_BALL_APAMUTI;
	latched = (Ship*) other;
	rel_angle = other->trajectory_angle( this ) - other->get_angle();
	old_angle = angle - other->get_angle();
	rel_dist = distance( other );
	change_owner( other );       // are now part of the other ship
	collide_flag_sameship = 0;
}

void Ball::calculate(){
	STACKTRACE

	if( apamuti ){
		apamuti -= frame_time;
		if( apamuti < 0 ) apamuti = 0;
	}
	if( !latched ) {
		SpaceObject::calculate();
	}
	else if( !latched->exists() ){
		latched = NULL;
		change_owner( NULL );
		collide_flag_sameship = ALL_LAYERS;
		SpaceObject::calculate();
	}
	else if( latched->nextkeys & keyflag::fire ){
		game->add( new DisableFire( latched, LEAGUE_BALL_NOFIRE_DELAY ));
		kick();
		SpaceObject::calculate();
	}
	else{
		Vector2 T = unit_vector ( latched->get_angle() + rel_angle );
		pos = latched->normal_pos() + rel_dist * T;
		vel = latched->vel;
		angle = old_angle + latched->get_angle();
	}
	if( disassembling ){
		if( step <= 0 ){
			step += time_ratio;
			sprite_index++;
			if( sprite_index == ASTEROIDEXPLOSION_FRAMES ){
				sprite_index--;
				disassembling = false;
				assembling = true;
				pos = map_size / 2;
			}
		}
		else step -= frame_time;
	}
	else if( assembling ){
		if( step <= 0 ){
			step += time_ratio;
			sprite_index--;
			if( sprite_index == -1 ){
				sprite_index = 0;
				disassembling = false;
				assembling = false;
				sprite = game->asteroidSprite;
				collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
			}
		}
		else step -= frame_time;
	}
	else{
		sprite_index = get_index(angle);
	}

	double vv = magnitude_sqr(vel);
	if( vv > LEAGUE_BALL_MIN_SPEED ){
		double alpha = vel.angle();
		alpha = normalize(alpha, PI2);
		double v = sqrt( vv );
		accelerate( this, alpha, -v * LEAGUE_BALL_SLOWDOWN * frame_time, v );
	}
}

void Ball::kick(){
	STACKTRACE

	if( latched ){
		vel = LEAGUE_BALL_KICK_SPEED * unit_vector( latched->get_angle() );
		pos += vel * frame_time;
		latched = NULL;
		change_owner( NULL );
		collide_flag_sameship = ALL_LAYERS;
	}
}

void Ball::reassemble(){
	STACKTRACE

	if( latched ){
		latched = NULL;
		change_owner( NULL );
	}
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = 0;
	step = time_ratio;
	sprite_index = 0;
	sprite = game->asteroidExplosionSprite;
	assembling = false;
	disassembling = true;
	vel = 0;
}

void Ball::death(){
	// should be avoided
}

GoalLine::GoalLine( Vector2 opos, double oangle, double olength, double odamage,
	int ocolor, double ores ):
Laser( NULL, oangle, ocolor, olength, (int)odamage, 99999,
	new SpaceLocation( NULL, opos, 0 ), 0, false ),
_damage( odamage ),
_length( olength ),
residualDamage( ores ){
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
}

void GoalLine::calculate(){
	STACKTRACE

	length += _length * frame_time * LEAGUE_GOAL_LINE_REGEN;
	if( length > _length ) length = _length;
	damage_factor = (int)(_damage * (double)frame_time);
	residualDamage += (_damage * (double)frame_time) - damage_factor;
	if( residualDamage >= 1.0 ){
		residualDamage -= 1.0;
		damage_factor += 1;
	}
}

void GoalLine::inflict_damage( SpaceObject* other ){
	STACKTRACE

	Laser::inflict_damage( other );
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
	if( other->id == ID_BALL ){
		if( color == pallete_color[LEAGUE_TEAM_COLOR_1] ){
			message.out( "       GREEN TEAM SCORES" );
			((LeagueGame*)game)->score[0]++;
		}else{
			message.out( "       RED TEAM SCORES" );
			((LeagueGame*)game)->score[1]++;
		}
		((Ball*)other)->reassemble();
	}
}

FixedObject::FixedObject( SpaceLocation* creator, Vector2 opos, double oangle,
	SpaceSprite* osprite ):
SpaceObject( creator, opos, oangle, osprite ),
fixed_pos(opos) {
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
	mass = 999999;
}

void FixedObject::calculate(){
	SpaceObject::calculate();
	vel = 0;
	pos = fixed_pos;
}

void FixedObject::collide( SpaceObject* other ){
	SpaceObject::collide( other );
	vel = 0;
	pos = fixed_pos;
}

DisableFire::DisableFire( Ship* oship, int otime ):
SpaceLocation( oship, 0, 0 ), time( otime ){
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = 0;
	if( !ship ) state = 0;
	ship->nextkeys &= ~keyflag::fire;
}

void DisableFire::animate( Frame* space ){}

void DisableFire::calculate(){
	STACKTRACE

	if( !ship ){
		state = 0;
		return;
	}
	ship->nextkeys &= ~keyflag::fire;
	time -= frame_time;
	if( time <= 0 ) state = 0;
}

Boundary::Boundary( Vector2 point1, Vector2 point2 ):
SpaceLine( NULL, point1, atan3(point2-point1), distance_from( point1, point2 ),
	pallete_color[LEAGUE_BOUNDARY_COLOR] ){
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = OBJECT_LAYERS;
}

void Boundary::collide( SpaceObject* other ){
	STACKTRACE


	if((!canCollide(other)) || (!other->canCollide(this))) return;

	if( other->collide_ray( normal_pos(), 
		normal_pos() + edge(), length ) == length )
		return;

//	inflict_damage(other);

	Vector2 D;
	D = unit_vector(angle - PI/2);
	while( other->collide_ray( normal_pos(), 
		normal_pos() + edge(), length ) != length ){
		other->pos -= D;
	}
	double alpha = atan3( other->vel );
	double v = other->vel.length();
	v *= 0.9;
	other->vel = v * unit_vector( 2 * angle - alpha);
}

REGISTER_GAME( LeagueGame, "Sentient League Football" );
