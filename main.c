#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

static void CheckForExit(const SDL_Event *event, int * done);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   int done;
   int ticks_previous, ticks_diff, ticks_now;
   float seconds;

   
   SDL_Surface * surface_loading;
   SDL_Texture * texture_cube;
   
   
   SDL_Init(SDL_INIT_EVERYTHING);   
   window = SDL_CreateWindow("SDL 2D", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             800, 600, SDL_WINDOW_SHOWN);
   rend = SDL_CreateRenderer(window, -1, 
                             SDL_RENDERER_ACCELERATED | 
                             SDL_RENDERER_PRESENTVSYNC);
   

   
   surface_loading = IMG_Load("cube.bmp");
   texture_cube = SDL_CreateTextureFromSurface(rend, surface_loading);
   SDL_FreeSurface(surface_loading);
   
   
   ticks_previous = SDL_GetTicks();
   
   done = 0;
   while(done == 0)
   {
      while(SDL_PollEvent(&event))
      {
         CheckForExit(&event, &done);
      }
      
      ticks_now = SDL_GetTicks();
      ticks_diff = ticks_now - ticks_previous;
      seconds = (float)ticks_diff / 1000.0f;
      ticks_previous = ticks_now;

      seconds = seconds;
      
      
      SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear( rend );

      
      
      SDL_RenderPresent(rend);
   }
   
   
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

