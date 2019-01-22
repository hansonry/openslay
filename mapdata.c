#include "mapdata.h"
#include <stdlib.h>
#include <string.h>


#define GROWBY 32

#define FLAGS_SEARCHED      0x01
#define FLAGS_HASACTIVATED  0x02


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
   static int yoddoffsets[]  = { -1,  0,  1,  1, -1,  0 };

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
         data.caps.count --;
         memcpy(&data.caps.base[i], 
                &data.caps.base[data.caps.count], 
                sizeof(struct mapcapital));
      }
   }

   // Go though all tiles and self assign as a capital
   // Set them all as unsearched and moved

   for(i = 0; i < data.tiles.count; i++)
   {
      tile = &data.tiles.base[i];
      tile->cap_x = tile->x;
      tile->cap_y = tile->y;
      tile->flags = FLAGS_HASACTIVATED;
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
                  data.caps.count --;
                  memcpy(&data.caps.base[k], 
                         &data.caps.base[data.caps.count], 
                         sizeof(struct mapcapital));
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
            if(data.caps.count >= data.caps.size)
            {
               data.caps.size = data.caps.count + GROWBY;
               data.caps.base = realloc(data.caps.base, 
                                        sizeof(struct mapcapital) * data.caps.size);
            }

            cap = &data.caps.base[data.caps.count];
            data.caps.count ++;

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
            if(data.caps.count >= data.caps.size)
            {
               data.caps.size = data.caps.count + GROWBY;
               data.caps.base = realloc(data.caps.base, 
                                        sizeof(struct mapcapital) * data.caps.size);
            }

            cap = &data.caps.base[data.caps.count];
            data.caps.count ++;

            cap->money = 0;
            cap->x = tile->x;
            cap->y = tile->y;
         }
      }
   }
}


