#include "mapdata.h"
#include <stdlib.h>
#include <string.h>


#define GROWBY 32

struct mapdata
{
   struct maptile * base;
   size_t size;
   size_t count;
};

static struct mapdata data;

void mapdata_init(void)
{
   data.size = GROWBY;
   data.base = malloc(sizeof(struct maptile) * data.size);
   data.count = 0;
}

void mapdata_destroy(void)
{
   free(data.base);
   data.base = NULL;
}

int mapdata_count(void)
{
   return data.count;
}

static struct maptile * mapdata_find(int x, int y, size_t * index)
{
   size_t i;
   for(i = 0; i < data.count; i++)
   {
      if(data.base[i].x == x &&
         data.base[i].y == y)
      {
         if(index != NULL)
         {
            *index = i;
         }
         return &data.base[i];
      }
   }
   return NULL;
}


struct maptile * mapdata_addtile(int x, int y)
{
   struct maptile * tile;
   tile = mapdata_find(x, y, NULL);
   if(tile != NULL)
   {
      return tile;
   }

   if(data.count >= data.size)
   {
      data.size = data.count + GROWBY;
      data.base = realloc(data.base, sizeof(struct maptile) * data.size);
   }

   tile = &data.base[data.count];
   data.count ++;

   tile->x = x;
   tile->y = y;
   tile->owner = 0;
   tile->entity = e_ME_none;

   return tile;
}

struct maptile * mapdata_gettile(int x, int y)
{
   return mapdata_find(x, y, NULL);
}

struct maptile * mapdata_getindex(int index)
{
   if(index >= 0 && index < data.count)
   {
      return &data.base[index];
   }
   return NULL;
}

void             mapdata_removetile(int x, int y)
{
   struct maptile * tile;
   size_t rindex;
   tile = mapdata_find(x, y, &rindex);
   if(tile != NULL)
   {
      data.count --;
      memcpy(&data.base[rindex], 
             &data.base[data.count], 
             sizeof(struct maptile));
   }
}

void             mapdata_clear(void)
{
   data.count = 0;
}

