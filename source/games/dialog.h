//////////////////////////////////////////////////////////////////////////
//                                                                      //
//         Test Dialog module for TimeWarp                              //
//         (c) Yura Semashko aka Yurand <yurand@land.ru>                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_MODULE_H__
#define __DIALOG_MODULE_H__

extern "C" {
#	include "lua.h"
}
#include <allegro.h>

/*! \brief Communicatin */
class Dialog
{
 private:
  /*! \brief Alien creature image */
  static BITMAP * btmAlien;
  /*! \brief width of Alien creature */
  static int alien_image_x;
  /*! \brief height of Alien creature */
  static int alien_image_y;
  /*! \brief screen image before dialog */
  static BITMAP * btmPScreen;
  lua_State * L;
  
  /*! \brief Write text from lua to screen */
  static int l_Write(lua_State* ls);
  /*! \brief Decision module */
  static int l_Answer(lua_State* ls);
  /*! \brief Fight module */
  static int l_Fight(lua_State* ls);
  static int l_SetAilenImage(lua_State* ls);
 public:
  /*! \brief Talk function */
  void Conversate(const char *dialogfile);
  
  Dialog();
  ~Dialog();
};


#endif // __DIALOG_MODULE_H__
