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
static int hexmouse_x, hexmouse_y;


static void gamestate_hexposition(int x, int y, SDL_Rect * dest)
{

   // You may notice the off by one, that is due to the odd height 
   // of the hexes; These numbers are also trial and error
   // These are put into an odd-q vertical layout.
   // See www.redblobgames.com/grids/hexagons/
   dest->x = x * 23;
   dest->y = y * 30;
   if(x % 2 == 1)
   {
      dest->y += 15;
   }
}

int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
   int i, j, c = 0;
   for (i = 0, j = nvert-1; i < nvert; j = i++) 
   {
      if ( ((verty[i] > testy) != (verty[j] > testy)) &&
           (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
      {
         c = !c;
      }
   }
   return c;
}

static int gamestate_inhex(int x, int y, int tilex, int tiley)
{
   SDL_Rect r;
   float hx[] = {8, 23, 31, 23,  8, 0  };
   float hy[] = {1,  1, 16, 31, 31, 16 };
   gamestate_hexposition(tilex, tiley, &r);

   // A sneeky thing we are doing here is to translate
   // the coordinates of the point based on the tile position
   if(pnpoly(6, hx, hy, x - r.x, y - r.y))
   {
      return 1;
   }
   return 0;
}

static void gamestate_updatehexmouse(void)
{
   int x, y;

   int ex, ey;
   int px[6], py[6];
   int i;

   // 23 wide and 30 tall

   SDL_GetMouseState(&x, &y);

   // Estimate y and x
   ex = x / 23;
   if(ex % 2 == 0)
   {
      ey = y / 30;
   }
   else
   {
      ey = (y - 15) / 30;
   }

   // Using our estamets lets pick the nebors
   mapdata_get6suroundingCoordinates(ex, ey, px, py);

   // Assume the estimate if no hits are found
   hexmouse_x = ex;
   hexmouse_y = ey;

   for(i = 0; i < 6; i++)
   {
      if(gamestate_inhex(x, y, px[i], py[i]))
      {
         hexmouse_x = px[i];
         hexmouse_y = py[i];
         break;
      }
   }

}

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
   int x, y;
   imagedata = imagedata_get();

   mapdata_init();

   /*
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
   */

   // Create a random square map
   for(y = 0; y < 15; y ++)
   {
      for(x = 0; x < 15; x++)
      {
         (void)mapdata_addtile(x, y);
      }
   }

   gamestate_randomizemap();

   mapdata_fullclean();

}

void gamestate_destroy(void)
{
   mapdata_destroy();
}


void gamestate_onenter(void)
{
   gamestate_updatehexmouse();
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


static void gamestate_rendermousedebug(SDL_Renderer * rend)
{
   SDL_Rect dr;
   // Test Render Mouse location

   dr.w = dr.h = 32;
   gamestate_hexposition(hexmouse_x, hexmouse_y, &dr);
   SDL_SetTextureColorMod(imagedata->hex, 0xFF, 0xFF, 0xFF);
   SDL_SetTextureAlphaMod(imagedata->hex, 0xA0);
   SDL_RenderCopy(rend, imagedata->hex, NULL, &dr);
   SDL_SetTextureAlphaMod(imagedata->hex, 0xFF);
}

void gamestate_render(SDL_Renderer * rend)
{
   int i, mapcount;
   struct maptile * tile;
   SDL_Rect sr, dr;


   mapcount = mapdata_count();

   // Render tiles
   for(i = 0; i < mapcount; i++)
   {
      tile = mapdata_getindex(i);

      dr.w = dr.h = 32;
      gamestate_hexposition(tile->x, tile->y, &dr);

      SDL_SetTextureColorMod(imagedata->hex, 
                             teamcolors[tile->owner].r,
                             teamcolors[tile->owner].g,
                             teamcolors[tile->owner].b);
      SDL_RenderCopy(rend, imagedata->hex, NULL, &dr);
   }

  
  // Decorate the tiles 
   
   for(i = 0; i < mapcount; i++)
   {
      SDL_Texture * text;
      
      tile = mapdata_getindex(i);

      sr.w = sr.h = 32;
      sr.x = sr.y = 0;
      dr.w = dr.h = 32;

      gamestate_hexposition(tile->x, tile->y, &dr);


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


   drawtext(rend, 400, 100, "Hi MOM! 0123456789");



   gamestate_rendermousedebug(rend);



}

void gamestate_event(SDL_Event * event)
{
   if(event->type == SDL_MOUSEMOTION)
   {
      gamestate_updatehexmouse();
   }
}

