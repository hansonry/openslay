#include "mapdata.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define GROWBY 32

#define FLAGS_SEARCHED   0x01
#define FLAGS_CANMOVE    0x02

struct mdplayerdata
{
   int owner;
};

struct mdundoevent
{
   int moneypayed;
   int src_x;
   int src_y;
   int dest_x;
   int dest_y;
   int prevcap_x;
   int prevcap_y;
   int dest_owner;
   enum mapentity src_unit;
   enum mapentity dest_unit;
   int caps_count;
   struct mdundocapital
   {
      int destroyed_flag;
      int money;
      enum mapentity replacedunit;
      int x;
      int y;
   } caps[6];
};


struct mapdata
{
   struct mdtilelist
   {
      struct maptile * base;
      size_t size;
      size_t count;
   } tiles;
   struct mdcapitallist
   {
      struct mapcapital * base;
      size_t size;
      size_t count;
   } caps;
   struct mdplayer
   {
      int current;
      struct mdplayerdata * base;
      size_t count;
      size_t size;
   } players;
   struct mdundo
   {
      struct mdundoevent * base;
      size_t count;
      size_t size;
   } undo;
};

static struct mapdata data;

void mapdata_init(void)
{
   data.tiles.size = GROWBY;
   data.tiles.base = malloc(sizeof(struct maptile) * data.tiles.size);
   data.tiles.count = 0;

   data.caps.size = GROWBY;
   data.caps.base = malloc(sizeof(struct mapcapital) * data.caps.size);
   data.caps.count = 0;

   data.players.size = GROWBY;
   data.players.base = malloc(sizeof(struct mdplayerdata) * data.players.size);
   data.players.count = 0;

   data.players.current = 0;

   data.undo.size = GROWBY;
   data.undo.base = malloc(sizeof(struct mdundoevent) * data.undo.size);
   data.undo.count = 0;
}

void mapdata_destroy(void)
{
   free(data.tiles.base);
   data.tiles.base = NULL;
   data.tiles.size = 0;
   data.tiles.count = 0;

   free(data.caps.base);
   data.caps.base = NULL;
   data.caps.size = 0;
   data.caps.count = 0;

   free(data.players.base);
   data.players.base = NULL;
   data.players.size = 0;
   data.players.count = 0;

   free(data.undo.base);
   data.undo.base = NULL;
   data.undo.size = 0;
   data.undo.count = 0;
}

int mapdata_count(void)
{
   return data.tiles.count;
}

int  mapdata_getcurrentplayer(void)
{
   return data.players.current;
}

void mapdata_setcurrentplayer(int player)
{
   if(player >= -1 && player < (int)data.players.count)
   {
      data.players.current = player;
   }
}

int  mapdata_getplayercount(void)
{
   return data.players.count;
}

void mapdata_setplayercount(int count)
{
   if(count >= data.players.size)
   {
      data.players.size = count + GROWBY;
      data.players.base = realloc(data.players.base, 
                                  sizeof(struct mdplayerdata) * 
                                  data.players.size);
   }

   data.players.count = count;
}

void mapdata_setplayerowner(int playerindex, int owner)
{
   if(playerindex >= 0 && playerindex < data.players.count)
   {
      data.players.base[playerindex].owner = owner; 
   }
}

int  mapdata_getplayerowner(int playerindex)
{
   if(playerindex >= 0 && playerindex < data.players.count)
   {
      return data.players.base[playerindex].owner;
   }
   return 0;
}

int  mapdata_getplayerfromowner(int owner)
{
   struct mdplayerdata * d;
   size_t i;

   for(i = 0; i < data.players.count; i++)
   {
      d = &data.players.base[i];
      if(d->owner == owner)
      {
         return i;
      }
   }
   return -1;
}

static struct maptile * mapdata_findtile(int x, int y, size_t * index)
{
   size_t i;
   for(i = 0; i < data.tiles.count; i++)
   {
      if(data.tiles.base[i].x == x &&
         data.tiles.base[i].y == y)
      {
         if(index != NULL)
         {
            *index = i;
         }
         return &data.tiles.base[i];
      }
   }
   return NULL;
}


struct maptile * mapdata_addtile(int x, int y)
{
   struct maptile * tile;
   tile = mapdata_findtile(x, y, NULL);
   if(tile != NULL)
   {
      return tile;
   }

   if(data.tiles.count >= data.tiles.size)
   {
      data.tiles.size = data.tiles.count + GROWBY;
      data.tiles.base = realloc(data.tiles.base, 
                                sizeof(struct maptile) * data.tiles.size);
   }

   tile = &data.tiles.base[data.tiles.count];
   data.tiles.count ++;

   tile->x = x;
   tile->y = y;
   tile->owner = 0;
   tile->entity = e_ME_none;

   return tile;
}

struct maptile * mapdata_gettile(int x, int y)
{
   return mapdata_findtile(x, y, NULL);
}

struct maptile * mapdata_getindex(int index)
{
   if(index >= 0 && index < data.tiles.count)
   {
      return &data.tiles.base[index];
   }
   return NULL;
}

void             mapdata_removetile(int x, int y)
{
   struct maptile * tile;
   size_t rindex;
   tile = mapdata_findtile(x, y, &rindex);
   if(tile != NULL)
   {
      data.tiles.count --;
      memcpy(&data.tiles.base[rindex], 
             &data.tiles.base[data.tiles.count], 
             sizeof(struct maptile));
   }
}

void             mapdata_clear(void)
{
   data.tiles.count = 0;
   data.caps.count = 0;
}


void             mapdata_get6suroundingCoordinates(int x, int y, 
                                                   int * xs_out, int * ys_out)
{
   static int xevenoffsets[] = {  0,  1,  1,  0, -1, -1 };
   static int yevenoffsets[] = { -1, -1,  0,  1,  0, -1 };

   static int xoddoffsets[]  = {  0,  1,  1,  0, -1, -1 };
   static int yoddoffsets[]  = { -1,  0,  1,  1,  1,  0 };

   int * xoffset, * yoffset, i;

   if(x % 2 == 0) // Check for even
   {
      xoffset = xevenoffsets;
      yoffset = yevenoffsets;
   }
   else
   {
      xoffset = xoddoffsets;
      yoffset = yoddoffsets;
   }

   for(i = 0; i < 6; i++)
   {
      xs_out[i] = x + xoffset[i];
      ys_out[i] = y + yoffset[i];
   }


}

