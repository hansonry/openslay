#ifndef __IMAGEDATA_H__
#define __IMAGEDATA_H__
#include "SDL2/SDL.h"

struct imagedata
{
   SDL_Texture * hex;
   SDL_Texture * unit;
   SDL_Texture * capital;
   SDL_Texture * castle;
   SDL_Texture * grave;
   SDL_Texture * trees;
   SDL_Texture * hex_outline;
   SDL_Texture * bitmapfont;
};

void imagedata_load(SDL_Renderer * rend);
void imagedata_free(void);

struct imagedata * imagedata_get(void);


#endif // __IMAGEDATA_H__

