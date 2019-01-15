#include "mapdata.h"
#include <stdlib.h>
#include <string.h>


#define GROWBY 32



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
}

void mapdata_destroy(void)
{
   free(data.tiles.base);
   data.tiles.base = NULL;
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
}

