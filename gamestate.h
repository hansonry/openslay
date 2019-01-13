#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__
#include "SDL2/SDL.h"


void gamestate_init(void);
void gamestate_destroy(void);

void gamestate_onenter(void);
void gamestate_onexit(void);

void gamestate_update(float dt);

void gamestate_render(SDL_Renderer * rend);

void gamestate_event(SDL_Event * event);


#endif // __GAMESTATE_H__

