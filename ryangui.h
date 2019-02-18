#ifndef __RYANGUI_H__
#define __RYANGUI_H__
#include "SDL2/SDL.h"

struct ryangui_layoutbounds
{
   int width_min;
   int width_max;
   int height_min;
   int height_max;
};

struct ryangui_component;

typedef void (*ryangui_compfunccleanup)(struct ryangui_component * comp, 
                                        void * data);
typedef void (*ryangui_compfunclayout)(struct ryangui_component * comp, 
                                       struct ryangui_layoutbounds * bounds,
                                       void * data);
typedef void (*ryangui_compfuncrender)(struct ryangui_component, 
                                       SDL_Renderer * rend, 
                                       void * data,
                                       int x, int y,
                                       int width, int height);
struct ryangui_componentdefinition
{
   ryangui_compfunccleanup cleanup;
   ryangui_compfunclayout  layout;
   ryangui_compfuncrender  render;
   void * data;
};


typedef void (*ryangui_compfuncinit)(struct ryangui_componentdefinition * definition);



struct ryangui;

struct ryangui * ryangui_new(const char * rootcompname, 
                             ryangui_compfuncinit rootcompinit);
void ryangui_destroy(struct ryangui * gui);

void ryangui_layout(struct ryangui * gui);
void ryangui_layoutforce(struct ryangui * gui);
void ryangui_render(struct ryangui * gui, SDL_Renderer * rend);

struct ryangui_component * 
ryangui_getrootcomponent(struct ryangui * gui);

struct ryangui_component * 
ryangui_component_createchild(struct ryangui_component * parent, 
                              const char * compname, 
                              ryangui_compfuncinit compinit);

int
ryangui_component_childcount(struct ryangui_component * parent);

struct ryangui_component * 
ryangui_component_getchildbyindex(struct ryangui_component * parent, 
                                  int index);

struct ryangui_component *
ryangui_component_getchildbyname(struct ryangui_component * parent,
                                 const char * childname);

const char * ryangui_component_getname(struct ryangui_component * comp);

void ryangui_component_layout(struct ryangui_component * comp, int force_flag);

#endif // __RYANGUI_H__

