
#ifndef __GAMEX_DISK_STUFF__
#define __GAMEX_DISK_STUFF__

#include "../../twgui/twgui.h"

// part of this, should go into the gamedata section ?!

struct MemStore
{
	char filename[128];
	int pos;

	int dsize, maxdata;
	char *data;

	virtual ~MemStore();

	void reset(int N);

	virtual void fread(char *fname, int minsize);
	void fwrite();

	void read(char *d, int N);
	void read(char *d);
	void read(int *d);
	void read(int *d, int N);

	void write(char *d, int N);
	void write(char d);
	void write(int d);
	void write(int *d, int N=1);
	void writestring(char *d);

	void seek(int newpos);
	void initstring(char *txt);
};

class BlockStore : public MemStore
{
	struct Stats
	{
		int version;
		int Nindex;
		int maxindex;
	} *stats;

	int pos_index, pos_block;
	int *indexes;

public:

	BlockStore();
	virtual ~BlockStore();

	virtual void fread(char *fname, int minsize);

	void seekblock(int index);
	int getindex(int index);
	void setindex(int index, int pos);
	void add2index(int index, int dpos);

	// overwrite an existing block of data, with data stored in another "temp file".
	int writeblock(int index, MemStore *d);

	void setversion(int i);
	int getversion();


};




struct FileStore
{
	FILE *f;

	FileStore(char *afilename);
	~FileStore();

	// closest to c stuff
	void read(char *d, int N);
	void write(char *d, int N);
	void seek(int newpos);
	int tell();
	int size();

	// derived stuff
	void read(char *d);
	void read(int *d);
	void read(int *d, int N);

	void write(char d);
	void write(int d);
	void write(int *d, int N=1);
	void writestring(char *d);

	void initstring(char *txt, int max);
};


#endif // __GAMEX_DISK_STUFF__