static void mapdata_updateupkeep(void)
{
   size_t i, k;
   struct mapcapital * cap;
   struct maptile * tile;
   for(i = 0; i < data.caps.count; i++)
   {
      cap = &data.caps.base[i];
      cap->upkeep = 0;
   }

   for(i = 0; i < data.tiles.count; i++)
   {
      int upkeep;
      tile = &data.tiles.base[i];
      switch(tile->entity)
      {
      default: 
         upkeep = 0;
         break;
      case e_ME_peasant:
         upkeep = 2;
         break;
      case e_ME_spearman:
         upkeep = 6;
         break;
      case e_ME_knight:
         upkeep = 18;
         break;
      case e_ME_baron:
         upkeep = 54;
         break;
      }
      if(upkeep > 0)
      {
         for(k = 0; k < data.caps.count; k++)
         {
            cap = &data.caps.base[k];
            if(cap->x == tile->cap_x && cap->y == tile->cap_y)
            {
               cap->upkeep += upkeep;
               break;
            }
         }
      }
   }
}
static void mapdata_updateincome(void)
{
   size_t i, k;
   struct mapcapital * cap;
   struct maptile * tile;
   for(i = 0; i < data.caps.count; i++)
   {
      cap = &data.caps.base[i];
      cap->income = 0;
   }

   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      switch(tile->entity)
      {
      default: 
         // Do nothing on purpose
         break;
      case e_ME_none:
      case e_ME_capital:
      case e_ME_castle:
      case e_ME_grave: // Not sure if this matches the original game
      case e_ME_peasant:
      case e_ME_spearman:
      case e_ME_knight:
      case e_ME_baron:
         for(k = 0; k < data.caps.count; k++)
         {
            cap = &data.caps.base[k];
            if(cap->x == tile->cap_x && cap->y == tile->cap_y)
            {
               cap->income ++;
               break;
            }
         }
         break;
      }
   }
}

static struct mapcapital * mapdata_addcapital(size_t * index)
{
   struct mapcapital * cap;
   if(data.caps.count >= data.caps.size)
   {
      data.caps.size = data.caps.count + GROWBY;
      data.caps.base = realloc(data.caps.base, 
                               sizeof(struct mapcapital) * data.caps.size);
   }

   cap = &data.caps.base[data.caps.count];

   if(index != NULL)
   {
      *index = data.caps.count;
   }

   data.caps.count ++;
   return cap;
}

static void mapdata_removecapital(size_t index)
{
   data.caps.count --;
   memcpy(&data.caps.base[index], 
          &data.caps.base[data.caps.count], 
          sizeof(struct mapcapital));
}


static int       mapdata_paintcapital(struct maptile * tile)
{
   struct maptile * subtile;
   int next_x[6];
   int next_y[6];
   int i;
   int count = 1;

   //printf("Enter %d, %d\n", tile->x, tile->y);
   // Mark this as searched
   tile->flags |= FLAGS_SEARCHED;

   mapdata_get6suroundingCoordinates(tile->x, tile->y, next_x, next_y);
   for(i = 0; i < 6; i++)
   {
      subtile = mapdata_gettile(next_x[i], next_y[i]);
      //printf("%d, %d\n", next_x[i], next_y[i]);
      if(subtile != NULL &&
         (subtile->flags & FLAGS_SEARCHED) == 0 && // not searched
          subtile->owner == tile->owner)
      {
         // Remove captials
         if(subtile->entity == e_ME_capital)
         {
            subtile->entity = e_ME_none;
         }

         // set captial to parent
         subtile->cap_x = tile->cap_x;
         subtile->cap_y = tile->cap_y;

         count += mapdata_paintcapital(subtile);
      }
   }

   //printf("count: %d\n", count);

   return count;

}

static int mapdata_computecaptialscore(struct maptile * tile)
{
   int score;
   int i;
   int xs[6], ys[6];
   struct maptile * ltile;
   if(tile->entity == e_ME_none)
   {
      // Lets figure out how well defended this thing is
      score = 12;
      mapdata_get6suroundingCoordinates(tile->x, tile->y, xs, ys);
      for(i = 0; i < 6; i++)
      {
         ltile = mapdata_gettile(xs[i], ys[i]);
         if(ltile == NULL)
         {
            score += 6;
         }
         else if(ltile->cap_x == tile->cap_x &&
                 ltile->cap_y == tile->cap_y)
         {
            switch(ltile->entity)
            {
            default:
               // Do nothing on purpose (Add 0)
               break;
            case e_ME_peasant:  score += 1; break;
            case e_ME_spearman: score += 2; break;
            case e_ME_castle:   score += 3; break;
            case e_ME_knight:   score += 4; break;
            case e_ME_baron:    score += 5; break;
            }
         }
         else
         {
            // This is a hostile tile. It's best to put some room between
            score -= 2;
         }

      }

   }
   else
   {
      // Try not to remove anything
      // Even trees, That would give you a benifit
      switch(tile->entity)
      {
      case e_ME_baron:    score = -6; break;
      case e_ME_knight:   score = -5; break;
      case e_ME_castle:   score = -4; break;
      case e_ME_spearman: score = -3; break;
      case e_ME_peasant:  score = -2; break;
      case e_ME_tree:     score = -1; break;
      case e_ME_palmtree: score = -1; break;
      case e_ME_grave:    score = -1; break;
      default:            score = 0;  break;
      }
   }
   return score;
}

// cap_x and cap_y are the current temperary capital of the
// terratory
static struct maptile * mapdata_findgoodcapitalplace(int cap_x, int cap_y)
{
   struct maptile * tile, *best;
   size_t i;
   int bestscore;
   best = NULL;

   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      if(tile->cap_x == cap_x &&
         tile->cap_y == cap_y)
      {
         int thisscore;
         thisscore = mapdata_computecaptialscore(tile);
         if(best == NULL || thisscore > bestscore)
         {
            best = tile;
            bestscore = thisscore;
         }
      }
   }

   return best;
}

