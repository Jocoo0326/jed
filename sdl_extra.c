#include "sdl_extra.h"

Vec2f window_size(SDL_Window* win)
{
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  return vec2f(w, h);
}

void scc(int code)
{
  if (code < 0) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
}

void* scp(void* ptr)
{
  if (ptr == NULL) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
  return ptr;
}
