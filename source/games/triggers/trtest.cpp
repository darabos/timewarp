


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
REGISTER_FILE


#include "trtest.h"
#include "trhelper.h"


#include "trcontrol.h"
#include "trvarsetting.h"
#include "trmath.h"
#include "trgamesetting.h"
#include "trships.h"




action_vars_str		action_vars;

ActionID			action_id;


/*
------------------------------- ACTION FUNCTIONLIST (manager) ---------------------------
*/
void ActionID::init()
{
//	action_id.add(new action_startfunction() );		// don't add: it's an init function for the main program action: initialized by the main program
//	action_id.add(new action_subr() );				// don't add it; it's an init function and a variable (doh)

	add(new action_if() );
	add(new action_for() );
	add(new action_setI() );
	add(new action_setS() );
	add(new action_convID() );
	add(new action_addI() );
	add(new action_subI() );
	add(new action_mulI() );
	add(new action_divI() );

	add(new action_mapsize() );
	add(new action_planet() );
	add(new action_playership() );
	add(new action_AIship() );
	add(new action_newteam() );


}




ActionID::ActionID()
{
	N = 0;
}

ActionID::~ActionID()
{
	int i;
	for ( i = 0; i < N; ++i )
		if (action[i])
			delete action[i];
}

void ActionID::add(action_func *newaction)
{
	action[N] = newaction;
	action[N]->define();
	//action[N]->define_treat();
	++N;
}

action_func *ActionID::create(char *id)
{
	// create the trigger with this id.
	int i;
	for ( i = 0; i < N; ++i )
	{
		if (strcmp(id, action[i]->id) == 0)
			return action[i]->create();
	}

	// if the ID wasn't found:
	return 0;
}






/*
------------------------------- ACTION FUNCTION ---------------------------
*/


action_func::action_func()
{
	next = 0;
	prev = 0;

	int i;
	for ( i = 0; i < max_actionfunc_vars; ++i )
	{
		descr[i] = 0;
		vartype[i] = 0;
		varindex[i] = -1;
	}

	id = 0;
}


// remove the "current" item
action_func::~action_func()
{
	
	action_func *a, *b;

	a = prev;
	b = next;

	if (a)
		a->next = b;
	if (b)
		b->prev = a;
		

	if (id)
		delete id;

	int i;
	for ( i = 0; i < max_actionfunc_vars; ++i )
	{
		if (descr[i])
			delete descr[i];
	}
	
	
}


void action_func::insert(action_func *a, action_func *b)
{
	// insert it in the list.
	if (a)
		a->next = this;
	if (b)
		b->prev = this;
	
	prev = a;
	next = b;
}


// append a to the end of the list
void action_func::append(action_func *a)
{
	action_func *current;

	current = this;
	while (current->next)
		current = current->next;

	current->next = a;
	a->prev = current;
	a->next = 0;
}




void action_func::interpret(char *d ...)
{
	// variable argument list va
	va_list va;

	// starts the argument list
	va_start(va, d);

	description = d;

	// this interprets the description and derives the number of arguments (Narg)
	define_treat();

	int i;
	for ( i = 0; i < Narg; ++i )
		varptr[i] = va_arg(va, void**);

	va_end(va);
}



void action_func::define()
{
	interpret("gen: genfunc");
}





void action_func::define_treat()
{

	// check the id string.
	int i;

	char *s;

	Narg = 0;

	s = description;

	s = strsub(&id, s, ':');

	if (!s) return;
	s += 1;	// skip the ":" character.


	while ( s && s[0] != 0 && Narg < max_actionfunc_vars )
	{

		s = skipspaces(s);
		if (!s) break;

		// scan the description
		s = strsub(&descr[Narg], s, ' ');
		if (!s) break;

		s = skipspaces(s);
		if (!s) break;

		// obtain the id type (a 1-char thingy - I, D, S or F)
		vartype[Narg] = s[0];
		s += 1;

		++ Narg;

	}

	for ( i = 0; i < Narg; ++i )
	{
		varindex[i] = -1;
	}

}








// write variable-indexes
void action_func::write(FILE *f)
{
	file_write(Narg, varindex, f);
}


