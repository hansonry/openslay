#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "imagedata.h"

// States include
#include "gamestate.h"


enum state
{
   e_S_game
};

static void CheckForExit(const SDL_Event *event, int * done);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   int done;
   int ticks_previous, ticks_diff, ticks_now;
   float seconds;
   struct gamestatesetting settings;
   enum gamestateplayertype playertypes[5] = {
      e_GSPT_human,
      e_GSPT_ai,
      e_GSPT_ai,
      e_GSPT_ai,
      e_GSPT_ai
   };

   enum state state;
   

   settings.playertypes = playertypes;
   settings.playercount = 2;
   
   SDL_Init(SDL_INIT_EVERYTHING);   
   window = SDL_CreateWindow("Open Slay", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             800, 600, SDL_WINDOW_SHOWN);
   rend = SDL_CreateRenderer(window, -1, 
                             SDL_RENDERER_ACCELERATED | 
                             SDL_RENDERER_PRESENTVSYNC);
   

   imagedata_load(rend);

   
   ticks_previous = SDL_GetTicks();

   // state init
   gamestate_init();
   

   // loop setup
   state = e_S_game;
   gamestate_onenter(&settings);
   done = 0;
   while(done == 0)
   {
      while(SDL_PollEvent(&event))
      {
         CheckForExit(&event, &done);
         switch(state)
         {
         case e_S_game:
            gamestate_event(&event);
            break;
         default:
            fprintf(stderr, "main: Unknown state %d for event\n", state);
            break;
             
         }
      }
      
      ticks_now = SDL_GetTicks();
      ticks_diff = ticks_now - ticks_previous;
      seconds = (float)ticks_diff / 1000.0f;
      ticks_previous = ticks_now;

      switch(state)
      {
      case e_S_game:
         gamestate_update(seconds);
         break;
      default:
         fprintf(stderr, "main: Unknown state %d for update\n", state);
         break;
      }

      
      
      SDL_SetRenderDrawColor(rend, 0x00, 0x33, 0x66, 0xFF);
      SDL_RenderClear( rend );

      switch(state)
      {
      case e_S_game:
         gamestate_render(rend);
         break;
      default:
         fprintf(stderr, "main: Unknown state %d for render\n", state);
         break;
      }

       
      
      SDL_RenderPresent(rend);
   }

   switch(state)
   {
   case e_S_game:
      gamestate_onexit();
      break;
   default:
      fprintf(stderr, "main: Unknown state %d for exit\n", state);
      break;
   }
   
   // state destroy
   gamestate_destroy();
   
   imagedata_free();
   
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

