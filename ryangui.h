#ifndef __RYANGUI_H__
#define __RYANGUI_H__
#include "SDL2/SDL.h"

#define RYANGUI_FLAGS_DRAWBACKGROUND 0x01
#define RYANGUI_FLAGS_DRAWBORDER     0x02

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
typedef void (*ryangui_compfuncrender)(struct ryangui_component * comp, 
                                       void * data,
                                       SDL_Renderer * rend, 
                                       int x, int y,
                                       int width, int height);
// Returns 1 if the event was consumed
typedef int (*ryangui_compfuncevent)(struct ryangui_component * comp,
                                     void * data,
                                     SDL_Event * event,
                                     int x, int y,
                                     int width, int height);
struct ryangui_componentdefinition
{
   ryangui_compfunccleanup cleanup;
   ryangui_compfunclayout  layout;
   ryangui_compfuncrender  render;
   ryangui_compfuncevent   event;
   void * data;
};


typedef void (*ryangui_compfuncinit)(struct ryangui_componentdefinition * definition);


struct ryangui;

struct ryangui * ryangui_new(SDL_Renderer * rend,
                             const char * rootcompname, 
                             ryangui_compfuncinit rootcompinit);
void ryangui_destroy(struct ryangui * gui);

void ryangui_layout(struct ryangui * gui);
void ryangui_layoutforce(struct ryangui * gui);
void ryangui_render(struct ryangui * gui, SDL_Renderer * rend);
int  ryangui_event(struct ryangui * gui, SDL_Event * event);


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
                                 const char * childname, int * foundindex);

const char * ryangui_component_getname(struct ryangui_component * comp);

void ryangui_component_layout(struct ryangui_component * comp, int force_flag);

void ryangui_component_set_possize(struct ryangui_component * comp, 
                                   int x, int y, 
                                   int width, int height);
void ryangui_component_set_position(struct ryangui_component * comp,
                                    int x, int y);
void ryangui_component_set_size(struct ryangui_component * comp,
                                int width, int height);
void ryangui_component_set_x(struct ryangui_component * comp, int x);
void ryangui_component_set_y(struct ryangui_component * comp, int y);
void ryangui_component_set_width(struct ryangui_component * comp, int width);
void ryangui_component_set_height(struct ryangui_component * comp, int height);

void ryangui_component_get_possize(struct ryangui_component * comp, 
                                   int * x, int * y, 
                                   int * width, int * height);

void * ryangui_component_get_data(struct ryangui_component * comp);

void ryangui_component_set_flags(struct ryangui_component * comp, int flags);


// Custom Component Helpers
void ryangui_component_notify_layout(struct ryangui_component * comp);
void ryangui_component_draw_background(struct ryangui_component * comp,
                                       SDL_Renderer * rend,
                                       int x, int y,
                                       int width, int height);
void ryangui_component_draw_border(struct ryangui_component * comp,
                                   SDL_Renderer * rend,
                                   int x, int y,
                                   int width, int height);



// Components

void ryangui_component_box_init(struct ryangui_componentdefinition * definition);

void ryangui_component_label_init(struct ryangui_componentdefinition * definition);

void ryangui_component_label_set_text(struct ryangui_component * label, const char * text);

#endif // __RYANGUI_H__

