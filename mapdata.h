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
   e_ME_grave,
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
   int upkeep;
   int size;
};

enum mapcommandresulttype
{
   e_MCRT_success, // Everything was good
   e_MCRT_wrongowner, // The owner does not match the current player
   e_MCRT_notreachable, // You attempted to place a unit on a spot it can't get to
   e_MCRT_notacapital, // The capital you provided is actualy not a capital
   e_MCRT_notenoughmoney, // Not enugh money to do that thing
   e_MCRT_blocked, // Your command is prevent because a stronger unit is blocking it
   e_MCRT_notyourunit, // You attempted to command a unit that is not yours
   e_MCRT_sourcenottile, // You attempt to command something that is not even a tile
   e_MCRT_notvalidentity, // You attempted to command blades of grass (something that is not a unit)
   e_MCRT_sourcenotvalidunit, // Your source is not a valid unit
   e_MCRT_destnottile, // You attempt to move something onto a non-existant tile
   e_MCRT_destblockedbybuilding, // You attempted to move your unit onto a capital or castle
   e_MCRT_sourceunitcantmove, // The unit you attempted to move has alread moved this turn
   e_MCRT_combinedunitabovemax, // The resulting unit would be above barron.
   e_MCRT_cantupgradetocastle, // You tried to upgrade a unit to a castle
   e_MCRT_cantcastleattack, // You attempted to place a casle outside you terratory
   e_MCRT_error // A logical error has occured, this should never happen.
};

struct mapcommandresult
{
   enum mapcommandresulttype type;
   enum mapentity blockedby;
   int blockedby_x;
   int blockedby_y;
   int mapchanged_flag;
};

void mapdata_init(void);
void mapdata_destroy(void);

int mapdata_count(void);

int  mapdata_getcurrentplayer(void);
void mapdata_setcurrentplayer(int player);
int  mapdata_getplayercount(void);
void mapdata_setplayercount(int count);
void mapdata_setplayerowner(int playerindex, int owner);
int  mapdata_getplayerowner(int playerindex);
int  mapdata_getplayerfromowner(int owner);

struct maptile * mapdata_addtile(int x, int y);
struct maptile * mapdata_gettile(int x, int y);
struct maptile * mapdata_getindex(int index);
void             mapdata_removetile(int x, int y);
void             mapdata_clear(void);

void             mapdata_fullclean(void);

struct mapcapital * mapdata_getcapital(int x, int y);

void             mapdata_setmoneyallcapitals(int amount);
int              mapdata_getentitycost(enum mapentity entity);


int              mapdata_getcanmove(struct maptile * tile);

void             mapdata_get6suroundingCoordinates(int x, int y, 
                                                   int * xs_out, int * ys_out);


int  mapdata_endturn(void);

int  mapdata_startturn(int owner);

int  mapdata_moveunit(struct mapcommandresult * result, int owner, 
                      int from_x, int from_y, int to_x, int to_y,
                      enum mapentity entity);


#endif // __MAPDATA_H__

