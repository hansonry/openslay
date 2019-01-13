#include "gamestate.h"
#include "imagedata.h"
#include "mapdata.h"



static struct imagedata * imagedata;
static SDL_Rect rect;

void gamestate_init(void)
{
   imagedata = imagedata_get();

   mapdata_init();

   (void)mapdata_addtile(0, 0);
   (void)mapdata_addtile(1, 0);
   (void)mapdata_addtile(2, 0);

   (void)mapdata_addtile(0, 1);
   (void)mapdata_addtile(1, 1);
   (void)mapdata_addtile(2, 1);

   (void)mapdata_addtile(0, 2);
   (void)mapdata_addtile(1, 2);
   (void)mapdata_addtile(2, 2);

}

void gamestate_destroy(void)
{
   mapdata_destroy();
}


void gamestate_onenter(void)
{
}

void gamestate_onexit(void)
{
}


void gamestate_update(float dt)
{
}

#define BMFONT_WIDTH      10
#define BMFONT_HEIGHT     24
#define BMFONT_SPACE      4
#define BMFONT_SHEETWIDTH 20
static void drawtext(SDL_Renderer * rend, int x, int y, const char * text)
{
   SDL_Rect dr, sr;
   int sx, sy;

   sr.w = BMFONT_WIDTH;
   sr.h = BMFONT_HEIGHT;
   dr.w = BMFONT_WIDTH;
   dr.h = BMFONT_HEIGHT;
   dr.x = x;
   dr.y = y;
   while(*text != '\0')
   {
      int index;
      // Look at the char and figure out where on the sheet it is;

      if(*text >= ' ' && *text <= '~')
      {
         index = (*text) - ' '; // Compute the offset
      }
      else
      {
         index = '?' - ' ';
      }
      sy = index / BMFONT_SHEETWIDTH; // Compute y
      sx = index - sy * BMFONT_SHEETWIDTH;
      sr.x = sx * BMFONT_WIDTH;
      sr.y = sy * BMFONT_HEIGHT;
      SDL_RenderCopy(rend, imagedata->bitmapfont, &sr, &dr);

      dr.x += BMFONT_WIDTH + BMFONT_SPACE;
      text ++;
   }

}



void gamestate_render(SDL_Renderer * rend)
{
   int i, mapcount;
   struct maptile * tile;
   rect.w = 32; rect.h = 32;

   // Test Render
   rect.x = (800 - rect.w) / 2;
   rect.y = (600 - rect.h) / 2;
   SDL_RenderCopy(rend, imagedata->hex, NULL, &rect);


   // Render tiles
   mapcount = mapdata_count();
   for(i = 0; i < mapcount; i++)
   {
      tile = mapdata_getindex(i);

      // You may notice the off by one, that is due to the odd height 
      // of the hexes
      rect.x = tile->x * 23;
      rect.y = tile->y * 30;
      if(tile->x % 2 == 1)
      {
         rect.y += 15;
      }
      SDL_RenderCopy(rend, imagedata->hex, NULL, &rect);
   }
   


   drawtext(rend, 100, 100, "Hi MOM! 0123456789");



}

void gamestate_event(SDL_Event * event)
{
}

