#include "gamestate.h"
#include "imagedata.h"
#include "mapdata.h"
#include <stdlib.h>
#include <time.h>

static struct SDL_Color teamcolors[] =
{
   {0x66, 0x99, 0x00, 0xFF},
   {0x00, 0x99, 0x80, 0xFF},
   {0x99, 0x80, 0x00, 0xFF},
   {0x80, 0x00, 0x00, 0xFF},
   {0x99, 0x33, 0x1A, 0xFF}
};

static struct imagedata * imagedata;
static SDL_Rect rect;


static void gamestate_randomizemap(void)
{
   int r;
   int i;
   struct maptile * tile;
   srand(time(NULL));


   for(i = 0; i < mapdata_count(); i++)
   {   
      tile = mapdata_getindex(i);
      r = rand();
      tile->owner = (r % 5);
   }

}

void gamestate_init(void)
{
   imagedata = imagedata_get();

   mapdata_init();

   mapdata_addtile(0, 0)->owner = 0;
   mapdata_addtile(1, 0)->owner = 1;
   mapdata_addtile(2, 0)->owner = 2;
   mapdata_addtile(3, 0)->owner = 3;
   mapdata_addtile(4, 0)->owner = 4;
   (void)mapdata_addtile(5, 0);

   mapdata_addtile(0, 1)->entity = e_ME_capital;
   mapdata_addtile(1, 1)->entity = e_ME_castle;
   mapdata_addtile(2, 1)->entity = e_ME_peasant;
   mapdata_addtile(3, 1)->entity = e_ME_spearman;
   mapdata_addtile(4, 1)->entity = e_ME_knight;
   mapdata_addtile(5, 1)->entity = e_ME_baron;

   mapdata_addtile(0, 2)->entity = e_ME_tree;
   mapdata_addtile(1, 2)->entity = e_ME_palmtree;
   (void)mapdata_addtile(2, 2);

   gamestate_randomizemap();

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

   mapcount = mapdata_count();

   // Render tiles
   for(i = 0; i < mapcount; i++)
   {
      tile = mapdata_getindex(i);

      // You may notice the off by one, that is due to the odd height 
      // of the hexes; These numbers are also trial and error
      // These are put into an odd-q vertical layout.
      // See www.redblobgames.com/grids/hexagons/
      rect.x = tile->x * 23;
      rect.y = tile->y * 30;
      if(tile->x % 2 == 1)
      {
         rect.y += 15;
      }

      SDL_SetTextureColorMod(imagedata->hex, 
                             teamcolors[tile->owner].r,
                             teamcolors[tile->owner].g,
                             teamcolors[tile->owner].b);
      SDL_RenderCopy(rend, imagedata->hex, NULL, &rect);
   }

  
  // Decorate the tiles 
   
   for(i = 0; i < mapcount; i++)
   {
      SDL_Rect sr, dr;
      SDL_Texture * text;
      
      tile = mapdata_getindex(i);

      sr.w = sr.h = 32;
      sr.x = sr.y = 0;
      dr.w = dr.h = 32;

      dr.x = tile->x * 23;
      dr.y = tile->y * 30;
      if(tile->x % 2 == 1)
      {
         dr.y += 15;
      }

      switch(tile->entity)
      {
      default:
         text = NULL;
         break;
      case e_ME_tree:
         text = imagedata->trees;
         break;
      case e_ME_palmtree:
         text = imagedata->trees;
         sr.x = 32;
         break;
      case e_ME_peasant:
         text = imagedata->unit;
         SDL_SetTextureColorMod(imagedata->unit, 0xFF, 0xFF, 0xFF);
         break;
      case e_ME_spearman:
         text = imagedata->unit;
         SDL_SetTextureColorMod(imagedata->unit, 0x34, 0x98, 0xDB);
         break;
      case e_ME_knight:
         text = imagedata->unit;
         SDL_SetTextureColorMod(imagedata->unit, 0xE6, 0x7E, 0x22);
         break;
      case e_ME_baron:
         text = imagedata->unit;
         SDL_SetTextureColorMod(imagedata->unit, 0xD0, 0x29, 0x1B);
         break;
      case e_ME_capital:
         text = imagedata->capital;
         break;
      case e_ME_castle:
         text = imagedata->castle;
         break;
      }
      if(text != NULL)
      {
         SDL_RenderCopy(rend, text, &sr, &dr);
      }
   }


   drawtext(rend, 100, 100, "Hi MOM! 0123456789");



}

void gamestate_event(SDL_Event * event)
{
}

