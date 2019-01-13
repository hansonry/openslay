#include "gamestate.h"
#include "imagedata.h"


static struct imagedata * imagedata;
static SDL_Rect rect;

void gamestate_init(void)
{
   rect.w = 32; rect.h = 32;
   rect.x = (800 - rect.w) / 2;
   rect.y = (600 - rect.h) / 2;
   imagedata = imagedata_get();
}

void gamestate_destroy(void)
{
}


void gamestate_onenter(void)
{
}

void gamestate_onexit(void)
{
}


void gamestate_update(float dt)
{
}

void gamestate_render(SDL_Renderer * rend)
{
   SDL_RenderCopy(rend, imagedata->hex, NULL, &rect);
}

void gamestate_event(SDL_Event * event)
{
}