// read variable indexes
void action_func::read(FILE *f)
{

	// read variable indexes
	file_read(&Narg, varindex, f);

	if (Narg > max_actionfunc_vars)
		tw_error("Too many arguments read");
	

	// let the required pointers point to the variable locations.
	int i;

	// THIS CAUSES AN ERROR !!
	// butte whye ??
	// dubious code.
	for ( i = 0; i < Narg; ++i )
	{
		/*
		// int var
		// int *k        --> k = &var
		// void *p = &i  --> ??

		int var;
		int *k;				// pointer to int
		void** p = (void**) &k;		// pointer to pointer to int
		// note that &k is op type (int **)

		k = &var;

		*p = &var;
		*/


			//= varptr[i];
		//void *p;
		//p = action_vars.getptr(vartype[i], varindex[i]);

		// copy the pointer contents (= the address it points to)
		// I don't see how to do this otherwise ...
		//memcpy(varptr[i], &p, sizeof(void*));

		*varptr[i] = action_vars.getptr(vartype[i], varindex[i]);
	}

}


void action_func::calculate()
{
	// nothing done by default.
}



// for constructing an array of these functions
void action_func::addme(action_func **funclist, char *levellist, int *N, int *level, int max)
{
	if (*N < max)
	{
		funclist[*N] = this;

		levellist[*N] = *level;

		++*N;
	}
}





/*
------------------------------- ACTION VARIABLES ---------------------------
*/


/* variables
// everything, including "constants", are treated through variables ...

  variables starting with id "constant_########" should be showing only their value in the editor.
  variables with id "" (0) are empty and ready for use (or can be overwritten).
*/



action_vars_str::action_vars_str()
{
	int i;
	for ( i = 0; i < max_action_vars; ++i )
	{
		intvar[i] = 0;
		intvarid[i][0] = 0;
		doublevar[i] = 0;
		doublevarid[i][0] = 0;
		funcvar[i] = 0;
		funcvarid[i][0] = 0;
		strvar[i] = 0;
		strvarid[i][0] = 0;
	}
}

action_vars_str::~action_vars_str()
{
	int i;
	for ( i = 0; i < max_action_vars; ++i )
	{
		if (strvar[i])
			delete strvar[i];

		if (funcvar[i])
			delete funcvar[i];
	}
}


// initialize variables values and their id's from disk
void action_vars_str::read_vars(char *fname)
{
	FILE *f;

	f = fopen(fname, "rb");
	if (!f)
		return;

	fread(&nI, 1, sizeof(int), f);
	if (nI > max_action_vars) {tw_error("wrong number of vars");}
	fread(intvar, nI, sizeof(int), f);
	fread(intvarid, nI, var_id_len, f);

	fread(&nD, 1, sizeof(int), f);
	if (nD > max_action_vars) {tw_error("wrong number of vars");}
	fread(doublevar, nD, sizeof(int), f);
	fread(doublevarid, nD, var_id_len, f);

	fread(&nF, 1, sizeof(int), f);
	if (nF > max_action_vars) {tw_error("wrong number of vars");}
	fread(funcvar, nF, sizeof(action_func*), f);
	fread(funcvarid, nF, var_id_len, f);

	// read char strings ...
	// reserve memory for it ? Nah ...
	fread(&nS, 1, sizeof(int), f);
	if (nS > max_action_vars) {tw_error("wrong number of vars");}

	int i;
	for ( i = 0; i < nS; ++i )
	{
		//int nchar;
		//fread(&nchar, 1, sizeof(int), f);	// nchar includes the zero ending it.
		//strchar[i] = new char [nchar];
		//fread(strchar[i], nchar, sizeof(char), f);
		file_init(&strvar[i], f);
	}
	fread(strvarid, nS, var_id_len, f);

	// also, read all the subroutine info :
	for ( i = 0; i < nF; ++i )
		read_subr(i, f);

	fclose(f);
}


// store them on disk
void action_vars_str::write_vars(char *fname)
{
	FILE *f;

	f = fopen(fname, "wb");
	if (!f)
		return;

	fwrite(&nI, 1, sizeof(int), f);
	fwrite(intvar, nI, sizeof(int), f);
	fwrite(intvarid, nI, var_id_len, f);

	fwrite(&nD, 1, sizeof(int), f);
	fwrite(doublevar, nD, sizeof(int), f);
	fwrite(doublevarid, nD, var_id_len, f);

	fwrite(&nF, 1, sizeof(int), f);
	fwrite(funcvar, nF, sizeof(action_func*), f);
	fwrite(funcvarid, nF, var_id_len, f);

	fwrite(&nS, 1, sizeof(int), f);

	int i;
	for ( i = 0; i < nS; ++i )
	{
		//int nchar;
		//nchar = strlen(strchar[i]) + 1;	// include the 0
		//fwrite(&nchar, 1, sizeof(int), f);
		//fwrite(strchar[i], nchar, sizeof(char), f);

		file_write(strvar[i], f);
	}
	fwrite(strvarid, nS, var_id_len, f);

	// also, write all the subroutine info :
	for ( i = 0; i < nF; ++i )
		write_subr(i, f);

	fclose(f);
}






