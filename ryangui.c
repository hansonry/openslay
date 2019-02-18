#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ryangui.h"


#define GROWBY 32
struct ryangui_component
{
   struct ryangui_componentdefinition definition;
   struct ryangui_layoutbounds bounds;
   size_t childlist_count;
   size_t childlist_size;
   struct ryangui_component * childlist_base;
   char * name;
   int needs_layout;
   int x;
   int y;
   int width;
   int height;
};

struct ryangui
{
   struct ryangui_component * root;
};

static struct ryangui_component * ryangui_component_new(const char * name,
                                                        ryangui_compfuncinit init)
{
   struct ryangui_component * comp;
   comp = malloc(sizeof(struct ryangui_component));
   
   // Set Name
   comp->name = strdup(name);
   
   // Init Child List
   comp->childlist_count = 0;
   comp->childlist_size = GROWBY;
   comp->childlist_base = malloc(sizeof(struct ryangui_component *) *  
                                 comp->childlist_size);

   
   // Layout Info
   comp->needs_layout = 1;
   comp->bounds.width_min = 0;
   comp->bounds.width_max = -1;
   comp->bounds.height_min = 0;
   comp->bounds.height_max = -1;

   // Run Init function
   comp->definition.cleanup = NULL;
   comp->definition.layout = NULL;
   comp->definition.render = NULL;
   comp->definition.data = NULL;
   init(&comp->definition);

   return comp;
}

struct ryangui * ryangui_new(const char * rootcompname, 
                             ryangui_compfuncinit rootcompinit)
{
   struct ryangui * gui;
   gui = malloc(sizeof(struct ryangui));
   gui->root = ryangui_component_new(rootcompname, rootcompinit);
   return gui;
}

static void  ryangui_component_destroy(struct ryangui_component * comp)
{
   size_t i;
   struct ryangui_component * childcomp;

   comp->definition.cleanup(comp, comp->definition.data); 

   for(i = 0; i < comp->childlist_count; i++)
   {
      childcomp = &comp->childlist_base[i];
      ryangui_component_destroy(childcomp);
   }

   free(comp->name);
   free(comp->childlist_base);
   free(comp);
}

void ryangui_destroy(struct ryangui * gui)
{
   ryangui_component_destroy(gui->root);
}

void ryangui_component_layout(struct ryangui_component * comp, int force_flag)
{
   size_t i;
   struct ryangui_component * childcomp;

   if(force_flag == 1 ||
      comp->needs_layout == 1)
   {
      comp->definition.layout(comp, &comp->bounds, comp->definition.data);

      // limit this component based on bounds
      if(comp->bounds.width_min >= 0 && 
         comp->width < comp->bounds.width_min)
      {
         comp->width = comp->bounds.width_min;
      }
      else if(comp->bounds.width_max >= 0 &&
              comp->width > comp->bounds.width_max)
      {
         comp->width = comp->bounds.width_max;
      }

      if(comp->bounds.height_min >= 0 &&
         comp->height < comp->bounds.height_min)
      {
         comp->height = comp->bounds.height_min;
      }
      else if(comp->bounds.height_max >= 0 &&
              comp->height > comp->bounds.height_max)
      {
         comp->height = comp->bounds.height_max;
      }

      // This compoent may or may not have triggered a layout, so layout again
      // just to be sure
      for(i = 0; i < comp->childlist_count; i++)
      {
         childcomp = &comp->childlist_base[i];
         ryangui_component_layout(childcomp, force_flag);
      }

      comp->needs_layout = 0;
   }
}

void ryangui_layout(struct ryangui * gui)
{
   ryangui_component_layout(gui->root, 0);
}

void ryangui_layoutforce(struct ryangui * gui)
{
   ryangui_component_layout(gui->root, 1);
}

void ryangui_render(struct ryangui * gui, SDL_Renderer * rend);

struct ryangui_component * 
ryangui_getrootcomponent(struct ryangui * gui)
{
   return gui->root;
}

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

