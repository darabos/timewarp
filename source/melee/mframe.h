#ifndef __MFRAME_H__
#define __MFRAME_H__

//debuging purposes only:
extern int total_presences;

//physics context
extern int frame_time;
extern Vector2 map_size;
extern double MAX_SPEED;

extern DATAFILE *melee;

typedef unsigned int TeamCode;

struct Query;
class Planet;
class Presence;

enum {
	ATTRIB_SYNCHED =            0x00000001, 
	ATTRIB_LOGGED =             0x00000002, 
	ATTRIB_INGAME =             0x00000004, 

	ATTRIB_TARGET =             0x00000100,
	ATTRIB_FOCUS  =             0x00000200,
	ATTRIB_ACTIVE_FOCUS =       0x00000400,

	ATTRIB_NOTIFY_ON_DEATH =    0x00000800,

	ATTRIB_STANDARD_INDEX =     0x00001000,
	ATTRIB_STRICT_RECT =        0x00002000,

	ATTRIB_COLLIDE_STATIC =     0x00004000,

	//ATTRIB_BOUNCY         //currently, anything that has mass
	//ATTRIB_MASSFULL       //anything that has mass
	//ATTRIB_CREWED         //currently, only ships with green crew
	//ATTRIB_ORGANIC        //currently, only ships with green crew
	//ATTRIB_SENTIENT       //currently, only ships
	//ATTRIB_NEUTRAL        //asteroids & planets
	//ATTRIB_CONTACT_DANGER //things to avoid

	ATTRIB_LOCATION =           0x01000000, 
	ATTRIB_OBJECT =             0x02000000, 
	ATTRIB_LINE =               0x04000000,
	ATTRIB_SHOT =               0x08000000, 
	ATTRIB_SHIP =               0x10000000,

	// this hides objects from queries = important in case there are many objects (hide hotspots!!)!
	ATTRIB_UNDETECTABLE =       0x20000000

};

/*struct Listed {
	int serial;
	Presence *pointer;

	void init(Presence *p);
	void clear();
};*/


class Physics : public BaseClass {
	public:

	SpaceLocation **quadrant;
	friend struct Query;

	/*Listed *listed;
	int num_listed, max_listed;
	void _list(Presence *p);
	int last_serial;
	int new_serial();
	int last_unsynched_serial;

	int new_unsynched_serial();*/

	int num_items, max_items;
	SpaceLocation **item;
	int num_presences, max_presences;
	Presence **presence;
	void add(SpaceLocation *p);
	void add(Presence *p);
	bool remove(SpaceLocation *o);
	bool remove(Presence *p) ;

	TeamCode last_team;
	unsigned int last_ship;

	protected:
	virtual void collide();

	public:
	virtual unsigned int new_ship();
	virtual TeamCode new_team();
	unsigned int get_code(unsigned int ship, TeamCode team) ;
	virtual void merge_teams ( TeamCode team1, TeamCode team2) ; //causes team1 and team2 to be the same team (changes members of team2 to being members of team1 instead)
	virtual void switch_team ( unsigned int ship,  TeamCode team ) ; //causes ship to join team.  if ship was already on an old team, ship betrays that team.  

	int frame_number; //the total number of frames that have passed so far
	int frame_time;
	int game_time;
	double max_speed;
	Vector2 size;
	double turbo;

	virtual void prepare();

	virtual void calculate();
	virtual void animate(Frame *frame);
	virtual void animate_predict(Frame *frame, int time);

	/*Presence *find_serial(int serial);
	int _find_serial(int serial);*/

	virtual void play_sound (SAMPLE *sample, SpaceLocation *source, int vol = 255, int freq = 1000);
	virtual void play_sound2 (SAMPLE *sample, SpaceLocation *source, int vol = 255, int freq = 1000);

	virtual void preinit();
	virtual void init();
	virtual ~Physics();
	virtual void destroy_all();

	virtual int checksum();
	virtual void dump_state ( const char *file_name );

	// moved ROB - from mgame to here.
	bool friendly_fire;
	double shot_relativity;

	// to init parameters of space-objects...
	virtual void log_file (const char *fname);

	// to handle the death of objects in the game ...
	virtual void object_died(SpaceObject *who, SpaceLocation *source);
	virtual void ship_died(Ship *who, SpaceLocation *source);
};


class Presence : public BaseClass { 
	friend class Physics;
	public:

	int    id;            // id code, indicates what type it is
	int attributes;       // describes it
	signed char state;    // may be set to any positive value if alive, or 0 to kill it
	//private: int _serial; // unique reference number
	private: int _depth;  // determines rendering order ; read/write with set_depth, get_depth
	public:
	void set_depth(double d);
	double get_depth();
	friend int compare_depth (const void *_a, const void *_b) ;

