


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
#include "../../melee/mframe.h"
REGISTER_FILE


#include "trtest.h"
#include "trhelper.h"




// ------------------ CONTROL FLOW !!!!! ------------------

// this is how it should work ...


int action_func_control::exec_block(action_func *first)
{
	action_func *current;
	current = first;

	while (current)
	{
		current->calculate();
		current = current->next;
	}

	return 1;
}


int action_func_control::write_block(action_func *first, FILE *f)
{
	action_func *current;
	current = first;

	while (current)
	{
		file_write(current->id, f);		// the id is overhead for re-initialization.
		current->write(f);				// action_func content
		current = current->next;
	}

	// to signal the end of the block (0 id string length).
	file_write(int(0), f);

	return 1;
}



// this initializes a block of actions
// this should start with
// activating a control function (which has no parameters itself, just
// pointers to the blocks, and manages the "read" actions).

action_func *action_func_control::init_block(FILE *f)
{
	char idstr[512];
	action_func *first, *current, *newaction;

	first = 0;
	current = first;

	for (;;)
	{
		file_read(idstr, f);

		if (idstr[0] == 0)
			break;	// the last element ...

		// allocate the action structure
		newaction = action_id.create(idstr);
		newaction->define();
		//newaction->define_treat();

		// read the variables which tweak the action
		newaction->read(f);

		// insert it in the list, if it exists.
		if (first != 0)
			newaction->insert(current, current->next);
		else
			first = newaction;

		current = newaction;
	}

	return first;
}



// for use with addme
void action_func_control::add_block(action_func **funclist, char *levellist, int *N, int *level, int max, action_func *first)
{
	action_func *current;
	current = first;

	++ *level;

	while (current)
	{
		current->addme(funclist, levellist, N, level, max);
		current = current->next;

		if (*N >= max)
			break;
	}

	-- *level;

}




// --------------------------- main block control ------------------------------



// the program should use it like this ...
// it should call the start-function, whose only function is to
// execute other functions ...


//action_startfunction::action_startfunction()
//:
//action_func_control()
//{
//	timer = 0;
//	main_ptr = 0;
//}

void action_startfunction::define()
{
	//description = "start:";
	//varptr = 0;
	interpret("start:");

	timer = 0;
	main_ptr = 0;
	timeforaction = 0.0;
}


void action_startfunction::calculate()
{
	timer += physics->frame_time * 1E-3;

	if (timer > timeforaction)
	{
		exec_block(main_ptr);

		timer -= timeforaction;
	}
}


void action_startfunction::write(char *filename)
{
	FILE *f;
	f = fopen(filename, "wb");
	if (!f)
		return;

	write(f);

	fclose(f);
}



void action_startfunction::write(FILE *f)
{
	action_func::write(f);

	// write the controlled block (parameters/functions)
	write_block(main_ptr, f);
}


void action_startfunction::read(char *filename)
{
	FILE *f;
	f = fopen(filename, "rb");
	if (!f)
		return;

	read(f);

	fclose(f);
}

void action_startfunction::read(FILE *f)
{
	action_func::read(f);

	// write the controlled block (parameters/functions)
	main_ptr = init_block(f);
}


void action_startfunction::append(action_func *a)
{
	if (main_ptr)
		main_ptr->append(a);
	else
		main_ptr = a;		// extra initialization ... (doh)

}



// for constructing an array of these functions
void action_startfunction::addme(action_func **funclist, char *levellist, int *N, int *level, int max)
{
	action_func::addme(funclist, levellist, N, level, max);
	add_block(funclist, levellist, N, level, max, main_ptr);
}



// --------------------------- subroutine-like control ------------------------------



// subroutine-like control
// this simply jumps to another part of code and executes that till the end of the block.

void action_subr::define()
{
	//description = "subroutine:";
	//varptr = 0;
	interpret("subroutine:");

	block_ptr = 0;
}



void action_subr::calculate()
{
	exec_block(block_ptr);
}