static void mapdata_shiftcaptial(int src_x, int src_y, int dest_x, int dest_y)
{
   struct maptile * tile;
   size_t i;

   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      if(tile->cap_x == src_x && tile->cap_y == src_y)
      {
         tile->cap_x = dest_x;
         tile->cap_y = dest_y;
      }
   }
   
}


// In this function we are trusting tile owners, positions, trees, and units
// Everything else this function will clean up
void             mapdata_fullclean(void)
{
   size_t i, k;
   struct maptile * tile;
   struct mapcapital * cap;

   // Verify all capitals and
   // remove capitals that don't match a tile
   
   for(i = data.caps.count - 1; i < data.caps.count; i--)
   {
      cap = &data.caps.base[i];
      tile = mapdata_gettile(cap->x, cap->y);
      if(tile == NULL ||
         tile->entity != e_ME_capital)
      {
         // It's a fake capital, lets remove it
         mapdata_removecapital(i);
      }
   }

   // Go though all tiles and self assign as a capital
   // Set them all as unsearched and moved

   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      tile->cap_x = tile->x;
      tile->cap_y = tile->y;
      tile->flags = FLAGS_CANMOVE;
   }

   // Find each captial and paint out. Remove captials that
   // are in the same area. Also remove captials that don't have a large
   // area
   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      if(tile->entity == e_ME_capital)
      {
         int count;
         count = mapdata_paintcapital(tile);
         if(count <= 1)
         {
            // Area is too small. Remove the building. Also see if we can
            // find a capital data entry and remove that too
            tile->entity = e_ME_none;
            for(k = 0; k <  data.caps.count; k ++)
            {
               cap = &data.caps.base[k];
               if(cap->x == tile->x &&
                  cap->y == tile->y)
               {
                  mapdata_removecapital(k);
                  break;
               }
            }
         }
      }
      
   }


   // Find areas that should have a capital but don't
   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      if(tile->entity != e_ME_capital &&
         tile->cap_x == tile->x &&
         tile->cap_y == tile->y)
      {
         // We have found a possiblity. Let's map the area and
         // see if it has enugh size to support a capital.
         int count;
         count = mapdata_paintcapital(tile);
         if(count > 1)
         {
            struct maptile * best;
            best = mapdata_findgoodcapitalplace(tile->x, tile->y);
            mapdata_shiftcaptial(tile->x, tile->y, best->x, best->y);
            //printf("Make Captial\n");
            // We should have a capital here, so add a capital
            cap = mapdata_addcapital(NULL);

            cap->money = 0;
            cap->x = best->x;
            cap->y = best->y;
            best->entity = e_ME_capital;
         }
      }
   }


   // make sure each capital has a captial data entry
   for(i = 0; i < data.tiles.count; i++)
   {
      
      tile = &data.tiles.base[i];
      if(tile->entity == e_ME_capital)
      {
         int found;

         // Search for matching entry in captial list
         found = 0;
         for(k = 0; k < data.caps.count; k++)
         {
            cap = &data.caps.base[k];
            if(cap->x == tile->cap_x &&
               cap->y == tile->cap_y)
            {
               found = 1;
               break;
            }
         }

         // If we didn't find a matching captial entity,
         // we need to create that entity
         if(found == 0)
         {
            // Grow the data if nessary
            cap = mapdata_addcapital(NULL);

            cap->money = 0;
            cap->x = tile->x;
            cap->y = tile->y;
         }
      }
   }

   // Go though all tiles and set them to unsearched

   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      tile->flags = FLAGS_CANMOVE;
   }

   // Count The area for each captital and setup the income 
   // Also mark the can move flags on the capital
   for(i = 0; i < data.caps.count; i++)
   {
      cap = &data.caps.base[i];
      tile = mapdata_gettile(cap->x, cap->y);
      if(tile == NULL)
      {
         fprintf(stderr, "mapdata_fullclean: Error Unexpected NULL Tile\n");
         return;
      }
      cap->size = mapdata_paintcapital(tile);

      if(cap->money >= 10)
      {
         tile->flags |= FLAGS_CANMOVE;
      }
      else
      {
         tile->flags &= ~FLAGS_CANMOVE;
      }
   }


   // update the incomes
   mapdata_updateincome();
   mapdata_updateupkeep();

   // Count the number of players 
   mapdata_setplayercount(0);
   for(i = 0; i < data.tiles.count; i++)
   {
      int found;
      tile = &data.tiles.base[i];
      found = 0;
      for(k = 0; k < data.players.count; k++)
      {
         if(data.players.base[k].owner == tile->owner)
         {
            found = 1;
            break;
         }
      }

      if(found == 0)
      {
         mapdata_setplayercount(data.players.count + 1);
         data.players.base[data.players.count - 1].owner = tile->owner;
         //printf("Player %d uses owner %d\n", data.players.count -1, tile->owner);
      }

   }
}

struct mapcapital * mapdata_getcapital(int x, int y)
{
   struct mapcapital * cap, *t;
   size_t i;
   cap = NULL;
   for(i = 0; i < data.caps.count; i++)
   {
      t = &data.caps.base[i];
      if(t->x == x && t->y == y)
      {
         cap = t;
         break;
      }
   }
   return cap;
}

void             mapdata_setmoneyallcapitals(int amount)
{
   struct mapcapital * cap;
   struct maptile * tile;
   size_t i;
   for(i = 0; i < data.caps.count; i++)
   {
      cap = &data.caps.base[i];
      cap->money = amount;

      tile = mapdata_gettile(cap->x, cap->y);
      if(tile != NULL)
      {
         if(amount >= 10)
         {
            tile->flags |= FLAGS_CANMOVE;  
         }
         else
         {
            tile->flags &= ~FLAGS_CANMOVE;
         }
      }
   }
}