void action_vars_str::read_subr(int index, FILE *f)
{
	// still to write
	funcvar[index] = new action_subr();
	funcvar[index]->read(f);
}


void action_vars_str::write_subr(int index, FILE *f)
{
	// still to write
	funcvar[index]->write(f);
}


int action_vars_str::N(char vartype)
{

	switch(vartype)
	{
	case 'I':
		return nI;
		break;

	case 'D':
		return nD;
		break;

	case 'S':
		return nS;
		break;

	case 'F':
		return nF;
		break;

	default:
		return -1;
	}
}

int action_vars_str::newvar(char vartype, char *idstr)
{
	int *N;
	var_id *ids;


	switch(vartype)
	{
	case 'I':
		N = &nI;
		ids = intvarid;
		break;

	case 'D':
		N = &nD;
		ids = doublevarid;
		break;

	case 'S':
		N = &nS;
		ids = strvarid;
		break;

	case 'F':
		N = &nF;
		ids = funcvarid;
		break;

	default:
		return -1;
	}

	// just add a var.
	strncpy(ids[*N], idstr, var_id_len);
	ids[*N][var_id_len-1] = 0;

	++ *N;

	return *N;

	/* NO, NO OVERWRITING !!
	// first, check if the var already exists... (for overwriting)
	int i;
	for ( i = 0; i < *N; ++i )
		if (strcmp(idstr, ids[i]) == 0)
			break;

	if (i != *N)
	{
		// otherwise, find the first empty one (or the new one)
		i = 0;
		while (i < *N)
		{
			if (ids[i][0] == 0)	// an empty one
				break;
			++i;
		}
	}

	return i;
	*/
}





void *action_vars_str::getptr(char vartype, int i)
{
	if (i < 0)
		return 0;

	// let the required pointers point to the variable locations.
	switch( vartype)
	{
	case 'I':
		return &intvar[i];
		break;
		
	case 'D':
		return &doublevar[i];
		break;
		
	case 'S':
		return &strvar[i];
		break;
		
	case 'F':
		return &funcvar[i];
		break;
		
	default:
		tw_error("Unknow action var type");
		break;
	}

	return 0;
}





void action_vars_str::set_id(char vartype, int i, char *id)
{
	char *trg = NULL;

	switch (vartype)
	{
	case 'I':
			trg = intvarid[i];
			break;
	case 'D':
			trg = doublevarid[i];
			break;
	case 'S':
			trg = strvarid[i];
			break;
	default:
	  ASSERT(0&&"trg is not valid");
	  break;
	}

	strncpy(trg, id, var_id_len);
	trg[var_id_len-1] = 0;
}



char *action_vars_str::get_id(char vartype, int i)
{

	switch (vartype)
	{
	case 'I':
			return intvarid[i];
			break;
	case 'D':
			return doublevarid[i];
			break;
	case 'S':
			return strvarid[i];
			break;
	}

	return 0;
}





void action_vars_str::set_val(char vartype, int i, char *str)
{

	switch (vartype)
	{
	case 'I':
			intvar[i] = atoi(str);
			break;
	case 'D':
			doublevar[i] = atof(str);
			break;
	case 'S':
			// strvar[i] = str;
			// first, allocate memory for the string (make a copy of it !!)
			strvar[i] = new char [strlen(str)+1];
			strcpy(strvar[i], str);
			break;
	}

}



void action_vars_str::get_val(char vartype, int i, char *trg)
{

	switch (vartype)
	{
	case 'I':
			sprintf(trg, "%10i", intvar[i]);
			break;
	case 'D':
			sprintf(trg, "%10.3f", doublevar[i]);
			break;
	case 'S':
			strcpy(trg, strvar[i]);
			break;
	default:
		trg = 0;
	}

}
