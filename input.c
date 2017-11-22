#include "tetris.h"

int input_update(){
  SDL_Event e;
  while(SDL_PollEvent(&e)){
    if(e.type == SDL_QUIT) return 1;
  }
  prev_key.up = key.up;
  prev_key.right = key.right;
  prev_key.down = key.down;
  prev_key.left = key.left;
  prev_key.esc = key.esc;
  prev_key.enter = key.enter;
  prev_key.space = key.space;
  prev_key.lshift = key.lshift;
  prev_key.rshift = key.rshift;
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  key.up = state[SDL_SCANCODE_UP];
  key.right = state[SDL_SCANCODE_RIGHT];
  key.down = state[SDL_SCANCODE_DOWN];
  key.left = state[SDL_SCANCODE_LEFT];
  key.esc = state[SDL_SCANCODE_ESCAPE];
  key.space = state[SDL_SCANCODE_SPACE];
  key.enter = state[SDL_SCANCODE_RETURN];
  key.lshift = state[SDL_SCANCODE_LSHIFT];
  key.rshift = state[SDL_SCANCODE_RSHIFT];
  return 0;
}