int              mapdata_getcanmove(struct maptile * tile)
{
   if((tile->flags & FLAGS_CANMOVE) == FLAGS_CANMOVE)
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

int              mapdata_getentitycost(enum mapentity entity)
{
   int cost;
   switch(entity)
   {
   case e_ME_peasant:  cost = 10; break;
   case e_ME_spearman: cost = 20; break;
   case e_ME_knight:   cost = 30; break;
   case e_ME_baron:    cost = 40; break;
   case e_ME_castle:   cost = 25; break;
   default:            cost = 0;  break;
   }
   return cost;
}

static void      mapdata_placetree(struct maptile * tile)
{
   struct maptile * ltile;
   int sx[6], sy[6];
   int i;
   int foundwater;
   
   mapdata_get6suroundingCoordinates(tile->x, tile->y, sx, sy);
   foundwater = 0;
   for(i = 0; i < 6; i++)
   {
      ltile = mapdata_gettile(sx[i], sy[i]);
      if(ltile == NULL)
      {
         foundwater = 1;
         break;
      }
   }

   if(foundwater == 1)
   {
      tile->entity = e_ME_palmtree;
   }
   else
   {
      tile->entity = e_ME_tree;
   }
}


// This function assumes you have not messed with any of the tile
// entities before you run this.
// After this is run you can place any unit in the taken spot.
static void      mapdata_taketile(struct maptile * tile, int new_owner, 
                                  int new_cap_x, int new_cap_y,
                                  struct mdundoevent * event)
{
   size_t i, k;
   struct maptile * ltile;
   struct mapcapital * cap, * largest_cap;
   int old_cap_x, old_cap_y;
   int money;
   size_t largest_cap_index;

   
   // Init Event
   event->caps_count = 0;
   event->prevcap_x = tile->cap_x;
   event->prevcap_y = tile->cap_y;

   // 1. Handel the opponent losses
   // 1.a. Check to see if we split the oppent's terratory

   // start by unmarking all terratory

   for(i = 0; i < data.tiles.count; i++)
   {
      ltile = &data.tiles.base[i];
      ltile->flags &= ~FLAGS_SEARCHED;
   }

   // set the tile to the new owner
   old_cap_x = tile->cap_x;
   old_cap_y = tile->cap_y;

   tile->owner = new_owner;
   tile->cap_x = new_cap_x;
   tile->cap_y = new_cap_y;

   // Check to see if we have killed a capital
   if(tile->entity == e_ME_capital)
   {
      // If we have, we need to remove the capital
      for(i = 0; i < data.caps.count; i++)
      {
         cap = &data.caps.base[i];
         if(cap->x == tile->x &&
            cap->y == tile->y)
         {
            event->caps[event->caps_count].x              = cap->x;
            event->caps[event->caps_count].y              = cap->y;
            event->caps[event->caps_count].money          = cap->money;
            event->caps[event->caps_count].replacedunit   = e_ME_none;
            event->caps[event->caps_count].destroyed_flag = 1;
            event->caps_count ++;
            mapdata_removecapital(i);
            break;
         }
      }
   }
      
   // Search the old owner captial, if we find it, then recalculate
   // the area.
   cap = mapdata_getcapital(old_cap_x, old_cap_y);
   if(cap != NULL)
   {
      ltile = mapdata_gettile(cap->x, cap->y);
      if(ltile != NULL)
      {
         cap->size = mapdata_paintcapital(ltile);
         // Check to see if there is enughf room left to have a capital.
         if(cap->size < 2)
         {
            mapdata_placetree(ltile);

            event->caps[event->caps_count].x              = cap->x;
            event->caps[event->caps_count].y              = cap->y;
            event->caps[event->caps_count].money          = cap->money;
            event->caps[event->caps_count].replacedunit   = ltile->entity;
            event->caps[event->caps_count].destroyed_flag = 1;
            event->caps_count ++;

            // Find the captial and remove it
            for(k = 0; k < data.caps.count; k++)
            {
               if(cap == &data.caps.base[k])
               {
                  mapdata_removecapital(k);
                  break;
               }
            }
            
         }
      }
   }

   // Now there could be some tiles that have the old captial, but are
   // not marked as searched. These tiles need to have a new capital


   for(i = 0; i < data.tiles.count; i++)
   {
      ltile = &data.tiles.base[i];
      if(ltile->cap_x == old_cap_x && 
         ltile->cap_y == old_cap_y &&
         (ltile->flags & FLAGS_SEARCHED) == 0)
      {
         int size;
         ltile->cap_x = ltile->x;
         ltile->cap_y = ltile->y;
         size = mapdata_paintcapital(ltile);
         if(size > 1)
         {
            struct maptile * best;
            best = mapdata_findgoodcapitalplace(ltile->x, ltile->y);
            mapdata_shiftcaptial(ltile->x, ltile->y, best->x, best->y);

            event->caps[event->caps_count].x              = best->x;
            event->caps[event->caps_count].y              = best->y;
            event->caps[event->caps_count].money          = 0;
            event->caps[event->caps_count].replacedunit   = best->entity;
            event->caps[event->caps_count].destroyed_flag = 0;
            event->caps_count ++;

            best->entity = e_ME_capital;
            cap = mapdata_addcapital(NULL);
            cap->x = best->x;
            cap->y = best->y;
            cap->money = 0;
            cap->size = size;
            best->flags &= ~FLAGS_CANMOVE;

         }
      }
   }


   // 2. Handel the gains from the move

   // To start we are going to switch all tiles to point to the capital
   // that the attacking unit came from. We will also remove all the captial
   // tiles 
   ltile = mapdata_gettile(new_cap_x, new_cap_y);
   cap = mapdata_getcapital(new_cap_x, new_cap_y);
   if(cap != NULL && ltile != NULL)
   {
      // This removes all other captial tiles and sets all connected tiles
      // to the source capital
      //printf("Reset all capitals to %d %d\n", cap->x, cap->y);
      (void)mapdata_paintcapital(ltile);
      ltile->entity = e_ME_none;

   }
   else
   {
      fprintf(stderr, "mapdata_taketile: Capital or tile is unexpectedly NULL\n");
   }

   // Now we will loop though all the captials, Find the tiles that are
   // pointing to the original captial. Find the one with the most
   // terratory and set that one
   money = 0;
   largest_cap = NULL;
   for(i = data.caps.count - 1; i < data.caps.count; i--)
   {
      cap = &data.caps.base[i];
      ltile = mapdata_gettile(cap->x, cap->y);
      if(ltile != NULL && 
         ltile->cap_x == new_cap_x && 
         ltile->cap_y == new_cap_y)
      {
         struct mapcapital *smaller_cap;
         int smaller_index;
         money += cap->money;
         smaller_cap = cap;
         smaller_index = i;
         //printf("Found Cap %d %d at size %d\n", cap->x, cap->y, cap->size);
         if(largest_cap == NULL || cap->size > largest_cap->size)
         {
            smaller_cap = largest_cap;
            smaller_index = largest_cap_index;
            largest_cap_index = i;
            largest_cap = cap;
         }

         if(smaller_cap != NULL)
         {
            // A smaller capital has been picked
            // So we will remove it from the capital list
            //printf("Removeing Cap %d %d at size %d\n", smaller_cap->x, smaller_cap->y, smaller_cap->size);
            event->caps[event->caps_count].x              = smaller_cap->x;
            event->caps[event->caps_count].y              = smaller_cap->y;
            event->caps[event->caps_count].money          = smaller_cap->money;;
            event->caps[event->caps_count].replacedunit   = e_ME_none;
            event->caps[event->caps_count].destroyed_flag = 1;
            event->caps_count ++;
            mapdata_removecapital(smaller_index);
         }
         
      }
   }
   
   // Reset all the search tiles once more for a final paint
   for(i = 0; i < data.tiles.count; i++)
   {
      ltile = &data.tiles.base[i];
      ltile->flags &= ~FLAGS_SEARCHED;
   }

   // We now have the larget_capital. Give it all the money and set the 
   // entity to a capital and then repaint everything to this capital coordinat
   largest_cap->money = money;
   ltile = mapdata_gettile(largest_cap->x, largest_cap->y);
   if(ltile != NULL)
   {
      ltile->entity = e_ME_capital;
      ltile->cap_x = ltile->x;
      ltile->cap_y = ltile->y;
      largest_cap->size = mapdata_paintcapital(ltile);

      if(largest_cap->money >= 10)
      {
         ltile->flags |= FLAGS_CANMOVE;
      }
      else
      {
         ltile->flags &= ~FLAGS_CANMOVE;
      }
   }

   mapdata_updateincome();
}

static int mapdata_wincheck(enum mapentity attacker, 
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

static enum mapentity mapdata_sumunit(enum mapentity e1, enum mapentity e2)
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

static int mapdata_commonmove(struct mapcommandresult * result, int owner, 
                              struct maptile * src_tile, 
                              struct maptile * dest_tile,
                              enum mapentity * entity,
                              int * cost, struct mapcapital ** cap)
{
   enum mapentity lentity;
   int sx[6], sy[6];
   struct maptile * stile[6];
   int i;
   int found;

   if(result != NULL)
   {
      result->mapchanged_flag = 0;
   }

   // Check our entity to see if it is valid
   if((*entity) != e_ME_castle && 
      (*entity) != e_ME_peasant &&
      (*entity) != e_ME_spearman &&
      (*entity) != e_ME_knight &&
      (*entity) != e_ME_baron)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_notvalidentity;
      }

      return 0;
   }


   // Check our source entity to see if it is valid
   if(src_tile->entity != e_ME_capital && // used to create units 
      src_tile->entity != e_ME_peasant &&
      src_tile->entity != e_ME_spearman &&
      src_tile->entity != e_ME_knight &&
      src_tile->entity != e_ME_baron)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_sourcenotvalidunit;
      }

      return 0;
   }


   // Check to see if you own the unit
   if(src_tile->owner != owner)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_notyourunit;
      }

      return 0;
   }

   // Check to see if the source unit has move capablities
   if(src_tile->entity != e_ME_capital && // New units can move
      (src_tile->flags & FLAGS_CANMOVE) != FLAGS_CANMOVE)
   {
      
      if(result != NULL)
      {
         result->type = e_MCRT_sourceunitcantmove;
      }
      return 0;
   }

   // Are we just moving a unit or upgrading it too?
   if(src_tile->entity != *entity)
   {

      // We are upgrading or buying the unit. Compute the cost

      (*cost) = mapdata_getentitycost(*entity) -
                mapdata_getentitycost(src_tile->entity);

      // Check to see if we have the money and a capital
      (*cap) = mapdata_getcapital(src_tile->cap_x, src_tile->cap_y);
      if((*cap) == NULL)
      {
         if(result != NULL)
         {
            result->type = e_MCRT_notacapital;
         }
         return 0;
      }


      if((*cap)->money < (*cost))
      {
         if(result != NULL)
         {
            result->type = e_MCRT_notenoughmoney;
         }
         return 0;
      }
   }
   else
   {
      (*cap) = NULL;
      (*cost) = 0;
   }


   // Castle specific stuff
   if((*entity) == e_ME_castle)
   {
      // You can only buy castles, not upgrade to them
      if(src_tile->entity != e_ME_capital)
      {
         if(result != NULL)
         {
            result->type = e_MCRT_cantupgradetocastle;
         }
         return 0;
      }

      // Check to see if we are placing castles in our own territory
      if(src_tile->cap_x != dest_tile->cap_x ||
         src_tile->cap_y != dest_tile->cap_y)
      {
         if(result != NULL)
         {
            result->type = e_MCRT_cantcastleattack;
         }
         return 0;
      }

      // We can only place castls on empty tiles
      if(dest_tile->entity != e_ME_none)
      {
         if(result != NULL)
         {
            result->type = e_MCRT_blocked;
            result->blockedby = dest_tile->entity;
            result->blockedby_x = dest_tile->x;
            result->blockedby_y = dest_tile->y;
         }
         return 0;
      }

      // Ok we can place the castle now
      if(result != NULL)
      {
         result->type = e_MCRT_success;
      }
      return 1;

   }

   // Check to see if we are placing the unit right back down
   if(src_tile == dest_tile)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_success;
      }
      return 1;
   }

   // Check to see if we are moving within our terratory 
   if(src_tile->cap_x == dest_tile->cap_x &&
      src_tile->cap_y == dest_tile->cap_y)
   {
         
      switch(dest_tile->entity)
      {
      case e_ME_none:
         // Nothing here, just move the unit and mark it as good to move again
         dest_tile->flags |= FLAGS_CANMOVE;
         if(result != NULL)
         {
            result->type = e_MCRT_success;
         }
         return 1;
         break;
      case e_ME_tree:
      case e_ME_palmtree:
      case e_ME_grave:
         // Cleanup the blockers and mark the unit as moved
         dest_tile->flags &= ~FLAGS_CANMOVE;
         if(result != NULL)
         {
            result->type = e_MCRT_success;
         }
         return 1;
         break;
      case e_ME_peasant:
      case e_ME_spearman:
      case e_ME_knight:
      case e_ME_baron:
         // Combine the two units. Use the movement state of
         // the destination unit.
         lentity = mapdata_sumunit(*entity, dest_tile->entity);
         if(lentity == e_ME_grave) // Grave indicates larger than barron
         {
            if(result != NULL)
            {
               result->type = e_MCRT_combinedunitabovemax;
            }
            return 0;
         }
         else if(lentity == e_ME_none)
         {
            fprintf(stderr, "mapdata_moveunit: The sum of two units shouldn't be nothing\n");
            if(result != NULL)
            {
               result->type = e_MCRT_error;
            }
            return 0;
         }
         else
         {
            *entity = lentity;

            if(result != NULL)
            {
               result->type = e_MCRT_success;
            }
            return 1;
         }
         break;
      default:
         if(result != NULL)
         {
            result->type = e_MCRT_destblockedbybuilding;
         }
         return 0;
         break;
      }
   }

   // We are not moving onto our terrain if we get here

   // Gather information about the surrounding tiles
   mapdata_get6suroundingCoordinates(dest_tile->x, dest_tile->y, sx, sy);
   for(i = 0; i < 6; i++)
   {
      stile[i] = mapdata_gettile(sx[i], sy[i]);
   }

   // Check to see if we are one away from our attacking terratory
   found = 0;
   for(i = 0; i < 6; i++)
   {
      if(stile[i] != NULL &&
         stile[i]->cap_x == src_tile->cap_x &&
         stile[i]->cap_y == src_tile->cap_y)
      {
         found = 1;
         break;
      }
   }

   if(found == 0)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_notreachable;
      }
      return 0;
   }

   // Can we win a fight with what is in the destination tile
   if(!mapdata_wincheck(*entity, dest_tile->entity))
   {
      if(result != NULL)
      {
         result->type = e_MCRT_blocked;
         result->blockedby = dest_tile->entity;
         result->blockedby_x = dest_tile->x;
         result->blockedby_x = dest_tile->y;
      }
      return 0;
   }

   // Ok we can win that fight, but can we win against anything else near by
   // (Withing their own terratory)
   
   for(i = 0; i < 6; i++)
   {
      if(stile[i] != NULL &&
         dest_tile->cap_x == stile[i]->cap_x && // Check to see if this is the same cap as
         dest_tile->cap_y == stile[i]->cap_y && // the defending tile
         !mapdata_wincheck(*entity, stile[i]->entity)) // See if we lose 
      {
         // So we lost a fight with something one away from the defending tile
         // that is the same capital. So we can make this move
         if(result != NULL)
         {
            result->type = e_MCRT_blocked;
            result->blockedby = stile[i]->entity;
            result->blockedby_x = sx[i];
            result->blockedby_x = sy[i];
         }
         return 0;
      }
   }

   // At this point there is nothing stopping the attacker from taking this
   // spot. So take it.


   dest_tile->flags &= ~FLAGS_CANMOVE;
   
   if(result != NULL)
   {
      result->type = e_MCRT_success;
      result->mapchanged_flag = 1;
   }
   return 1;
}