	inline void add(SpaceLocation *o) {physics->add(o);}
	inline void add(Presence *p) {physics->add(p);}

	virtual void animate(Frame *space); //displays ? on screen (NOT permitted to affect game physics in any way)
	virtual void animate_predict(Frame *space, int time); //like animate, but attempts to predict the future
	virtual void calculate(); //advance the item frame_time milliseconds in time
	inline bool exists() {return state > 0;};  //returns 0 if dead, non-zero if alive
	Presence();
	virtual ~Presence();  // called when a presence is deallocated
	virtual SpaceLocation *get_focus();
//	void play_sound (SAMPLE *sample, int vol = 255, int freq = 1000); //plays a sound
//	void play_sound2 (SAMPLE *sample, int vol = 255, int freq = 1000);//like play_sound, only stops the previous sound

	virtual bool die(); // calling this kills a presence (returns true on success)

	bool isLocation() const ;  //returns true if it is the specified engine-defined type
	bool isLine() const ;
	bool isObject() const ;
	bool isAsteroid() const ;
	bool isPlanet() const ;
	bool isShip() const ;
	bool isShot() const;
	bool isSynched() const;

	//inline int get_serial() const {return _serial;}

};

class SpaceLocation : public Presence { // base class for all items in game
	friend class Physics;
	friend struct Query;

protected: public: //aught to be protected, but we're lazy
	Vector2 pos;
	union {
		SpaceObject   *qnexto;
		SpaceLine     *qnextl;
		SpaceLocation *qnext;
	};
	public:
	int    layer;       // effects collisions & queries... (will be phased out eventually)
protected: public://aught to be protected, but we're lazy
	Vector2 vel;
	double angle;        // the angle it's facing (sometimes not very meaningfull)


	enum { 
			team_bits = 14, ship_bits =  18, 
			team_shift = 0, ship_shift = 14, 
			team_mask = ((1<<team_bits)-1) << team_shift,
			ship_mask = ((1<<ship_bits)-1) << ship_shift
		};
	unsigned int ally_flag;

	public:

	Ship *ship;          // the ship it's associated with
	ShipData *data;      // the data module it depends upon
	SpaceObject *target; // it's target, if it has one

	inline  bool sameShip (const SpaceLocation *other) {return ally_flag == other->ally_flag;}
	virtual bool sameTeam (const SpaceLocation *other) const;
	TeamCode get_team() const;
	void set_team(TeamCode k);

	virtual void ship_died();
	virtual void target_died();
	virtual bool change_owner(SpaceLocation *new_owner);


	double damage_factor;         // the damage this item inflicts

	int    collide_flag_anyone;   // mask for which layers it can collide with for non-allies
	int    collide_flag_sameteam; // mask for which layers it can collide with for allies
	int    collide_flag_sameship; // mask for which layers it can collide with objects associated with the same ship



	int damage(SpaceLocation *who, double normal, double direct = 0) { return who->handle_damage(this, normal, direct); }
	/*handle_damage returns: (this is a lie)
		-1 = benefitted from damage
		 0 = ignored damage
		 1 = took damage normally
		 2 = reduced damage
	*/
	virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
	virtual int handle_fuel_sap(SpaceLocation *source, double normal);
	virtual double handle_speed_loss(SpaceLocation *source, double normal);

	virtual void change_vel(Vector2 dvel);
	virtual void change_pos(Vector2 dpos);
	virtual void change_pos(double scale);

	SpaceLocation(SpaceLocation *creator, Vector2 lpos, double langle);
	virtual void death();      // called after an item is killed
	virtual ~SpaceLocation();  // called when an item is deallocated

	double get_angle() const;        // various public functions for reading otherwise protected data
	Vector2 get_vel() const {return vel;}

	virtual double get_angle_ex() const; // stupid helper for camera

	int getID() const;

	virtual SpaceLocation *get_focus();

	Vector2 normal_pos() const;          // returns the x,y coordinates, and normalizes it (0 <= pos.x < map_size.x), (0 <= pos.y < map_size.y)
	Vector2 nearest_pos(SpaceLocation *l) const;
	double distance(SpaceLocation *l);
	double trajectory_angle(SpaceLocation *l);

	virtual double isProtected() const;  // returns 0 normally, or a positive number if shielded
	virtual double isInvisible() const;  // returns 0 normally, or a positive number if cloaked
	
	// this is used in queries, to see if it's detectable and should be examined.
	virtual inline bool detectable();

	Planet *nearest_planet(); //returns the nearest planet, or NULL if no planets are nearby

	virtual int canCollide(SpaceLocation *other); // returns 0 if collision impossible

