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
static int capital_selected, cap_x, cap_y;

struct grabbed
{
// This will be used to determine what unit is grabbed by the mouse
// only valid values will be e_ME_none e_ME_peasant e_ME_spearman
// e_ME_knight e_ME_baron
   enum mapentity entity;
   int cap_x;
   int cap_y;
   int owner;
};

static struct grabbed grabbed;

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

   mapdata_setmoneyallcapitals(10);

}

void gamestate_destroy(void)
{
   mapdata_destroy();
}


void gamestate_onenter(void)
{
   gamestate_updatehexmouse();
   capital_selected = 0;
   grabbed.entity = e_ME_none;
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

static void gamestate_renderui(SDL_Renderer * rend)
{
   char text[128];
   struct mapcapital * cap;

   if(capital_selected)
   {
      cap = mapdata_getcapital(cap_x, cap_y);
      if(cap != NULL)
      {
         sprintf(text, "Money: %d", cap->money);
         drawtext(rend, 400, 20, text);
         sprintf(text, "Size: %d", cap->size);
         drawtext(rend, 400, 48, text);
      }
   }
}

static SDL_Texture * gamestate_getUnitTexture(enum mapentity entity, SDL_Rect * sr)
{
   SDL_Texture * text;
   SDL_Rect isr;
   isr.x = isr.y = 0;
   isr.w = isr.h = 32;
   switch(entity)
   {
   default:
      text = NULL;
      break;
   case e_ME_tree:
      text = imagedata->trees;
      break;
   case e_ME_palmtree:
      text = imagedata->trees;
      isr.x = 32;
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
   if(sr != NULL)
   {
      sr->x = isr.x;
      sr->y = isr.y;
      sr->w = isr.w;
      sr->h = isr.h;
   }
   return text;
}

void gamestate_render(SDL_Renderer * rend)
{
   int i, mapcount;
   struct maptile * tile;
   SDL_Rect sr, dr;
   SDL_Texture * text;


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
      
      tile = mapdata_getindex(i);

      dr.w = dr.h = 32;

      gamestate_hexposition(tile->x, tile->y, &dr);


      text = gamestate_getUnitTexture(tile->entity, &sr);
      if(text != NULL)
      {
         SDL_RenderCopy(rend, text, &sr, &dr);
      }
   }


   drawtext(rend, 400, 100, "Hi MOM! 0123456789");



   gamestate_rendermousedebug(rend);

   // Render Grabbed

   if(grabbed.entity != e_ME_none)
   {
      text = gamestate_getUnitTexture(grabbed.entity, &sr);
      if(text != NULL)
      {
         SDL_GetMouseState(&dr.x, &dr.y);
         SDL_RenderCopy(rend, text, &sr, &dr);
      }
   }

   // Render UI
   gamestate_renderui(rend);


}

static int gamestate_maptilehascaptital(struct maptile * tile)
{
   if(tile->x == tile->cap_x && tile->y == tile->cap_y)
   {
      if(tile->entity == e_ME_capital)
      {
         return 1;
      }
      else
      {
         return 0;
      }
   }
   else
   {
      // Problably has a capital because this territory has more than
      // one tile
      return 1;
   }
}

static int gamestate_wincheck(enum mapentity attacker, 
                              enum mapentity defender)
{
   int win;
   if(attacker == e_ME_baron)
   {
      switch(defender)
      {
      default:
         win = 1;
         break;
      case e_ME_baron:
         win = 0;
         break;
      }
   }
   else if(attacker == e_ME_knight)
   {
      switch(defender)
      {
      default:
         win = 1;
         break;
      case e_ME_baron:
      case e_ME_knight:
         win = 0;
         break;
      }
   }
   else if(attacker == e_ME_spearman)
   {
      switch(defender)
      {
      default:
         win = 1;
         break;
      case e_ME_baron:
      case e_ME_knight:
      case e_ME_spearman:
      case e_ME_castle:
         win = 0;
         break;
      }
   }
   else if(attacker == e_ME_peasant)
   {
      switch(defender)
      {
      default:
         win = 1;
         break;
      case e_ME_baron:
      case e_ME_knight:
      case e_ME_spearman:
      case e_ME_castle:
      case e_ME_peasant:
      case e_ME_capital:
         win = 0;
         break;
      }
   }
   else
   {
      win = 0;
   }
   return win;
}

static enum mapentity gamestate_sumunit(enum mapentity e1, enum mapentity e2)
{
   enum mapentity result;
   enum mapentity e[2];
   int v[2];
   int vr;
   int i;
   
   e[0] = e1;
   e[1] = e2;
   for(i = 0; i < 2; i++)
   {
      switch(e[i])
      {
      default:            v[i] = 0; break;
      case e_ME_peasant:  v[i] = 1; break; 
      case e_ME_spearman: v[i] = 2; break; 
      case e_ME_knight:   v[i] = 3; break; 
      case e_ME_baron:    v[i] = 4; break; 
      }
   }

   vr = v[0] + v[1];

   switch(vr)
   {
   case 0:  result = e_ME_none;     break;
   case 1:  result = e_ME_peasant;  break;
   case 2:  result = e_ME_spearman; break;
   case 3:  result = e_ME_knight;   break;
   case 4:  result = e_ME_baron;    break;
   default: result = e_ME_grave;    break;
   }

   return result;
}

static void gamestate_attempttoplace(struct maptile * tile)
{
   int sx[6];
   int sy[6];
   int i;
   int win;
   int canmove_flag;
   struct maptile * ntile;
   enum mapentity entity;

   if(tile->cap_x == grabbed.cap_x && 
      tile->cap_y == grabbed.cap_y)
   {
      switch(tile->entity)
      {
      case e_ME_none:
         mapdata_setcanmove(tile, 1);
         tile->entity = grabbed.entity;
         grabbed.entity = e_ME_none;
         break;
      case e_ME_tree:
      case e_ME_palmtree:
      case e_ME_grave:
         mapdata_setcanmove(tile, 0);
         tile->entity = grabbed.entity;
         grabbed.entity = e_ME_none;
         break;
      case e_ME_peasant:
      case e_ME_spearman:
      case e_ME_knight:
      case e_ME_baron:
         
         entity = gamestate_sumunit(tile->entity, grabbed.entity);
         if(entity != e_ME_grave && entity != e_ME_none)
         {
            tile->entity = entity;
            grabbed.entity = e_ME_none;
         }
         break;
      default:
         // Do Nothing on purpose
         // Not a valid spot to drop a unit
         break;
      }
   }
   else
   {
      mapdata_get6suroundingCoordinates(tile->x, tile->y, sx, sy);

      // Check to see if the move is possible
      win = gamestate_wincheck(grabbed.entity, tile->entity);
      if(win == 1)
      {
         canmove_flag = 1;
         for(i = 0; i < 6; i++)
         {
            ntile = mapdata_gettile(sx[i], sy[i]);
            if(ntile == NULL)
            {
               continue;
            }
            win = gamestate_wincheck(grabbed.entity, ntile->entity);
            if(win == 0 && ntile->owner == tile->owner)
            {
               canmove_flag = 0;
               break;
            }
            
         }
      }
      else
      {
         canmove_flag = 0;
      }

      if(canmove_flag == 1)
      {
         mapdata_taketile(tile, grabbed.owner, grabbed.cap_x, grabbed.cap_y);
         tile->cap_x = grabbed.cap_x;
         tile->cap_y = grabbed.cap_y;
         tile->entity = grabbed.entity;
         mapdata_setcanmove(tile, 0);

         grabbed.entity = e_ME_none;

      }

   }
}

static void gamestate_eventleftmouse(int button_state)
{
   struct maptile * tile;
   if(button_state == SDL_PRESSED)
   {
      tile = mapdata_gettile(hexmouse_x, hexmouse_y);
      if(grabbed.entity == e_ME_none)
      {
         if(tile != NULL && gamestate_maptilehascaptital(tile))
         {
            capital_selected = 1;
            cap_x = tile->cap_x;
            cap_y = tile->cap_y;
            if((tile->entity == e_ME_peasant ||
                tile->entity == e_ME_spearman ||
                tile->entity == e_ME_knight ||
                tile->entity == e_ME_baron) &&
                mapdata_getcanmove(tile) == 1)
            {
               grabbed.entity = tile->entity;
               grabbed.owner = tile->owner;
               grabbed.cap_x = tile->cap_x;
               grabbed.cap_y = tile->cap_y;

               tile->entity = e_ME_none;
            }
         }
      }
      else
      {
         if(tile != NULL)
         {
            gamestate_attempttoplace(tile);
         }
      }
   }
   else if(button_state == SDL_RELEASED)
   {
   }
}

static void gamestate_eventrightmouse(int button_state)
{
   struct mapcapital * cap;
   struct maptile * tile;
   if(button_state == SDL_PRESSED)
   {
      if(capital_selected == 1)
      {
         cap = mapdata_getcapital(cap_x, cap_y);
         tile = mapdata_gettile(cap_x, cap_y);
         if(cap != NULL && cap->money >= 10)
         {
            cap->money -= 10;
            grabbed.entity = e_ME_peasant;
            grabbed.cap_x = cap_x;
            grabbed.cap_y = cap_y;
            if(tile != NULL)
            {
               grabbed.owner = tile->owner;
            }
         }
      }
   }
}

void gamestate_event(SDL_Event * event)
{
   if(event->type == SDL_MOUSEMOTION)
   {
      gamestate_updatehexmouse();
   }

   if(event->type == SDL_MOUSEBUTTONDOWN ||
      event->type == SDL_MOUSEBUTTONUP)
   {
      gamestate_updatehexmouse();
      if(event->button.button == SDL_BUTTON_LEFT)
      {
         gamestate_eventleftmouse(event->button.state);
      }
      else if(event->button.button == SDL_BUTTON_RIGHT)
      {
         gamestate_eventrightmouse(event->button.state);
      }
   }
}