// Buying the unit set the to_x and to_y to the capital
int  mapdata_moveunit(struct mapcommandresult * result, int owner, 
                      int from_x, int from_y, int to_x, int to_y,
                      enum mapentity entity)
{
   int cost;
   int rvalue;
   struct mapcapital * cap;
   struct maptile * src_tile;
   struct maptile * dest_tile;
   struct maptile * cap_tile;
   struct mdundoevent event;


   event.src_x = from_x;
   event.src_y = from_y;
   event.dest_x = to_x;
   event.dest_y = to_y;

   if(data.players.current != -1 &&
      data.players.base[data.players.current].owner != owner)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_wrongowner;
      }
      return 0;
   }
   

   // Check our source tile to see if it exists
   src_tile = mapdata_gettile(from_x, from_y);
   if(src_tile == NULL)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_sourcenottile;
      }
      return 0;
   }

   // Check the destination tile
   dest_tile = mapdata_gettile(to_x, to_y);
   if(dest_tile == NULL)
   {
      if(result != NULL)
      {
         result->type = e_MCRT_destnottile;
      }
      return 0;
   }

   rvalue = mapdata_commonmove(result, owner, 
                               src_tile, 
                               dest_tile, 
                               &entity, &cost, &cap);

   if(rvalue == 1)
   {

      event.dest_unit = dest_tile->entity;
      event.dest_owner = dest_tile->owner;
      event.src_unit = src_tile->entity;
   
      // Note: The order of this block of code is very important
      // and strange

      // First clear old tile if nessary
      if(src_tile->entity != e_ME_capital)
      {
         src_tile->entity = e_ME_none;
      }


      // Charge the money if nessary
      if(cap != NULL)
      {
         cap->money -= cost;
         event.moneypayed = cost;

         cap_tile = mapdata_gettile(cap->x, cap->y);

         if(cap_tile == NULL)
         {
            fprintf(stderr, "mapdata_moveunit: Error, captial has no tile\n");
         }
         else
         {
            if(cap->money >= 10)
            {
               cap_tile->flags |= FLAGS_CANMOVE;
            }
            else
            {
               cap_tile->flags &= ~FLAGS_CANMOVE;
            }
         }
      }
      else
      {
         event.moneypayed = 0;
      }

      // Take the tile if we land in terrory that isn't ours
      if(dest_tile->owner != owner)
      {
         cap = mapdata_getcapital(dest_tile->x, dest_tile->y);


         mapdata_taketile(dest_tile, src_tile->owner, 
                          src_tile->cap_x, src_tile->cap_y, 
                          &event);
      }

      // Finaly set the entity
      dest_tile->entity = entity;


      mapdata_updateupkeep();

      // Add the event to the undo stack

      if(data.undo.count >= data.undo.size)
      {
         data.undo.size = data.undo.count + GROWBY;
         data.undo.base = realloc(data.undo.base, 
                                  sizeof(struct mdundoevent) * 
                                  data.undo.size);
      }
      memcpy(&data.undo.base[data.undo.count], &event, 
             sizeof(struct mdundoevent));
      data.undo.count ++;
   }

   return rvalue;
}

