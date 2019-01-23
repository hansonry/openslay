#ifndef __MAPDATA_H__
#define __MAPDATA_H__

enum mapentity
{
   e_ME_none,
   e_ME_tree,
   e_ME_palmtree,
   e_ME_peasant,
   e_ME_spearman,
   e_ME_knight,
   e_ME_baron,
   e_ME_capital,
   e_ME_castle
};

struct maptile
{
   enum mapentity entity;
   int x;
   int y;
   int owner;
   int cap_x;
   int cap_y;
   int flags;
};

struct mapcapital
{
   int x;
   int y;
   int money;
   int income;
};

void mapdata_init(void);
void mapdata_destroy(void);

int mapdata_count(void);

struct maptile * mapdata_addtile(int x, int y);
struct maptile * mapdata_gettile(int x, int y);
struct maptile * mapdata_getindex(int index);
void             mapdata_removetile(int x, int y);
void             mapdata_clear(void);

void             mapdata_fullclean(void);

struct mapcapital * mapdata_getcapital(int x, int y);



void             mapdata_get6suroundingCoordinates(int x, int y, 
                                                   int * xs_out, int * ys_out);


#endif // __MAPDATA_H__

