/* $Id$ */ 
/*
The use of this:
one huge block of memory, where you save a copy of disk data. Data can be
saved in separate blocks, each one representing a record.

Can be useful if you got lots of varying sized data, which you want to acces
randomly. I intended to use this for dialogues ... but those are so small, it
is not really needed for that...

*/

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
REGISTER_FILE


#include "disk_stuff.h"




MemStore::~MemStore()
{
	delete data;
}


void MemStore::fread(char *fname, int minsize)
{

	strncpy(filename, fname, 127);
	filename[127] = 0;

	FILE *f;
	f = fopen(fname, "rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		dsize = ftell(f);
		rewind(f);
	} else
		dsize = 0;
	
	if (dsize < minsize)
	{
		data = new char [minsize];
		maxdata = minsize;
	} else {
		data = new char [dsize];
		maxdata = dsize;
	}

	pos = 0;
	if (f)
	{
		// by default, position at the end of the data
		// (then, pos returns the #bytes that were actually read !!)
		pos = ::fread(data, 1, dsize, f);
		fclose(f);
	}

}

void MemStore::fwrite()
{
	// use filename
	FILE *f;
	f = fopen(filename, "wb");
	if (!f)
		return;
	
	::fwrite(data, dsize, 1, f);
	fclose(f);
}

// it should be possible to seek outside the size range (to add new data elsewhere)
void MemStore::seek(int newpos)
{
	pos = newpos;

	if (pos < 0)
		pos = 0;

	if (pos >= dsize)
		//pos = dsize-1;
		dsize = pos;	// not +1, because don't create space for an item yet (don't know its size - there's no item)
	
}

void MemStore::read(char *d, int N)
{
	if (N < 0)
		return;

	if (pos+N-1 >= dsize)			// -1 because pos itself also is included.
		N = dsize - pos;	// this condition should never occur
	if (N < 0)
		N = 0;

	memcpy(d, &data[pos], N);
	pos += N;
}


void MemStore::write(char *d, int N)
{
	if (N <= 0)
		return;

	if (pos+N >= dsize)
		//N = dsize-1 - pos;
		dsize = pos+N;		// adjust the size so that you can store new data

	memcpy(&data[pos], d, N);
	pos += N;
}



void MemStore::read(char *d)
{
	read(d, 1);
}

void MemStore::read(int *d)
{
	read((char*) d, 4);
}

void MemStore::read(int *d, int N)
{
	read((char*) d, 4*N);
}



void MemStore::write(char d)
{
	write(&d, 1);
}

void MemStore::write(int d)
{
	write((char*) &d, 4);
}

void MemStore::write(int *d, int N)
{
	write((char*) d, 4*N);
}


void MemStore::writestring(char *d)
{
	write(d, strlen(d)+1);
}


void MemStore::initstring(char *txt)
{
	char *s;
	s = &data[pos];
	int L;
	L = strlen(s);

	//*txt = new char [L+1];
	read(txt, L+1);
}



void MemStore::reset(int N)
{
	dsize = 0;
	maxdata = N;
	data = new char [N];
	pos = 0;

}


BlockStore::BlockStore()
{
}

BlockStore::~BlockStore()
{
}


void BlockStore::fread(char *fname, int minsize)
{
	MemStore::fread(fname, minsize);

	stats = (Stats*) data;

	// indexes follow the stats immediately

	pos_index = sizeof(Stats);
	indexes = (int*) &data[pos_index];

	// create an empty data structure...
	if ( pos == 0)
	{
		stats->maxindex = 10000;	// a ridiculous high number ;)
		stats->Nindex = 0;
	}

	pos_block = pos_index + 4 * stats->maxindex;
}


int BlockStore::getversion()
{
	return stats->version;
}

void BlockStore::setversion(int i)
{
	stats->version = i;
}

void BlockStore::seekblock(int index)
{
	pos = getindex(index);
}

int BlockStore::getindex(int index)
{
	if (index <= stats->Nindex)
		// <= because the last-equal one shows the end position of the file.
		return  indexes[index];
	else
		return -1;
}

void BlockStore::setindex(int index, int pos)
{
	indexes[index] = pos;
}

