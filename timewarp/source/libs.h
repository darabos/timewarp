/* 
Optional Library: JGMOD (libjgmod.a, libjgmod.lib, whatever).  
Provides music support.  Disable by uncommenting the first 
line of util/sounds.cpp (NO_JGMOD)
*/

/* 
Optional Library: Win/BSD sockets (wsock32.lib / whatever)
Networking:
Networking may be disabled by uncommenting the first line of util/net_tcp.cpp
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
Compile-Time Option for Debuging: 
If enabled, an excessive # of checksums will be generated...
This can help demo recordings pinpoint the locations of desynchronizations
But demos/network games with it are not compatible with those without this option
Also, this option will slow down normal games and do TERRIBLE things to network games
*/

//#define LOTS_OF_CHECKSUMS

