#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL2/SDL_image.h"
#include "ryangui.h"


#define GROWBY 32

struct ryangui_lookfeel
{
   SDL_Texture * bmpfont_texture;
   int bmpfont_char_width;
   int bmpfont_char_height;
   int bmpfont_char_spacing;
   int bmpfont_src_text_width;
   int padding;
   SDL_Color background;
   SDL_Color outline;
};

struct ryangui_component
{
   struct ryangui_lookfeel * lookfeel;
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
   int flags;
};

struct ryangui
{
   struct ryangui_component * root;
   struct ryangui_lookfeel lookfeel;
};

static SDL_Texture * ryangui_loadimage(SDL_Renderer * rend,
                                       const char * imagefilename)
{
   SDL_Texture * texture;
   SDL_Surface * surface;
   surface = IMG_Load(imagefilename);
   if(surface == NULL)
   {
      fprintf(stderr, 
              "imagedata_loadimage: failed to turn %s into an image\n", 
              imagefilename);
      return NULL;
   }
   texture = SDL_CreateTextureFromSurface(rend, surface);
   SDL_FreeSurface(surface);
   return texture;
}

static struct ryangui_component * 
ryangui_component_new(const char * name,
                      ryangui_compfuncinit init,
                      struct ryangui_lookfeel * lookfeel)
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

   
   // Look and Feel
   comp->lookfeel = lookfeel;

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

   // Flags
   //comp->flags = RYANGUI_FLAGS_DRAWBORDER | RYANGUI_FLAGS_DRAWBACKGROUND;
   comp->flags = 0;

   init(&comp->definition);

   return comp;
}

struct ryangui * ryangui_new(SDL_Renderer * rend, 
                             const char * rootcompname, 
                             ryangui_compfuncinit rootcompinit)
{
   struct ryangui * gui;
   struct ryangui_lookfeel * lookfeel;
   gui = malloc(sizeof(struct ryangui));

   // Look and Feel
   lookfeel = &gui->lookfeel;
   lookfeel->bmpfont_texture = ryangui_loadimage(rend, 
                                                 "assets/images/bitmapfont.png");
   lookfeel->bmpfont_char_width     = 10;
   lookfeel->bmpfont_char_height    = 24;
   lookfeel->bmpfont_char_spacing   = 4;
   lookfeel->bmpfont_src_text_width = 200;

   lookfeel->background.r = 0x15;
   lookfeel->background.g = 0x16;
   lookfeel->background.b = 0x17;
   lookfeel->background.a = SDL_ALPHA_OPAQUE;
   
   lookfeel->outline.r = 0x4C; 
   lookfeel->outline.g = 0x5C;
   lookfeel->outline.b = 0x7D;
   lookfeel->outline.a = SDL_ALPHA_OPAQUE;

   lookfeel->padding = 5;

   gui->root = ryangui_component_new(rootcompname, 
                                     rootcompinit, 
                                     &gui->lookfeel);
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
   SDL_DestroyTexture(gui->lookfeel.bmpfont_texture);
   free(gui);
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
         if((comp->flags & RYANGUI_FLAGS_DRAWBORDER) == RYANGUI_FLAGS_DRAWBORDER)
         {
            comp->width += comp->lookfeel->padding * 2;
            comp->height += comp->lookfeel->padding * 2;

         }
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
   SDL_Rect clip, area;
   clip.x = x_offset + comp->x;
   clip.y = y_offset + comp->y;
   clip.w = comp->width;
   clip.h = comp->height;

   if((comp->flags & RYANGUI_FLAGS_DRAWBACKGROUND) == RYANGUI_FLAGS_DRAWBACKGROUND)
   {
      ryangui_component_draw_background(comp, rend, clip.x, clip.y, 
                                                    clip.w, clip.h);
   }

   if((comp->flags & RYANGUI_FLAGS_DRAWBORDER) == RYANGUI_FLAGS_DRAWBORDER)
   {
      ryangui_component_draw_border(comp, rend, clip.x, clip.y, 
                                                clip.w, clip.h);

      area.x = clip.x + comp->lookfeel->padding;
      area.y = clip.y + comp->lookfeel->padding;
      area.w = clip.w - comp->lookfeel->padding;
      area.h = clip.h - comp->lookfeel->padding;
   }
   else
   {
      area.x = clip.x;
      area.y = clip.y;
      area.w = clip.w;
      area.h = clip.h;
   }

   if(comp->definition.render != NULL)
   {
      SDL_RenderSetClipRect(rend, &clip); 
      comp->definition.render(comp, 
                              comp->definition.data, 
                              rend,
                              area.x, 
                              area.y,
                              area.w,
                              area.h);
   }

   for(i = 0; i < comp->childlist_count; i++)
   {
      struct ryangui_component * child;
      child = comp->childlist_base[i];

      ryangui_component_render(child, 
                               rend, 
                               area.x, 
                               area.y);
      

   }
}

