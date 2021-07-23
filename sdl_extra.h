#ifndef SDL_EXTRA_H
#define SDL_EXTRA_H
#include <SDL2/SDL.h>
#include "la.h"

Vec2f window_size(SDL_Window* win);

void scc(int code);

void* scp(void* ptr);

#endif /* SDL_EXTRA_H */
