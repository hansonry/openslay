#include "imagedata.h"
#include "SDL2/SDL_image.h"
#include <stdio.h>


static struct imagedata imagedata;
static int imagedataloaded = 0;

static SDL_Texture * imagedata_loadimage(SDL_Renderer * rend,
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

void imagedata_load(SDL_Renderer * rend)
{
   imagedata.hex         = imagedata_loadimage(rend, "assets/images/32x32Hex.png");
   imagedata.unit        = imagedata_loadimage(rend, "assets/images/Unit.png");
   imagedata.capital     = imagedata_loadimage(rend, "assets/images/Capital.png");
   imagedata.castle      = imagedata_loadimage(rend, "assets/images/Castle.png");
   imagedata.trees       = imagedata_loadimage(rend, "assets/images/trees.png");
   imagedata.grave       = imagedata_loadimage(rend, "assets/images/grave.png");
   imagedata.hex_outline = imagedata_loadimage(rend, "assets/images/32x32HexOutline.png");
   imagedata.bitmapfont  = imagedata_loadimage(rend, "assets/images/bitmapfont.png");


   imagedataloaded = 1;
}

void imagedata_free(void)
{
   imagedataloaded = 0;

   SDL_DestroyTexture(imagedata.hex);
   SDL_DestroyTexture(imagedata.unit);
   SDL_DestroyTexture(imagedata.capital);
   SDL_DestroyTexture(imagedata.castle);
   SDL_DestroyTexture(imagedata.trees);
   SDL_DestroyTexture(imagedata.grave);
   SDL_DestroyTexture(imagedata.hex_outline);
   SDL_DestroyTexture(imagedata.bitmapfont);
}

struct imagedata * imagedata_get(void)
{
   if(imagedataloaded == 0)
   {
      return NULL;
   }
   return &imagedata;
}

