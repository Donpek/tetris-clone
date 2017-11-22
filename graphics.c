#include "tetris.h"
SDL_Texture *load_texture(char *file, SDL_Renderer *renderer, FILE *fp){
  SDL_Texture *texture = NULL;
  SDL_Surface *loaded_image = SDL_LoadBMP(file);
  if(loaded_image == NULL){
    logSDLError(fp,"SDL_LoadBMP");
  }else{
    texture = SDL_CreateTextureFromSurface(renderer,loaded_image);
    SDL_FreeSurface(loaded_image);
    if(texture == NULL){
      logSDLError(fp,"SDL_CreateTextureFromSurface");
    }
  }
  return texture;
}
void graphics_quit(SDL_Renderer *r, SDL_Window *w){
  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(w);
  SDL_Quit();
}
void render_whole_texture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
  SDL_Rect dst;
  dst.x = x; dst.y = y;
  SDL_QueryTexture(tex,NULL,NULL,&dst.w,&dst.h);
  SDL_RenderCopy(ren,tex,NULL,&dst);
}
void graphics_init(int width, int height, SDL_Renderer **r, SDL_Window **w, FILE *fp){
  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    logSDLError(fp,"SDL_Init");
  }
  *w = SDL_CreateWindow("Tetris",200,100,width,height,SDL_WINDOW_SHOWN);
  if(w == NULL){
    logSDLError(fp,"SDL_CreateWindow");
  }
  *r = SDL_CreateRenderer(*w,-1,SDL_RENDERER_PRESENTVSYNC);
  if(r == NULL){
    logSDLError(fp,"SDL_CreateRenderer");
  }
}
void draw_rect(SDL_Renderer *r, int x, int y, int w, int h){
  SDL_Rect rect = {x,y,w,h};
  SDL_RenderFillRect(r,&rect);
}
