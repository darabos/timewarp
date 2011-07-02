/* $Id$ */ 
/*
	This file contains information about 2 things:

1.  Libraries used by this program.  

2.  Compile-time options of this program.  

These two are related, because most libraries used by this 
program are not required, merely optional.  The dependance 
upon specific libraries can be turned off using the command 
line options described below.  

*/

/* 
Optional Library: JGMOD (libjgmod.a, libjgmod.lib, whatever).  
Provides music support.  Disable by uncommenting the first 
line of util/sounds.cpp (NO_JGMOD)
*/

/* 
Optional Library: Win/BSD sockets (wsock32.lib / whatever)
Networking:
Networking may be disabled by uncommenting the first line of util/net_tcp.cpp
(NETWORK_NONE)
If networking is not disabled, you will have to link with a 
platform-specific library.  On windows, this means wsock32.lib
*/

/* 
Optional Library: ?windows multimedia? (winmm.lib).  
Provides higher-precision time measurement under windows.  This 
is ignored on non-windows platforms, and can be disable by 
uncommenting the first line of util/get_time.cpp (NO_WINMM)
*/


/* 
Compile-Time Option for Stack Trace Debugging: 
If enabled, TW code will manually keep track of a stack trace similar 
to that which debuggers produce.  This can be a substancial help for 
debugging without an good debugger, or for debugging in release  mode.  
To enable, uncomment the 4th line of util/errors.h (DO_ )
*/


/* 
Compile-Time Option for Demo/Network Debugging: 
If enabled, an excessive # of checksums will be generated...
This can help demo recordings pinpoint the locations of desynchronizations
But demos/network games with it are not compatible with those without this option
Also, this option will slow down normal games and do TERRIBLE things to network games
*/

//#define LOTS_OF_CHECKSUMS