void mapdata_clearundo(void)
{
   data.undo.count = 0;
}

int  mapdata_endturn(void)
{
   if(data.players.current >= 0)
   {
      data.players.current ++;
      if(data.players.current >= data.players.count)
      {
         data.players.current = 0;
      }

      mapdata_startturn(data.players.base[data.players.current].owner);
      mapdata_clearundo();
   }
   return data.players.current;
}

int mapdata_startturn(int owner)
{
   struct mapcapital * cap;
   struct maptile * tile, * ltile;
   size_t i, k;
   int z;
   int sx[6], sy[6];
   // Grow Trees

   // To Grow trees, we are going to use the seached flag to indicate
   // trees from last round. That way we dont grow a bunch of trees in one
   // round

   for(i = 0; i < data.tiles.count; i++)
   {     
      tile = &data.tiles.base[i];
      if(tile->entity == e_ME_tree ||
         tile->entity == e_ME_palmtree)
      {
         tile->flags |= FLAGS_SEARCHED;
      }
      else
      {
         tile->flags &= ~FLAGS_SEARCHED;
      }
   }

   for(k = 0; k < data.tiles.count; k++)
   {

      tile = &data.tiles.base[k];
      if(tile->owner == owner && 
         (tile->entity == e_ME_none ||
          tile->entity == e_ME_grave))
      {
         // We can grow a tree here
         // Count nearby trees and water
         int palmtree_count;
         int tree_count;
         int water_count;

         palmtree_count = 0;
         tree_count = 0;
         water_count = 0;

         mapdata_get6suroundingCoordinates(tile->x, tile->y, sx, sy);
         for(z = 0; z < 6; z++)
         {
            ltile = mapdata_gettile(sx[z], sy[z]);
            if(ltile == NULL)
            {
               water_count ++;
            }
            else if(ltile->entity == e_ME_tree && 
                    (ltile->flags & FLAGS_SEARCHED) == FLAGS_SEARCHED)
            {
               tree_count ++;
            }
            else if(ltile->entity == e_ME_palmtree &&
                    (ltile->flags & FLAGS_SEARCHED) == FLAGS_SEARCHED)
            {
               palmtree_count ++;
            }
            
         }

         // Now if we have 2 trees, grow a tree.
         // Not sure if we grow a palm tree next to the water or not
         if(tree_count >= 2)
         {
            if(water_count > 0)
            {
               tile->entity = e_ME_palmtree;
            }
            else
            {
               tile->entity = e_ME_tree;
            }
         }
         else if (water_count >= 1 && palmtree_count >= 1)
         {
            // If we are next to water and a palm tree, grow a palmtree
            tile->entity = e_ME_palmtree;
         }
      }
   }

   // Transform Graves into Trees
   for(k = 0; k < data.tiles.count; k++)
   {

      tile = &data.tiles.base[k];
      if(tile->owner == owner &&
         tile->entity == e_ME_grave)
      {
         mapdata_placetree(tile);
      }
   }


   // Loop though all the capitals and find the owners
   for(i = 0; i < data.caps.count; i++)
   {
      cap = &data.caps.base[i];
      tile = mapdata_gettile(cap->x, cap->y);
      if(tile == NULL)
      {
         fprintf(stderr, "mapdata_startturn: Unxpected Null Tile\n");
         return 0;
      }

      if(tile->owner == owner)
      {
         // TODO: Maybe move this into it's own function


         // Add the income to the money
         cap->money += cap->income;

         // Check to see if we are bankruped
         if(cap->money < cap->upkeep)
         {
            // The terratory went bankrup everyone dies
            cap->money = 0;
            tile->flags &= ~FLAGS_CANMOVE;

            for(k = 0; k < data.tiles.count; k++)
            {
               ltile = &data.tiles.base[k];
               if((ltile->entity == e_ME_peasant ||
                   ltile->entity == e_ME_spearman ||
                   ltile->entity == e_ME_knight ||
                   ltile->entity == e_ME_baron) &&
                   ltile->cap_x == cap->x &&
                   ltile->cap_y == cap->y)
               {
                  ltile->entity = e_ME_grave;
               }
            }
         }
         else
         {
            // Yay we have money to live, subract the upkeep from the money
            cap->money -= cap->upkeep;
            if(cap->money >= 10)
            {
               tile->flags |= FLAGS_CANMOVE;
            }
            else
            {
               tile->flags &= ~FLAGS_CANMOVE;
            }
         }

      }
   }

   // Kill all units that don't have a capital
   for(k = 0; k < data.tiles.count; k++)
   {
      ltile = &data.tiles.base[k];
      if(ltile->owner == owner &&
         (ltile->entity == e_ME_peasant ||
          ltile->entity == e_ME_spearman ||
          ltile->entity == e_ME_knight ||
          ltile->entity == e_ME_baron))
      {
         cap = mapdata_getcapital(ltile->cap_x, ltile->cap_y);
         if(cap == NULL)
         {
            ltile->entity = e_ME_grave;
         }
      }
   }

   // Finaly give all the units move positions
   for(k = 0; k < data.tiles.count; k++)
   {

      ltile = &data.tiles.base[k];
      if(ltile->owner == owner &&
         (ltile->entity == e_ME_peasant ||
          ltile->entity == e_ME_spearman ||
          ltile->entity == e_ME_knight ||
          ltile->entity == e_ME_baron))
      {
         ltile->flags |= FLAGS_CANMOVE;
      }
   }

   mapdata_updateupkeep();
   mapdata_updateincome();
   return 1;
}

