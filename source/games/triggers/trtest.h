
#ifndef __TRTEST__
#define __TRTEST__

// baseclass for the triggers ...


// basic trigger function.

const int max_actionfunc_vars = 5;

class action_func
{
public:
	// for use in a list of actions.
	action_func *prev, *next;

	action_func();
	~action_func();

	virtual void insert(action_func *a, action_func *b);
	virtual void append(action_func *a);

	// the "name" of the function, and the description defining the var types and editor helper strings
	char	*id, *description;

	// the number of arguments this function has
	int		Narg;

	// define the description.
	virtual void define();
	// interpret the description
	void define_treat();

	// description used for the editor, to explain the meaning of the variables.
	char	*descr[max_actionfunc_vars];

	// type of the variables
	char	vartype[max_actionfunc_vars];

	virtual void write(FILE *f);
	virtual void read(FILE *f);

	// indexes of the variables (points to certain variables in the total var-lists).
	int		varindex[max_actionfunc_vars];
	void	** varptr[max_actionfunc_vars];
	void	interpret(char *d ...);

//	// by reference only ( no new mem is allocated for it? )
//	void set_data(char *dnew);

	// should not be changed:

//	virtual int basesize();
//	virtual void make_datablock(char *argumentlist, char *d, int *N);

//	virtual void argstr(char **arg, char **id = 0, char ***comments = 0);

	// can (and should) be changed:

	virtual action_func *create() {return new action_func(); };
	virtual void calculate();

	// for constructing an array of these functions
	virtual void addme(action_func **funclist, char *levellist, int *N, int *level, int max);
};



// become aware of control triggers ...
// (for the action_subr var array)
#include "trcontrol.h"



/* variables
// everything, including "constants",are treated through variables ...

  variables starting with id "constant_########" should be showing only their value in the editor.
  variables with id "" (0) are empty and ready for use (or can be overwritten).
*/

const int max_action_vars = 64000;
const int var_id_len = 32;
typedef char var_id[var_id_len];	// 32 chars for a variable - used for editing it.

struct action_vars_str
{
	int		nI, nD, nF, nS;

	int		intvar[max_action_vars];
	var_id	intvarid[max_action_vars];

	double	doublevar[max_action_vars];
	var_id	doublevarid[max_action_vars];

	action_subr	*funcvar[max_action_vars];
	var_id	funcvarid[max_action_vars];

	char	*strvar[max_action_vars];
	var_id	strvarid[max_action_vars];

	void read_vars(char *fname);
	void write_vars(char *fname);

	void read_subr(int index, FILE *f);
	void write_subr(int index, FILE *f);

	int newvar(char vartype, char *idstr);

	action_vars_str();
	~action_vars_str();

	void *getptr(char vartype, int i);

	int action_vars_str::N(char vartype);
	void set_id(char vartype, int i, char *id);
	char *get_id(char vartype, int i);
	void set_val(char vartype, int i, char *str);
	void get_val(char vartype, int i, char *trg);

};

extern action_vars_str action_vars;







// All the trigger types that you can use; they're kept in this list,
// so that you can specifiy a ID string, it's searched, and a new
// struct can then be created.

const int maxactions = 512;

struct ActionID
{
	int N;

	ActionID();
	~ActionID();

	action_func *action[maxactions];

	void add(action_func *newaction);

	action_func *create(char *id);

	virtual void init();
};

extern ActionID action_id;


#endif