	virtual int translate(Vector2 rel_pos); //moves an object (returns non-zero on success)
	int translate(double rel_x, double rel_y) {return translate(Vector2(rel_x,rel_y));}
	void _accelerate(double angle, double vel, double max_speed=MAX_SPEED); //accelerates an object by vel at angle, to a maximum of max_speed
	void _accelerate(Vector2 delta_v, double max_speed=MAX_SPEED); //changes an objects velocity by delta_v, to a maximum of max_speed
	virtual int accelerate(SpaceLocation *source, double angle, double vel, double max_speed=MAX_SPEED); //accelerates an object by vel at angle, to a maximum of max_speed
	virtual int accelerate(SpaceLocation *source, Vector2 delta_v, double max_speed=MAX_SPEED); //changes an objects velocity by delta_v, to a maximum of max_speed
	virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, double max_speed) ; //accelerates an object, with gravity whip
	void match_velocity(SpaceLocation *other) {vel = other->vel;}

	virtual void animate(Frame *space); //displays an object on screen (NOT permitted to affect game physics in any way)
	virtual void animate_predict(Frame *space, int time); //like animate, but attempts to predict the future
	virtual void calculate(); //advance the item frame_time milliseconds in time

	void play_sound (SAMPLE *sample, int vol = 256, int freq = 1000); //plays a sound
	void play_sound2 (SAMPLE *sample, int vol = 256, int freq = 1000);//like play_sound, only stops the previous sound
};

class SpaceObject : public SpaceLocation {
	public:
	Vector2 size;      //size of sprite
	double  mass; //mass of object
	bool isblockingweapons;	// this object blocks weaponry (shots)
	protected:
	SpaceSprite *sprite; //the pictures that this object looks like
	int          sprite_index; //which one of those pictures is active at the moment

	public:
	SpaceObject(SpaceLocation *creator, Vector2 opos, double oangle, 
			SpaceSprite *osprite);
	virtual void death();      // called after an item is killed

	SpaceSprite *get_sprite() const {return sprite;}
	int get_sprite_index() const {return sprite_index;}

	Vector2 get_size() const {return size;}

	virtual void animate(Frame *space);
	virtual void calculate();
	virtual void collide(SpaceObject *other);
	virtual double collide_ray(Vector2 lpos1, Vector2 lpos2, double llength);

	virtual void inflict_damage(SpaceObject *other);

	virtual void set_sprite ( SpaceSprite *sprite );
};


class SpaceLine : public SpaceLocation {
	protected:
	double length;
	int    color;

	public:
	SpaceLine(SpaceLocation *creator, Vector2 lpos, double langle, 
		double llength, int lcolor);

	Vector2 edge() const;
	double edge_x() const;
	double edge_y() const;
	double get_length() const;

	virtual void animate(Frame *space);
	virtual void collide(SpaceObject *o);
	virtual void inflict_damage(SpaceObject *other);
};

struct Query {
	private:
	int layers;
	SpaceLocation *target;
	int qx_min, qx_max;
	int qy_min, qy_max;
	int qx, qy;
	Vector2 target_pos;
	double range_sqr;
	void next_quadrant ();
	public:
	union {
		SpaceObject   *currento;
		SpaceLine     *currentl;
		SpaceLocation *current;
	};
	void begin (SpaceLocation *target, int layers, double range);
	void begin (SpaceLocation *target, Vector2 center, int layers, double range);
	void next ();
	void end();
	private:
	bool current_invalid() {
		if (!(bit(current->layer) & layers) || (current == target) || !current->exists()) return true;
		if (magnitude_sqr(min_delta(target_pos, current->normal_pos())) > range_sqr) return true;
		return false;
	}
};

inline Uint64 REQUIRE_ATTRIBUTES(Uint32 a) {return a | (Uint64(a) << 32);}
inline Uint64 PROHIBIT_ATTRIBUTES(Uint32 a) {return Uint64(a) << 32;}

struct Query2 {
	private:
	int attributes_desired;
	int attributes_mask;
	SpaceLocation *target;
	int qx_min, qx_max;
	int qy_min, qy_max;
	int qx, qy;
	Vector2 target_pos;
	double range_sqr;
	void next_quadrant ();
	public:
	union {
		SpaceObject   *currento;
		SpaceLine     *currentl;
		SpaceLocation *current;
	};
	void begin (SpaceLocation *target, Uint64 attribute_filter, double range);
	void begin (SpaceLocation *target, Vector2 center, Uint64 attribute_filter, double range);
	void next ();
	void end();
	private:
	bool current_invalid() {
		if (((current->attributes & attributes_mask) != attributes_desired) || (current == target) || !current->exists()) return true;
		if (magnitude_sqr(min_delta(target_pos, current->normal_pos())) > range_sqr) return true;
		return false;
	}
};

#endif // __MFRAME_H__
