#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "isometric_engine.h"

static void CheckForExit(const SDL_Event *event, int * done);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   int done;
   int prevTicks, diffTicks, nowTicks;
   float seconds;
   
   SDL_Surface * surface_loading;
   SDL_Texture * texture_cube;
   SDL_Rect      dest_rect;
   
   isoe_t isoe;
   
   SDL_Init(SDL_INIT_EVERYTHING);   
   window = SDL_CreateWindow("Isometric Engine", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             800, 600, SDL_WINDOW_SHOWN);
   
   rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   
   isoe_init(&isoe);
   

   isoe_setup_viewport(&isoe.viewport, 100, 100, 200, 200);
   
   dest_rect.x = isoe.viewport.x;
   dest_rect.y = isoe.viewport.y;
   dest_rect.w = isoe.viewport.width;   
   dest_rect.h = isoe.viewport.height;

   
   //                               tex_width, tex_height, top_width, top_height, height
   isoe_setup_tile(&isoe.tile_info, 64,        71,         64,        32,         39);
   
   //isoe.cam.offset_x = 10;
   //isoe.cam.offset_y = 10;
   
   isoe_cam_center(&isoe, 2, 0, 0);

   
   surface_loading = IMG_Load("cube.bmp");
   texture_cube = SDL_CreateTextureFromSurface(rend, surface_loading);
   SDL_FreeSurface(surface_loading);
   
   
   prevTicks = SDL_GetTicks();
   
   done = 0;
   while(done == 0)
   {
      while(SDL_PollEvent(&event))
      {
         CheckForExit(&event, &done);
      }
      
      nowTicks = SDL_GetTicks();
      diffTicks = nowTicks - prevTicks;
      seconds = (float)diffTicks / 1000.0f;
      prevTicks = nowTicks;
      
      seconds = seconds; // Warning Fix
      
      SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear( rend );

      //isoe_draw(rend, texture_cube, &tile_info, &view, 10, 10, 1, 1, 1);
      //isoe_draw(rend, texture_cube, &tile_info, &view, 10, 10, 1, 0, -1);
      //isoe_draw(rend, texture_cube, &tile_info, &view, 10, 10, 0, 0, 0);
      //isoe_draw(rend, texture_cube, &tile_info, &view, 10, 10, 1, 0, 0);
      //isoe_draw(rend, texture_cube, &tile_info, &view, 10, 10, 1, 0, 1);
      
      //isoe_buffer(&isoe, texture_cube, 1, 1, 1);
      //isoe_buffer(&isoe, texture_cube, 1, 0, -1);
      //isoe_buffer(&isoe, texture_cube, 0, 0, 0);
      //isoe_buffer(&isoe, texture_cube, 1, 0, 0);
      //isoe_buffer(&isoe, texture_cube, 1, 0, 1);
      
      

      isoe_buffer(&isoe, texture_cube, 0, 0, 0);

      isoe_buffer(&isoe, texture_cube, 1, 0, 1);
      isoe_buffer(&isoe, texture_cube, 1, 1, 1);
      isoe_buffer(&isoe, texture_cube, 1, 0, -1);
      
      isoe_buffer(&isoe, texture_cube, 1, 0, 0);
      isoe_buffer(&isoe, texture_cube, 2.0f, 0, 0);
      isoe_buffer(&isoe, texture_cube, 0, -1, 0);      
      
      SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);
      isoe_flush(&isoe, rend);
      
      SDL_RenderDrawRect(rend, &dest_rect);
      
      SDL_RenderPresent(rend);
   }
   
   
   isoe_destroy(&isoe);
   SDL_DestroyTexture(texture_cube);
   
   SDL_DestroyRenderer(rend);
   SDL_DestroyWindow(window);
   SDL_Quit();
   
   
   printf("End\n");
   return 0;
}

static void CheckForExit(const SDL_Event *event, int * done)
{
   if(event->type == SDL_QUIT)
   {
      (*done) = 1;
   }
   else if(event->type == SDL_KEYDOWN)
   {
      if(event->key.keysym.sym == SDLK_ESCAPE)
      {
         (*done) = 1;
      }
   }

}
