#ifndef TETRIS
#define TETRIS
#include <SDL.h>
#include <stdio.h>
#include <time.h>
// graphics.c
SDL_Texture *load_texture(char *file, SDL_Renderer *renderer, FILE *fp);
void graphics_quit(SDL_Renderer *r, SDL_Window *w);
void render_whole_texture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y);
void graphics_init(int width, int height, SDL_Renderer **r, SDL_Window **w, FILE *fp);
void fill_screen_with_tiles(SDL_Texture *t, SDL_Renderer *r);
void draw_rect(SDL_Renderer *r, int x, int y, int w, int h);
// input.c
struct {
  Uint8 up, right, down, left;
  Uint8 esc, space, enter, lshift, rshift;
} key, prev_key;
int input_update();
// other
void logSDLError(FILE *fp, char *msg);
#endif
