#include "mapdata.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define GROWBY 32

#define FLAGS_SEARCHED      0x01
#define FLAGS_CANMOVE  0x02


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
}

int mapdata_count(void)
{
   return data.tiles.count;
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
            //printf("Make Captial\n");
            // TODO: Place a capital in a nice place
            // We should have a capital here, so add a capital
            // Grow the data if nessary
            cap = mapdata_addcapital(NULL);

            cap->money = 0;
            cap->x = tile->x;
            cap->y = tile->y;
            tile->entity = e_ME_capital;
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
   size_t i;
   for(i = 0; i < data.caps.count; i++)
   {
      cap = &data.caps.base[i];
      cap->money = amount;
   }
}

void             mapdata_setcanmove(struct maptile * tile, int flag)
{
   if(flag)
   {
      tile->flags |= FLAGS_CANMOVE;
   }
   else
   {
      tile->flags &= ~FLAGS_CANMOVE;
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

void             mapdata_taketile(struct maptile * tile, int new_owner, 
                                  int new_cap_x, int new_cap_y)
{
   size_t i, k;
   struct maptile * ltile;
   struct mapcapital * cap, * largest_cap;
   int old_cap_x, old_cap_y;
   int money;

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
      mapdata_removecapital(i);
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
            // TODO: Pick the correct kind of tree.
            ltile->entity = e_ME_tree;
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
            ltile->entity = e_ME_capital;
            cap = mapdata_addcapital(NULL);
            cap->x = ltile->x;
            cap->y = ltile->y;
            cap->money = 0;
            cap->size = size;
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
         money += cap->money;
         smaller_cap = cap;
         if(largest_cap == NULL || cap->size > largest_cap->size)
         {
            smaller_cap = largest_cap;
            largest_cap = cap;
         }

         if(smaller_cap != NULL)
         {
            // This is a smaller capital
            // So we will remove it from the capital list
            mapdata_removecapital(i);
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
      largest_cap->size = mapdata_paintcapital(ltile);
   }
}