void action_subr::write(FILE *f)
{
	action_func::write(f);

	// write the controlled block (parameters/functions)
	write_block(block_ptr, f);
}


void action_subr::read(FILE *f)
{
	action_func::read(f);

	// write the controlled block (parameters/functions)
	block_ptr = init_block(f);
}



// for constructing an array of these functions
void action_subr::addme(action_func **funclist, char *levellist, int *N, int *level, int max)
{
	action_func::addme(funclist, levellist, N, level, max);

	// don't show content of subroutines ...
	//add_block(funclist, N, max, block_ptr);
}



// --------------------------- for loop control ------------------------------

void action_for::define()
{

	interpret("for: for I = I to I",
				&ivar, &istart, &iend);

	block_ptr = 0;
}



void action_for::calculate()
{
	for ( *ivar = *istart; *ivar < *iend; ++*ivar )
		exec_block(block_ptr);
}


void action_for::write(FILE *f)
{
	action_func::write(f);

	// write the controlled block (parameters/functions)
	write_block(block_ptr, f);
}


void action_for::read(FILE *f)
{
	action_func::read(f);

	// write the controlled block (parameters/functions)
	block_ptr = init_block(f);
}



// for constructing an array of these functions
void action_for::addme(action_func **funclist, char *levellist, int *N, int *level, int max)
{
	action_func::addme(funclist, levellist, N, level, max);
	add_block(funclist, levellist, N, level, max, block_ptr);
}




// --------------------------- if control ------------------------------


// if type control


void action_if::define()
{

	//description = "if: if I";
	//varptr = (void**) (&ichoice);
	interpret("if: if I",
				&ichoice);

	true_block_ptr = 0;
	false_block_ptr = 0;
}



void action_if::calculate()
{
	if (*ichoice)
		exec_block(true_block_ptr);
	else
		exec_block(false_block_ptr);
}


void action_if::write(FILE *f)
{
	action_func::write(f);

	// write the controlled blocks (parameters/functions)
	write_block(true_block_ptr, f);
	write_block(false_block_ptr, f);
}


void action_if::read(FILE *f)
{
	action_func::read(f);

	// initialize the controlled blocks (parameters/functions)
	true_block_ptr = init_block(f);
	false_block_ptr = init_block(f);
}



// for constructing an array of these functions
void action_if::addme(action_func **funclist, char *levellist, int *N, int *level, int max)
{
	action_func::addme(funclist, levellist, N, level, max);
	add_block(funclist, levellist, N, level, max, true_block_ptr);
	add_block(funclist, levellist, N, level, max, false_block_ptr);
}




// --------------------------- levels control ------------------------------


// a set of different levels ...


void action_levels::define()
{

	//description = "if: if I";
	//varptr = (void**) (&ichoice);
	interpret("levels:");

	// always start with level 0
	currentlevel = 0;

	// initialy no level defined yet
	int i;
	for ( i = 0; i < max_action_levels; ++i )
		level_ptr[i] = 0;
}



void action_levels::calculate()
{
	exec_block(level_ptr[currentlevel]);
}


void action_levels::write(FILE *f)
{
	action_func::write(f);

	// write the controlled blocks (parameters/functions)
	int i;
	for ( i = 0; i < max_action_levels; ++i )
		write_block(level_ptr[i], f);

	// but, what happens with an invalid pointer ?
	
}


void action_levels::read(FILE *f)
{
	action_func::read(f);

	// initialize the controlled blocks (parameters/functions)
	int i;
	for ( i = 0; i < max_action_levels; ++i )
		level_ptr[i] = init_block(f);

	// can you read 0 pointers and blocks ?
}




// for constructing an array of these functions
void action_levels::addme(action_func **funclist, char *levellist, int *N, int *level, int max)
{
	action_func::addme(funclist, levellist, N, level, max);
	int i;
	for ( i = 0; i < max_action_levels; ++i )
		add_block(funclist, levellist, N, level, max, level_ptr[i]);
}

