/*
 *        Conversation module for GOB
 * (C) Yura Semashko aka Yurand <yurand@land.ru>
 *
 */

#include <allegro.h>

#ifdef ALLEGRO_MSVC
	#pragma warning (disable:4786)
#endif

#include "gdialog.h"
#include "../games/ggob.h"

#include "../util/aautil.h"
#include "../util/aastr.h"

#include <string>

#define gobgame ((GobGame*)game)

/*! \brief Alien Picture */
static BITMAP * g_btmAlien = NULL;
static BITMAP * g_btmOld = NULL;
bool g_bPause = false;
Music* old_music = NULL;
Music* music = NULL;

static int alien_image_x = 0;
static int alien_image_y = 0;

///////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////////////////
/*! \brief split string by words
	\param source string
	\chars maximum length of returned string
	\return result string
*/
std::string SplitString(std::string source, unsigned int chars)
{
	std::string strResult;
	if (source.length()<chars+1)
		return source;

	int size = source.rfind(' ', chars);
	source.resize(size+1);
	return source;
}

//////////////////////////////////////////////////////////////////////////////////////////
void InitConversationModule ( lua_State* L )
{
	ASSERT ( L != NULL );
  alien_image_x = SCREEN_W;
  alien_image_y = SCREEN_H/2;

  /////////////////////////////////////////////////////////
  // Register C function for using in lua
  ////////////////////////////////////////////////////////
  lua_register(L, "DialogStart",         l_DialogStart);
  lua_register(L, "DialogSetAlienImage", l_DialogSetAlienImage);
  lua_register(L, "DialogSetMusic",      l_DialogSetMusic);
  lua_register(L, "DialogWrite",         l_DialogWrite);
  lua_register(L, "DialogKeyPressed",    l_KeyPressed);
  lua_register(L, "DialogAnswer",        l_DialogAnswer);
  lua_register(L, "DialogEnd",           l_DialogEnd );
  
}

////////////////////////////////////////////////////////////////////////////
//Lua function implementation
///////////////////////////////////////////////////////////////////////////
int l_DialogStart(lua_State* ls)
{
  ASSERT ( g_btmAlien == NULL && "New StartDialog was called before DialogEnd"); 
  ASSERT ( g_btmOld   == NULL && "New StartDialog was called before DialogEnd"); 

   if ( !game->is_paused() )
   {
	    g_bPause = false;
		game->pause();
   }
   else 
	   g_bPause = true;
  
  g_btmOld = create_bitmap( game->window->surface->w, 
			    game->window->surface->h );
  game->window->lock();
  blit( game->window->surface, g_btmOld, 0, 0, 0, 0, 
	game->window->surface->w,
	game->window->surface->h );
  game->window->unlock();

  if (sound.is_music_playing())
	  old_music = sound.looping_music;

  return l_DialogSetAlienImage(ls);
}

int l_DialogSetAlienImage(lua_State* ls)
{
  if ( !lua_isstring(ls, 1) )
    {
      tw_error("SetAlienImage was summoned without image string");
    }

  const char* text = lua_tostring(ls, 1);
  if(g_btmAlien)
    {
      destroy_bitmap(g_btmAlien);
      g_btmAlien = NULL;
    }

   aa_set_mode(AA_DITHER);
   clear_bitmap ( game->window->surface );
   BITMAP *temp = load_bitmap( text, NULL);
   
   ASSERT(temp);
   g_btmAlien = create_bitmap(alien_image_x, alien_image_y);
   aa_stretch_blit(temp, g_btmAlien, 0, 0, temp->w, temp->h, 0, 0, 
		   alien_image_x, alien_image_y);
   destroy_bitmap(temp);

   game->window->lock();
   blit(g_btmAlien, game->window->surface, 0, 0, 0, 0, 
	alien_image_x, 	alien_image_y);
   game->window->unlock();
   return 0;
}

int l_DialogSetMusic(lua_State* ls)
{
	if ( !lua_isstring(ls, 1) )
	{
		tw_error("SetAlienImage was summoned without image string");
	}
	const char* mod = lua_tostring(ls, 1);

	if (music)
	{
		sound.stop_music();
		sound.unload_music(music);
	}

  music = sound.load_music(mod);

  if (!music && sound.is_music_supported())
     tw_error("Couldnt load music");

  if (music) sound.play_music( music, TRUE);

  return 0;
}

int l_DialogWrite(lua_State* ls)
{
  game->window->lock();
  blit(g_btmAlien, game->window->surface, 0, 0, 0, 0, alien_image_x, alien_image_y);
    
  if ( !lua_isstring(ls, 1) )
    {
      tw_error("Summon Write without text");
    }
  const char* text = lua_tostring(ls, 1);

  std::string strText = text;
  std::string strTemp;
  int k = 15;
  while(strText.length() != 0 )
  {
	  std::string strTmp;
	  strTemp = SplitString(strText, 60);
	  textout(game->window->surface, font, strTemp.c_str(), 10, k, -1);
	  strText.erase(strText.begin(), strText.begin() + strTemp.length());
	  k+=15;
  }

  game->window->unlock();
  return 0;
}


int l_KeyPressed (lua_State* ls)
{
	readkey();
	return 0;
}

int l_DialogAnswer(lua_State* ls)
{
  int i;
  int num = lua_gettop(ls); // number of argument
  MENU *pMenuAnswers = new MENU[num+1];
  for( i=0; i<num; i++)
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
  
  show_mouse(game->window->surface);
  int ret = -1;
  while( ret == -1 )
    ret = do_menu(pMenuAnswers, 0, screen->h/2);
  ret++;
  show_mouse(NULL);

  ///////////////////////////////////////////////
  //Free memory
  for(i=0;i<num;i++)
    free(pMenuAnswers[num].text);
  delete[] pMenuAnswers;
  lua_pushnumber(ls, ret);
  return 1;
}

int l_DialogEnd ( lua_State* ls )
{
  // restore pre-dialog image
  game->window->lock();
  blit( g_btmOld, game->window->surface, 0, 0, 0, 0, g_btmOld->w, g_btmOld->h );
  destroy_bitmap( g_btmOld );
  game->window->unlock();
  g_btmOld = NULL;
  
  
  // destroy alien picture
  destroy_bitmap( g_btmAlien );
  g_btmAlien = NULL;

  if (old_music)
  {
	  if (music)
	  {
		  sound.stop_music();
		  sound.unload_music(music);
	  }
	sound.play_music( old_music, TRUE);
  }

  old_music = NULL;
  music = NULL;

  if( !g_bPause )
	  game->unpause();

  return 0;
}
