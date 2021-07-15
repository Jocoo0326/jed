#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "buffer.h"
#include "la.h"
#include "stb_image.h"

void scc(int code) {
  if (code < 0) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
}

void *scp(void *ptr) {
  if (ptr == NULL) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
  return ptr;
}

SDL_Surface *surface_from_file(const char *file_path) {
  int width, height, n;
  unsigned char *pixels =
      stbi_load(file_path, &width, &height, &n, STBI_rgb_alpha);
  if (pixels == NULL) {
    fprintf(stderr, "ERROR: could not load image %s: %s\n", file_path,
            stbi_failure_reason());
    exit(1);
  }
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  const Uint32 rmask = 0xff000000;
  const Uint32 gmask = 0x00ff0000;
  const Uint32 bmask = 0x0000ff00;
  const Uint32 amask = 0x000000ff;
#else  // little endian, like x86
  const Uint32 rmask = 0x000000ff;
  const Uint32 gmask = 0x0000ff00;
  const Uint32 bmask = 0x00ff0000;
  const Uint32 amask = 0xff000000;
#endif
  return scp(SDL_CreateRGBSurfaceFrom((void *)pixels, width, height, 32,
                                      4 * width, rmask, gmask, bmask, amask));
}

#define ASCII_DISPLAY_HIGH 176
#define ASCII_DISPLAY_LOW 32
#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_ROWS 7
#define FONT_COLS 18
#define FONT_CHAR_WIDTH (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_SCALE 5.0f

typedef struct {
  SDL_Texture *spritesheet;
  SDL_Rect glyph_table[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW + 1];
} Font;

Font *font_load_from_file(SDL_Renderer *renderer, const char *path) {
  Font *font = (Font *)malloc(sizeof(Font));
  SDL_Surface *s = surface_from_file(path);
  scc(SDL_SetColorKey(s, SDL_TRUE, 0xFF000000));
  font->spritesheet = scp(SDL_CreateTextureFromSurface(renderer, s));
  SDL_FreeSurface(s);
  for (size_t i = ASCII_DISPLAY_LOW; i < ASCII_DISPLAY_HIGH; ++i) {
    const size_t index = i - ASCII_DISPLAY_LOW;
    const size_t row = index / FONT_COLS;
    const size_t col = index % FONT_COLS;

    font->glyph_table[index].x = (col * FONT_CHAR_WIDTH);
    font->glyph_table[index].y = (row * FONT_CHAR_HEIGHT);
    font->glyph_table[index].w = FONT_CHAR_WIDTH;
    font->glyph_table[index].h = FONT_CHAR_HEIGHT;
  }

  return font;
}

void set_texture_color(SDL_Texture *texture, Uint32 color) {
  scc(SDL_SetTextureAlphaMod(texture, (color >> (8 * 3)) & 0xFF));
  scc(SDL_SetTextureColorMod(texture, (color >> (8 * 2)) & 0xFF,
                             (color >> (8 * 1)) & 0xFF,
                             (color >> (8 * 0)) & 0xFF));
}

void render_char(SDL_Renderer *renderer, Font *font, const char c, Vec2f pos,
                 float scale) {
  const size_t index = c - 0x20;

  SDL_Rect dst = {.x = pos.x,
                  .y = pos.y,
                  .w = floorf(FONT_CHAR_WIDTH * scale),
                  .h = floorf(FONT_CHAR_HEIGHT * scale)};
  scc(SDL_RenderCopy(renderer, font->spritesheet, font->glyph_table + index,
                     &dst));
}

void render_text_sized(SDL_Renderer *renderer, Font *font, const char *text,
                       size_t text_size, Vec2f pos, Uint32 color, float scale) {
  set_texture_color(font->spritesheet, color);

  Vec2f offset = pos;

  for (size_t i = 0; i < text_size; ++i) {
    render_char(renderer, font, text[i], offset, scale);
    offset.x += FONT_CHAR_WIDTH * scale;
  }
}

void render_text(SDL_Renderer *renderer, Font *font, const char *text,
                 Vec2f pos, Uint32 color, float scale) {
  render_text_sized(renderer, font, text, strlen(text), pos, color, scale);
}

Line line = {0};
size_t cursor = 0;

#define UNHEX(color)                                        \
  ((color >> (8 * 3)) & 0xFF), ((color >> (8 * 2)) & 0xFF), \
      ((color >> (8 * 1)) & 0xFF), ((color >> (8 * 0)) & 0xFF)

void render_cursor(SDL_Renderer *renderer, Font *font) {
  const Vec2f pos = vec2f(cursor * FONT_CHAR_WIDTH * FONT_SCALE, 0.0f);
  const SDL_Rect rect = {
      .x = (int)pos.x,
      .y = (int)pos.y,
      .w = FONT_CHAR_WIDTH * FONT_SCALE,
      .h = FONT_CHAR_HEIGHT * FONT_SCALE,
  };

  scc(SDL_SetRenderDrawColor(renderer, UNHEX(0xFFFFFFFF)));
  scc(SDL_RenderFillRect(renderer, &rect));

  set_texture_color(font->spritesheet, 0xFF000000);
  if (cursor < line.size) {
    render_char(renderer, font, line.chars[cursor], pos, FONT_SCALE);
  }
}

int main() {
  scc(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *window =
      scp(SDL_CreateWindow("jed", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));

  SDL_Renderer *renderer =
      scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  Font *font = font_load_from_file(renderer, "./charmap-oldschool_white.png");

  int quit = 0;
  while (!quit) {
    SDL_Event event = {0};

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = 1;
          break;
        }
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_BACKSPACE: {
              line_backspace(&line, cursor);
              if (cursor > 0) {
                cursor -= 1;
              }
              break;
            }

            case SDLK_DELETE: {
              line_delete(&line, cursor);
              break;
            }

            case SDLK_LEFT: {
              if (cursor > 0) {
                cursor -= 1;
              }
              break;
            }

            case SDLK_RIGHT: {
              if (cursor < line.size) {
                cursor += 1;
              }
              break;
            }
          }
          break;
        }
        case SDL_TEXTINPUT: {
          line_insert_text_before_col(&line, event.text.text, cursor);
          cursor += strlen(event.text.text);
          break;
        }
        case SDL_KEYUP: {
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
              quit = 1;
              break;
          }
          break;
        }
      }
    }
    scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
    scc(SDL_RenderClear(renderer));

    render_text_sized(renderer, font, line.chars, line.size, vec2fs(0.0f),
                      0xFFFFFFFF, FONT_SCALE);
    render_cursor(renderer, font);

    SDL_RenderPresent(renderer);
  }
  SDL_Quit();
  return 0;
}