void BlockStore::add2index(int index, int dpos)
{
	indexes[index] += dpos;
}


int BlockStore::writeblock(int index, MemStore *d)
{
	// if it's a new block, then ... well... doh !!
	if (index < 0)
	{
		if (stats->Nindex == 0)
		{
			// an entirely new file, with 1 empty block
			index = 0;
			setindex(0, pos_block);
			setindex(1, pos_block);
			dsize = pos_block;
			// the minimum size of the file = all indexes and header and 0 blocks.
			// note that pos_block indicates the first byte of the (new) data, so no +1 is needed here.

		} else {
			// a new empty block at the end of the file
			index = stats->Nindex;
			setindex(index+1, getindex(index));
		}
		
		++ stats->Nindex;
	}

	if (index >= stats->Nindex)
	{
		tw_error("This block does not exist - use -1 to add a block at the end");
	}

	int k, pos1, pos2;
	
	pos1 = getindex(index);
	pos2 = getindex(index+1);
	
	k = pos2 - pos1;
	if (k != d->dsize)
	{
		// create or delete room for the new data of this block
		// d->dsize = the required new size
		// k = the old size of this record
		
		int shift = d->dsize - k;
		memmove(&data[pos2+shift], &data[pos2], dsize-pos2);
		// note that pos2 points 1 byte beyond - so, should be included in the move.
		
		// new data were added :
		dsize += shift;
		
		// update the index pointers 
		int i;
		for ( i = index+1; i <= stats->Nindex; ++i )	// the ==Nindex one is the closing one..
		{
			//	int p;
			//	p = getindex(i);
			//	p += shift;
			//	setindex(i, p);
			add2index(i, shift);
		}
		
		// shift the current file position accordingly.
		if (pos >= pos2)
			pos += shift;
	}



	// there's exactly enough room now
	int p;
	p = getindex(index);

	// if there's extra room (needed):
	if (p + d->dsize > dsize)
		dsize = p + d->dsize;
	if (dsize > maxdata)
	{
		tw_error("error: exceeding data limit");
	}

	memcpy(&data[p], d->data, d->dsize);

//	// set the start
//	setindex(index, p);
//	// set the end (=start of next block, even if that next block doesn't exist yet)
//	setindex(index+1, p+d->dsize);


	return index;
}









FileStore::FileStore(char *afilename)
{
	f = fopen(afilename, "rb+");
	if (!f)
	{
		f = fopen(afilename, "wb+");
		if (!f)
		{
			tw_error("FileStore: failed to open file");
		}
	}
	seek(0);
}

FileStore::~FileStore()
{
	fclose(f);
}


void FileStore::read(char *d, int N)
{
	if (N < 0)
		return;

	int i;
	i = fread(d, 1, N, f);

//	if (i != N)
//	{
//		tw_error("Unable to read from file");
//	}

}


void FileStore::write(char *d, int N)
{
	if (N <= 0)
		return;

	int i;
	i = fwrite(d, 1, N, f);

	if (i != N)
	{
		tw_error("Unable to write to file");
	}
}


void FileStore::seek(int newpos)
{
	fseek(f, newpos, SEEK_SET);
}


int FileStore::tell()
{
	return ftell(f);
}

int FileStore::size()
{
	int pos, s;

	pos = tell();
	fseek(f, 0, SEEK_END);
	s = tell();

	seek(pos);

	return s;
}



void FileStore::read(char *d)
{
	read(d, 1);
}

void FileStore::read(int *d)
{
	read((char*) d, 4);
}

void FileStore::read(int *d, int N)
{
	read((char*) d, 4*N);
}



void FileStore::write(char d)
{
	write(&d, 1);
}

void FileStore::write(int d)
{
	write((char*) &d, 4);
}

void FileStore::write(int *d, int N)
{
	write((char*) d, 4*N);
}


void FileStore::writestring(char *d)
{
	write(d, strlen(d)+1);
}


void FileStore::initstring(char *txt, int max)
{
	int pos;
	pos = tell();

	// this contains the whole string (plus garbage but that does not matter)
	read(txt, max);

	// proceed to the byte following the zero terminate char (--> + 1).
	int L;
	L = strlen(txt);
	seek(pos + L + 1);
}