void ryangui_render(struct ryangui * gui, SDL_Renderer * rend)
{
   ryangui_component_render(gui->root, rend, 0, 0);
   SDL_RenderSetClipRect(rend, NULL); // Clear the clip
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

   child = ryangui_component_new(compname, compinit, parent->lookfeel);

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
   comp->needs_layout = 1;
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
   comp->needs_layout = 1;
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
   comp->needs_layout = 1;
}

void ryangui_component_set_height(struct ryangui_component * comp, int height)
{
   if(height > 0) comp->height = height;
   comp->needs_layout = 1;
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

void * ryangui_component_get_data(struct ryangui_component * comp)
{
   return comp->definition.data;
}

void ryangui_component_set_flags(struct ryangui_component * comp, int flags)
{
   comp->flags = flags;
   comp->needs_layout = 1;
}

// Custom Component Helpers
void ryangui_component_notify_layout(struct ryangui_component * comp)
{
   comp->needs_layout = 1;
}

void ryangui_component_draw_background(struct ryangui_component * comp,
                                       SDL_Renderer * rend,
                                       int x, int y,
                                       int width, int height)
{
   SDL_Rect r;
   r.x = x;
   r.y = y;
   r.w = width;
   r.h = height;
   SDL_SetRenderDrawColor(rend,
                          comp->lookfeel->background.r,
                          comp->lookfeel->background.g,
                          comp->lookfeel->background.b,
                          comp->lookfeel->background.a);
   SDL_RenderFillRect(rend, &r);
}

void ryangui_component_draw_border(struct ryangui_component * comp,
                                   SDL_Renderer * rend,
                                   int x, int y,
                                   int width, int height)
{
   SDL_Rect r;
   r.x = x;
   r.y = y;
   r.w = width;
   r.h = height;
   SDL_SetRenderDrawColor(rend,
                          comp->lookfeel->outline.r,
                          comp->lookfeel->outline.g,
                          comp->lookfeel->outline.b,
                          comp->lookfeel->outline.a);
   SDL_RenderDrawRect(rend, &r);

}

// Components

// Box

static void ryangui_component_box_layout(struct ryangui_component * comp, 
                                         struct ryangui_layoutbounds * bounds,
                                         void * data)
{
   int i, size;
   int mw, mh;
   struct ryangui_component * child;
   mw = 0;
   mh = 0;
   size = ryangui_component_childcount(comp);
   for(i = 0; i < size; i++)
   {
      int w, h, x, y;
      child = ryangui_component_getchildbyindex(comp, i);
      ryangui_component_layout(child, 0);
      ryangui_component_get_possize(child, &x, &y, &w, &h);
      if(mw < x + w) mw = x + w;
      if(mh < y + h) mh = y + h;

   }

   ryangui_component_set_size(comp, mw, mh);
}


static void ryangui_component_box_render(struct ryangui_component * comp, 
                                         void * data,
                                         SDL_Renderer * rend, 
                                         int x, int y,
                                         int width, int height)
{
   //ryangui_component_draw_background(comp, rend, x, y, width, height);
   //ryangui_component_draw_border(comp, rend, x, y, width, height);
}

static int ryangui_component_box_event(struct ryangui_component * comp,
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

struct ryangui_component_label_data
{
   char * text;
};

static void ryangui_component_label_cleanup(struct ryangui_component * comp, 
                                            void * rdata)
{
   struct ryangui_component_label_data * data;
   data = (struct ryangui_component_label_data*)rdata;
   free(data->text);
   free(data);
}

static void ryangui_text_get_size(struct ryangui_lookfeel * lookfeel, 
                                  const char * text, 
                                  int * width, int * height)
{
   int lwidth, lheight;
   int startline, linewidth;

   lwidth = 0;
   lheight = lookfeel->bmpfont_char_height;

   startline = 1;
   linewidth = 0;

   while(*text != '\0')
   {
      if(startline == 1)
      {
         startline = 0;
      }
      else
      {
         linewidth += lookfeel->bmpfont_char_spacing;
      }
      if(*text == '\n')
      {
         lheight += lookfeel->bmpfont_char_height;
         if(linewidth > lwidth)
         {
            lwidth = linewidth;
         }
         startline = 1;
         linewidth = 0;
      }
      else
      {
         linewidth += lookfeel->bmpfont_char_width;
      }
      text ++;
   }
   if(linewidth > lwidth)
   {
      lwidth = linewidth;
   }
   

   if(width != NULL)
   {
      *width = lwidth;
   }
   if(height != NULL)
   {
      *height = lheight;
   }
}

static void ryangui_component_label_layout(struct ryangui_component * comp, 
                                           struct ryangui_layoutbounds * bounds,
                                           void * rdata)
{
   struct ryangui_component_label_data * data;
   int text_width, text_height;

   data = (struct ryangui_component_label_data*)rdata;
   ryangui_text_get_size(comp->lookfeel, data->text, 
                         &text_width, &text_height);

   ryangui_component_set_size(comp, text_width, text_height);
}


static void ryangui_component_label_render(struct ryangui_component * comp, 
                                           void * rdata,
                                           SDL_Renderer * rend, 
                                           int x, int y,
                                           int width, int height)
{
   SDL_Rect dest_rect, src_rect;
   struct ryangui_component_label_data * data;
   const char * text;
   char c;

   //ryangui_component_draw_background(comp, rend, x, y, width, height);
   //ryangui_component_draw_border(comp, rend, x, y, width, height);

   data = (struct ryangui_component_label_data*)rdata;
   

   src_rect.w = comp->lookfeel->bmpfont_char_width;
   src_rect.h = comp->lookfeel->bmpfont_char_height;
   dest_rect.w = comp->lookfeel->bmpfont_char_width;
   dest_rect.h = comp->lookfeel->bmpfont_char_height;

   dest_rect.x = x;
   dest_rect.y = y;

   text = data->text;
   while(*text != '\0')
   {
      // Figure out what charater to write
      
      if( *text == '\r')
      {
         c = '\0';
      }
      else if( (*text == '\n') || 
               ( (*text >= ' ') && (*text <= '~') ) )
      {
         c = *text;
      }      
      else
      {
         c = '?';
      }

      if(c != '\0')
      {
         if(c == '\n')
         {
            dest_rect.x = x;
            dest_rect.y += comp->lookfeel->bmpfont_char_height;
         }
         else
         {
            int offset;
            int pixel_offset;
            // Figure out where in the bitmap to pull from

            offset = c - ' ';
            pixel_offset = offset * comp->lookfeel->bmpfont_char_width;

            src_rect.x = pixel_offset % 
                         comp->lookfeel->bmpfont_src_text_width;
            src_rect.y = (pixel_offset / 
                          comp->lookfeel->bmpfont_src_text_width) * 
                          comp->lookfeel->bmpfont_char_height; 


            SDL_RenderCopy(rend, comp->lookfeel->bmpfont_texture, 
                           &src_rect, &dest_rect);
            dest_rect.x += comp->lookfeel->bmpfont_char_spacing + 
                           comp->lookfeel->bmpfont_char_width;
         }
      }

      text ++;
   }

}

int ryangui_component_label_event(struct ryangui_component * comp,
                                  void * data,
                                  SDL_Event * event,
                                  int x, int y,
                                  int width, int height)
{
   return 0;
}

void ryangui_component_label_init(struct ryangui_componentdefinition * definition)
{
   struct ryangui_component_label_data * data;
   data = malloc(sizeof(struct ryangui_component_label_data));

   definition->data    = data;
   definition->cleanup = ryangui_component_label_cleanup;
   definition->layout  = ryangui_component_label_layout;
   definition->render  = ryangui_component_label_render;
   definition->event   = ryangui_component_label_event;

   data->text = strdup("");


}

void ryangui_component_label_set_text(struct ryangui_component * label, const char * text)
{
   struct ryangui_component_label_data * data;
   data = ryangui_component_get_data(label);
   free(data->text);
   data->text = strdup(text);
   ryangui_component_notify_layout(label);
}

