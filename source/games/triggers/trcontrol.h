
#ifndef __TRCONTROL__
#define __TRCONTROL__






// a special type of "control" trigger with special functions...
class action_func_control : public action_func
{
public:
  virtual ~action_func_control(){};
	// execution of blocks is handled by this:
	action_func *current_action;

	int exec_block(action_func *first);

	int write_block(action_func *first, FILE *f);
	action_func *init_block(FILE *f);
	//int read_block(action_func *first, FILE *f);

	void add_block(action_func **funclist, char *levellist, int *N, int *level, int max, action_func *first);

};



class action_startfunction : public action_func_control
{
public:
  virtual ~action_startfunction(){};
	action_func *main_ptr;
	double timer, timeforaction;

	//action_startfunction();

	virtual void calculate();

	virtual void define();
	virtual void write(FILE *f);
	virtual void read(FILE *f);

	void read(char *filename);
	void write(char *filename);

	virtual void append(action_func *a);
	virtual void addme(action_func **funclist, char *levellist, int *N, int *level, int max);

	virtual action_func *create() {return new action_startfunction(); };

};


class action_subr : public action_func_control
{
public:
  virtual ~action_subr(){};
	action_func *block_ptr;

	virtual void calculate();

	virtual void define();
	virtual void write(FILE *f);
	virtual void read(FILE *f);
	virtual void addme(action_func **funclist, char *levellist, int *N, int *level, int max);

	virtual action_func *create() {return new action_subr(); };
};





class action_if : public action_func_control
{
public:
  virtual ~action_if(){};
	action_func *true_block_ptr, *false_block_ptr;
	int *ichoice;

	virtual void calculate();

	virtual void define();
	virtual void write(FILE *f);
	virtual void read(FILE *f);
	virtual void addme(action_func **funclist, char *levellist, int *N, int *level, int max);

	virtual action_func *create() {return new action_if(); };
};



class action_for : public action_func_control
{
public:
  virtual ~action_for(){};
	action_func *block_ptr;
	int *ivar, *istart, *iend;

	virtual void calculate();


	virtual void define();
	virtual void write(FILE *f);
	virtual void read(FILE *f);
	virtual void addme(action_func **funclist, char *levellist, int *N, int *level, int max);

	virtual action_func *create() {return new action_for(); };
};



const int max_action_levels = 16;

class action_levels : public action_func_control
{
public:
  virtual ~action_levels(){};
	action_func *level_ptr[max_action_levels];
	
	int currentlevel;

	virtual void calculate();

	virtual void define();
	virtual void write(FILE *f);
	virtual void read(FILE *f);
	virtual void addme(action_func **funclist, char *levellist, int *N, int *level, int max);

	virtual action_func *create() {return new action_levels(); };
};





#endif


