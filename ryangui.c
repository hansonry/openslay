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
   struct ryangui_component ** childlist_base;
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

   if(comp->definition.cleanup != NULL)
   {
      comp->definition.cleanup(comp, comp->definition.data); 
   }

   for(i = 0; i < comp->childlist_count; i++)
   {
      childcomp = comp->childlist_base[i];
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
      if(comp->definition.layout != NULL)
      {
         comp->definition.layout(comp, &comp->bounds, comp->definition.data);
      }

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
         childcomp = comp->childlist_base[i];
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

static void ryangui_component_render(struct ryangui_component * comp, 
                                     SDL_Renderer * rend, 
                                     int x_offset, int y_offset)
{
   size_t i;

   if(comp->definition.render != NULL)
   {
      comp->definition.render(comp, 
                              comp->definition.data, 
                              rend,
                              x_offset + comp->x, 
                              y_offset + comp->y,
                              comp->width,
                              comp->height);
   }

   for(i = 0; i < comp->childlist_count; i++)
   {
      struct ryangui_component * child;
      child = comp->childlist_base[i];

      ryangui_component_render(child, 
                               rend, 
                               x_offset + comp->x, 
                               y_offset + comp->y);
      

   }
}

void ryangui_render(struct ryangui * gui, SDL_Renderer * rend)
{
   ryangui_component_render(gui->root, rend, 0, 0);
}

static int ryangui_component_event(struct ryangui_component * comp,
                                   SDL_Event * event,
                                   int x_offset, int y_offset)
{
   size_t i;
   int event_used;
   event_used = 0;
   for(i = 0; i < comp->childlist_count; i++)
   {
      struct ryangui_component * child;
      child = comp->childlist_base[i];

      event_used = ryangui_component_event(child, 
                                           event,
                                           x_offset + comp->x, 
                                           y_offset + comp->y);
      
      if(event_used == 1)
      {
         break;
      }
   }

   if(event_used == 0 && comp->definition.render != NULL)
   {
      event_used = comp->definition.event(comp, 
                                          comp->definition.data, 
                                          event,
                                          x_offset + comp->x, 
                                          y_offset + comp->y,
                                          comp->width,
                                          comp->height);
   }

   return event_used;

}

int ryangui_event(struct ryangui * gui, SDL_Event * event)
{
   return ryangui_component_event(gui->root, event, 0, 0);
}

struct ryangui_component * 
ryangui_getrootcomponent(struct ryangui * gui)
{
   return gui->root;
}

struct ryangui_component * 
ryangui_component_createchild(struct ryangui_component * parent, 
                              const char * compname, 
                              ryangui_compfuncinit compinit)
{
   struct ryangui_component * child;
   // Check size and grow array if nessary
   if(parent->childlist_count >= parent->childlist_size)
   {
      parent->childlist_size = parent->childlist_count + GROWBY;
      parent->childlist_base = realloc(parent->childlist_base, 
                                       sizeof(struct ryangui_component *) * 
                                       parent->childlist_size);
   }

   child = ryangui_component_new(compname, compinit);

   parent->childlist_base[parent->childlist_count] = child;
   parent->childlist_count ++;
   return child;
}

int
ryangui_component_childcount(struct ryangui_component * parent)
{
   return parent->childlist_count;
}

struct ryangui_component * 
ryangui_component_getchildbyindex(struct ryangui_component * parent, 
                                  int index)
{
   if(index >= 0 && index <= parent->childlist_count)
   {
      return parent->childlist_base[index];
   }
   return NULL;
}

struct ryangui_component *
ryangui_component_getchildbyname(struct ryangui_component * parent,
                                 const char * childname, int * foundindex)
{
   size_t i;
   for(i = 0; i < parent->childlist_count; i++)
   {
      struct ryangui_component * child;
      child = parent->childlist_base[i];
      if(strcmp(child->name, childname) == 0)
      {
         if(foundindex != NULL)
         {
            *foundindex = i;
         }
         return child;
      }
   }
   return NULL;
}

const char * ryangui_component_getname(struct ryangui_component * comp)
{
   return comp->name;
}

void ryangui_component_set_possize(struct ryangui_component * comp, 
                                   int x, int y, 
                                   int width, int height)
{
   comp->x = x;
   comp->y = y;

   if(width > 0) comp->width = width;
   if(height > 0) comp->height = height;
}

void ryangui_component_set_position(struct ryangui_component * comp,
                                    int x, int y)
{
   comp->x = x;
   comp->y = y;
}
void ryangui_component_set_size(struct ryangui_component * comp,
                                int width, int height)
{
   if(width > 0) comp->width = width;
   if(height > 0) comp->height = height;
}

void ryangui_component_set_x(struct ryangui_component * comp, int x)
{
   comp->x = x;
}

void ryangui_component_set_y(struct ryangui_component * comp, int y)
{
   comp->y = y;
}

void ryangui_component_set_width(struct ryangui_component * comp, int width)
{
   if(width > 0) comp->width = width;
}

void ryangui_component_set_height(struct ryangui_component * comp, int height)
{
   if(height > 0) comp->height = height;
}

void ryangui_component_get_possize(struct ryangui_component * comp, 
                                   int * x, int * y, 
                                   int * width, int * height)
{
   if(x != NULL)      *x = comp->x;
   if(y != NULL)      *y = comp->y;
   if(width != NULL)  *width = comp->width;
   if(height != NULL) *height = comp->height;
}
// Components

// Box

void ryangui_component_box_layout(struct ryangui_component * comp, 
                                  struct ryangui_layoutbounds * bounds,
                                  void * data)
{
}


void ryangui_component_box_render(struct ryangui_component * comp, 
                                  void * data,
                                  SDL_Renderer * rend, 
                                  int x, int y,
                                  int width, int height)
{
   SDL_Rect r;
   r.x = x;
   r.y = y;
   r.w = width;
   r.h = height;
   SDL_SetRenderDrawColor(rend, 255, 255, 255, SDL_ALPHA_OPAQUE);
   SDL_RenderDrawRect(rend, &r);
}

int ryangui_component_box_event(struct ryangui_component * comp,
                                void * data,
                                SDL_Event * event,
                                int x, int y,
                                int width, int height)
{
   return 0;
}

void ryangui_component_box_init(struct ryangui_componentdefinition * definition)
{
   definition->data    = NULL;
   definition->cleanup = NULL;
   definition->layout  = ryangui_component_box_layout;
   definition->render  = ryangui_component_box_render;
   definition->event   = ryangui_component_box_event;
}

