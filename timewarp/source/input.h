/*
Key format
*/

typedef short int Key;

int enable_input ( int which = 255) ;
int disable_input ( int which = 255 ) ;
void poll_input ();

int key_pressed(Key key) ;
Key name_to_key ( const char *name);

Key get_key();

int key_to_name( Key key, char *buffer ); 
int key_to_description( Key key, char *buffer ); 
//returns at most 63 characters

int joykey_enumerate (Key *keys) ;
//no overflow check, better have plenty of room
//the max possible is about 2000
//typical is 6-12
