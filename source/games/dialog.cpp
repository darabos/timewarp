#include <string.h>

extern "C" {
#	include "lua.h"
#	include "lualib.h"
#	include "lauxlib.h"
}

#include <allegro.h>
#include "dialog.h"
#include "../util/aautil.h"
#include "../util/aastr.h"

BITMAP* Dialog::btmAlien   = NULL;
BITMAP* Dialog::btmPScreen = NULL;

int Dialog::alien_image_x = 0;
int Dialog::alien_image_y = 0;

Dialog::Dialog()
{
  btmPScreen = create_bitmap(SCREEN_W, SCREEN_H);
  ASSERT(btmPScreen);
  blit(screen, btmPScreen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
  clear(screen);
  L = lua_open();
  /////////////////////////////////////////////////////////
  // Register C function for using in lua
  ////////////////////////////////////////////////////////
  lua_register(L, "Write", l_Write);
  lua_register(L, "Answer", l_Answer);
  lua_register(L, "Fight", l_Fight);
  lua_register(L, "SetAlienImage", l_SetAilenImage);
  /////////////////////////////////////////////////////////
  alien_image_x = SCREEN_W;
  alien_image_y = SCREEN_H/2;
}

Dialog::~Dialog()
{
  lua_close(L);
  show_mouse(NULL);
  blit(btmPScreen, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
  show_mouse(screen);
  destroy_bitmap(btmPScreen);
  btmPScreen = NULL;
}
////////////////////////////////////////////////////////////////////////////
//Lua function implementation
///////////////////////////////////////////////////////////////////////////

int Dialog::l_Write(lua_State* ls)
{
  if(btmAlien)
    {
      show_mouse(NULL);
      blit(btmAlien, screen, 0, 0, 0, 0, alien_image_x, alien_image_y);
      show_mouse(screen);
    }
  const char* text = lua_tostring(ls, 1);
  ASSERT(text);
  show_mouse(NULL);
   textout(screen, font, text, 10, 10, -1);
  show_mouse(screen);
  return 0;
}

int Dialog::l_Answer(lua_State* ls)
{
  int i;
  int num = lua_gettop(ls); // number of argument
  MENU *pMenuAnswers = new MENU[num+1];
  for(i=0;i<num;i++)
    {
      const char* ctext = lua_tostring(ls, i+1);
      ASSERT(ctext);
      char * text = strdup(ctext);
      pMenuAnswers[i].text   = text;
      pMenuAnswers[i].proc   = NULL;
      pMenuAnswers[i].child  = NULL;
      pMenuAnswers[i].flags  = 0;
      pMenuAnswers[i].dp     = NULL;
    }
  pMenuAnswers[num].text   = NULL;
  pMenuAnswers[num].proc   = NULL;
  pMenuAnswers[num].child  = NULL;
  pMenuAnswers[num].flags  = 0;
  pMenuAnswers[num].dp     = NULL;
  
    int ret = -1;
    while( ret == -1 )
      ret = do_menu(pMenuAnswers, 0, screen->h/2);
    ret++;
  ///////////////////////////////////////////////
  //Free memory
   for(i=0;i<num;i++)
    free(pMenuAnswers[num].text);
  delete[] pMenuAnswers;
  lua_pushnumber(ls, ret);
  return 1;
}

int Dialog::l_Fight(lua_State* ls)
{
  return 0;
}

int Dialog::l_SetAilenImage(lua_State* ls)
{
  const char* text = lua_tostring(ls, 1);
   if(btmAlien)
     {
       destroy_bitmap(btmAlien);
       btmAlien = NULL;
     }

   BITMAP *temp = load_bitmap( text, NULL);
   aa_set_mode(AA_DITHER);
   ASSERT(temp);
   btmAlien = create_bitmap(alien_image_x, alien_image_y);
   aa_stretch_blit(temp, btmAlien, 0, 0, temp->w, temp->h, 0, 0, alien_image_x, alien_image_y);
   destroy_bitmap(temp);

   show_mouse(NULL);
   blit(btmAlien, screen, 0, 0, 0, 0, alien_image_x, alien_image_y);
   show_mouse(screen);
   return 0;
}

void Dialog::Conversate(const char *dialogfile)
{
  lua_dofile(L, dialogfile);
}