int mapdata_undomove(void)
{
   struct mdundoevent * event;
   struct maptile * src_tile, * dest_tile, * tile;
   struct mapcapital * cap;
   int src_cap_x, src_cap_y;
   size_t i, k;

   int money;


   // Grab the lastest event
   if(data.undo.count == 0)
   {
      return 0;
   }

   data.undo.count --;
   event = &data.undo.base[data.undo.count];

   // Grab the source and destiation tiles
   src_tile  = mapdata_gettile(event->src_x,  event->src_y);
   dest_tile = mapdata_gettile(event->dest_x, event->dest_y);

   if(src_tile == NULL || dest_tile == NULL)
   {
      fprintf(stderr, "mapdata_undomove: Null Source or Destination Tile\n");
   }

   src_cap_x = src_tile->cap_x;
   src_cap_y = src_tile->cap_y;

   money = 0;

   // Check to see if we captured area

   if(dest_tile->owner != event->dest_owner)
   {
      // We did capture area

      // Clear the search flag
      for(i = 0; i < data.tiles.count; i++)
      {
         tile = &data.tiles.base[i];
         tile->flags &= ~FLAGS_SEARCHED;
      }

      // Give it back it's tile
      dest_tile->owner = event->dest_owner;
      dest_tile->cap_x = event->prevcap_x;
      dest_tile->cap_y = event->prevcap_y;


      // Destroy all built captials
      for(i = 0; i < event->caps_count; i++)
      {
         struct mdundocapital * ucap;
         ucap = &event->caps[i];
         if(ucap->destroyed_flag == 0)
         {
            // Remove the capital from capitals
            for(k = 0; k < data.caps.count; k++)
            {
               cap = &data.caps.base[k];
               if(cap->x == ucap->x &&
                  cap->y == ucap->y)
               {
                  mapdata_removecapital(k);
                  break;
               }
            }

            // Find the tile and return it
            tile = mapdata_gettile(ucap->x, ucap->y);
            if(tile != NULL)
            {
               tile->entity = ucap->replacedunit;
            }

         }
      }

      // Paint the old capital that we took
      cap = mapdata_getcapital(event->prevcap_x, event->prevcap_y);
      if(cap != NULL) // May be null if we took a tile without a capital
      {
         tile = mapdata_gettile(cap->x, cap->y);
         cap->size = mapdata_paintcapital(tile);
      }

      // Rebuild all destroyed capitals 
      for(i = 0; i < event->caps_count; i++)
      {
         struct mdundocapital * ucap;
         ucap = &event->caps[i];
         if(ucap->destroyed_flag == 1)
         {
            cap = mapdata_addcapital(NULL);
            cap->x = ucap->x;
            cap->y = ucap->y;
            cap->money = ucap->money;

            if(ucap->money > 0)
            {
               // To get here this captial needed to be combined with the
               // attacking captial. So add this to the money pile
               money += ucap->money;
            }

            tile = mapdata_gettile(cap->x, cap->y);

            tile->cap_x = cap->x;
            tile->cap_y = cap->y;
            cap->size = mapdata_paintcapital(tile);
            tile->entity = e_ME_capital;
            //printf("Restored %d %d\n", cap->x, cap->y);

         }
      }

      // Resize owner of original lands so that is is accurate
      cap = mapdata_getcapital(src_cap_x, src_cap_y);
      tile = mapdata_gettile(src_cap_x, src_cap_y);
      if(tile == NULL || cap == NULL)
      {
         fprintf(stderr, "mapdata_undomove: Null capital and capital tile\n");
      }
      cap->size = mapdata_paintcapital(tile);

      mapdata_updateincome();
   }


   // Restor the tiles
   dest_tile->entity = event->dest_unit;
   src_tile->entity = event->src_unit;

   // Refund the attacking cappital if nessary
   cap = mapdata_getcapital(src_tile->cap_x, src_tile->cap_y);
   if(cap == NULL)
   {
      fprintf(stderr, "mapdata_undomove: Null capital\n");
   }

   // Handle Money of the attacing capital
   cap->money += event->moneypayed;
   tile = mapdata_gettile(cap->x, cap->y);
   if(tile == NULL)
   {
      fprintf(stderr, "mapdata_undomove: Null capital tile\n");
   }

   if(cap->money >= 10)
   {
      tile->flags |= FLAGS_CANMOVE;
   }
   else
   {
      tile->flags &= ~FLAGS_CANMOVE;
   }


   // Handle money for the captial that was merged, but now is split
  
   if(money > 0)
   {
      cap = mapdata_getcapital(src_cap_x, src_cap_y);
      tile = mapdata_gettile(src_cap_x, src_cap_y);
      if(tile == NULL || cap == NULL)
      {
         fprintf(stderr, "mapdata_undomove: Null capital and capital tile\n");
      }
      cap->money -= money;
      if(cap->money >= 10)
      {
         tile->flags |= FLAGS_CANMOVE;
      }
      else
      {
         tile->flags &= ~FLAGS_CANMOVE;
      }
   }
   
   mapdata_updateupkeep();
   return 1;
}


int mapdata_cancurrentplayermove(void)
{
   int owner;
   size_t i;
   struct maptile * tile;
   owner = mapdata_getplayerowner(data.players.current);

   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      if((tile->owner == owner || 
          data.players.current == -1) &&
         (tile->flags | FLAGS_CANMOVE) == FLAGS_CANMOVE &&
         (tile->entity == e_ME_capital ||
          tile->entity == e_ME_peasant ||
          tile->entity == e_ME_spearman ||
          tile->entity == e_ME_knight ||
          tile->entity == e_ME_baron))
      {
         return 1;      
      }
   }
   return 0;
}

