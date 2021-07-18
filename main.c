#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
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
  Font *shfont = (Font *)malloc(sizeof(Font));
  SDL_Surface *s = surface_from_file(path);
  scc(SDL_SetColorKey(s, SDL_TRUE, 0xFF000000));
  shfont->spritesheet = scp(SDL_CreateTextureFromSurface(renderer, s));
  SDL_FreeSurface(s);
  for (size_t i = ASCII_DISPLAY_LOW; i < ASCII_DISPLAY_HIGH; ++i) {
    const size_t index = i - ASCII_DISPLAY_LOW;
    const size_t row = index / FONT_COLS;
    const size_t col = index % FONT_COLS;

    shfont->glyph_table[index].x = (col * FONT_CHAR_WIDTH);
    shfont->glyph_table[index].y = (row * FONT_CHAR_HEIGHT);
    shfont->glyph_table[index].w = FONT_CHAR_WIDTH;
    shfont->glyph_table[index].h = FONT_CHAR_HEIGHT;
  }

  return shfont;
}

void set_texture_color(SDL_Texture *texture, Uint32 color) {
  scc(SDL_SetTextureAlphaMod(texture, (color >> (8 * 3)) & 0xFF));
  scc(SDL_SetTextureColorMod(texture, (color >> (8 * 2)) & 0xFF,
                             (color >> (8 * 1)) & 0xFF,
                             (color >> (8 * 0)) & 0xFF));
}

void render_char(SDL_Renderer *renderer, Font *shfont, const char c, Vec2f pos,
                 float scale) {
  const size_t index = c - 0x20;

  SDL_Rect dst = {.x = pos.x,
                  .y = pos.y,
                  .w = floorf(FONT_CHAR_WIDTH * scale),
                  .h = floorf(FONT_CHAR_HEIGHT * scale)};
  scc(SDL_RenderCopy(renderer, shfont->spritesheet, shfont->glyph_table + index,
                     &dst));
}

void render_text_sized(SDL_Renderer *renderer, Font *shfont, const char *text,
                       size_t text_size, Vec2f pos, Uint32 color, float scale) {
  set_texture_color(shfont->spritesheet, color);

  Vec2f offset = pos;

  for (size_t i = 0; i < text_size; ++i) {
    render_char(renderer, shfont, text[i], offset, scale);
    offset.x += FONT_CHAR_WIDTH * scale;
  }
}

void render_text(SDL_Renderer *renderer, Font *shfont, const char *text,
                 Vec2f pos, Uint32 color, float scale) {
  render_text_sized(renderer, shfont, text, strlen(text), pos, color, scale);
}

#define BUFFER_CAPACITY 1024
char buffer[BUFFER_CAPACITY];
size_t buffer_cursor = 0;
size_t buffer_size = 0;

#define UNHEX(color)                                        \
  ((color >> (8 * 3)) & 0xFF), ((color >> (8 * 2)) & 0xFF), \
      ((color >> (8 * 1)) & 0xFF), ((color >> (8 * 0)) & 0xFF)

void render_cursor(SDL_Renderer *renderer, Font *shfont) {
  const Vec2f pos = vec2f(buffer_cursor * FONT_CHAR_WIDTH * FONT_SCALE, 0.0f);
  const SDL_Rect rect = {
      .x = (int)pos.x,
      .y = (int)pos.y,
      .w = FONT_CHAR_WIDTH * FONT_SCALE,
      .h = FONT_CHAR_HEIGHT * FONT_SCALE,
  };

  scc(SDL_SetRenderDrawColor(renderer, UNHEX(0xFFFFFFFF)));
  scc(SDL_RenderFillRect(renderer, &rect));

  set_texture_color(shfont->spritesheet, 0xFF000000);
  if (buffer_cursor < buffer_size) {
    render_char(renderer, shfont, buffer[buffer_cursor], pos, FONT_SCALE);
  }
}

void buffer_insert_text_before_cursor(const char *text) {
  size_t text_size = strlen(text);
  const size_t free_space = BUFFER_CAPACITY - buffer_size;
  if (text_size > free_space) {
    text_size = free_space;
  }
  memmove(buffer + buffer_cursor + text_size, buffer + buffer_cursor,
          buffer_size - buffer_cursor);
  memcpy(buffer + buffer_cursor, text, text_size);
  buffer_cursor += text_size;
  buffer_size += text_size;
}

void buffer_delete_before_cursor() {
  if (buffer_size > 0 && buffer_cursor > 0) {
    memmove(buffer + buffer_cursor - 1, buffer + buffer_cursor,
            buffer_size - buffer_cursor);
    buffer_size -= 1;
    buffer_cursor -= 1;
  }
}

void buffer_delete_beginning_at_cursor() {
  if (buffer_cursor < buffer_size) {
    memmove(buffer + buffer_cursor, buffer + buffer_cursor + 1,
            buffer_size - buffer_cursor - 1);
    buffer_size -= 1;
  }
}

int main1() {
  scc(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *window =
      scp(SDL_CreateWindow("jed", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));

  SDL_Renderer *renderer =
      scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  Font *shfont = font_load_from_file(renderer, "./charmap-oldschool_white.png");

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
              buffer_delete_before_cursor();
              break;
            }
            case SDLK_DELETE: {
              buffer_delete_beginning_at_cursor();
              break;
            }
            case SDLK_LEFT: {
              if (buffer_cursor > 0) {
                buffer_cursor -= 1;
              }
              break;
            }
            case SDLK_RIGHT: {
              if (buffer_cursor < buffer_size) {
                buffer_cursor += 1;
              }
              break;
            }
          }
          break;
        }
        case SDL_TEXTINPUT: {
          buffer_insert_text_before_cursor(event.text.text);
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

    render_text_sized(renderer, shfont, buffer, buffer_size, vec2fs(0.0f),
                      0xFFFFFFFF, FONT_SCALE);
    render_cursor(renderer, shfont);

    SDL_RenderPresent(renderer);
  }
  SDL_Quit();
  return 0;
}

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                     GLsizei length, const GLchar *message,
                     const void *userParam) {
  (void)source;
  (void)id;
  (void)length;
  (void)userParam;
  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity,
          message);
}

#include "gl_extra.h"

typedef struct {
  Vec2f pos;
  float scale;
  int ch;
  Vec4f color;
} Glyph;

typedef enum {
  GLYPH_ATTR_POS = 0,
  GLYPH_ATTR_SCALE,
  GLYPH_ATTR_CH,
  GLYPH_ATTR_COLOR,
  COUNT_GLYPH_ATTRS
} Glyph_Attr;

typedef struct {
  size_t offset;
  size_t comps;
} Glyph_Attr_Def;

static const Glyph_Attr_Def glyph_attr_defs[COUNT_GLYPH_ATTRS] = {
    [GLYPH_ATTR_POS] = {.offset = offsetof(Glyph, pos), .comps = 2},
    [GLYPH_ATTR_SCALE] = {.offset = offsetof(Glyph, scale), .comps = 1},
    [GLYPH_ATTR_CH] = {.offset = offsetof(Glyph, ch), .comps = 1},
    [GLYPH_ATTR_COLOR] = {.offset = offsetof(Glyph, color), .comps = 4},
};
static_assert(COUNT_GLYPH_ATTRS == 4,
              "The amount of glyph vertex attributes have changed");

#define GLYPH_BUFFER_CAP 1024
Glyph glyph_buffer[GLYPH_BUFFER_CAP];
size_t glyph_buffer_count = 0;

void glyph_buffer_push(Glyph glyph) {
  assert(glyph_buffer_count < GLYPH_BUFFER_CAP);
  glyph_buffer[glyph_buffer_count++] = glyph;
}

void glyph_buffer_sync(void) {
  glBufferSubData(GL_ARRAY_BUFFER, 0, glyph_buffer_count * sizeof(Glyph),
                  glyph_buffer);
}

void gl_render_text(const char *text, size_t text_size, Vec2f pos, float scale,
                    Vec4f color) {
  const Vec2f char_size = vec2f(FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT);
  for (size_t i = 0; i < text_size; ++i) {
    Glyph glyph = {
        .pos = vec2f_add(
            pos, vec2f_mul3(char_size, vec2f((float)i, 0.0f), vec2fs(scale))),
        .scale = scale,
        .ch = text[i],
        .color = color};
    glyph_buffer_push(glyph);
  }
}

int main(void) {
  scc(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *window =
      scp(SDL_CreateWindow("jed", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    printf("GL Version %d.%d\n", major, minor);
  }

  scp(SDL_GL_CreateContext(window));

  if (GLEW_OK != glewInit()) {
    fprintf(stderr, "Could not initialize GLEW!");
    exit(EXIT_FAILURE);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (GLEW_ARB_debug_output) {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
  }

  GLuint time_uniform = 0;
  GLuint resolution_uniform = 0;
  // Initialize shader program
  {
    GLuint vert_shader = 0;
    if (!compile_shader_file("./shaders/font.vert", GL_VERTEX_SHADER,
                             &vert_shader)) {
      exit(1);
    }
    GLuint frag_shader = 0;
    if (!compile_shader_file("./shaders/font.frag", GL_FRAGMENT_SHADER,
                             &frag_shader)) {
      exit(1);
    }

    GLuint program = 0;
    if (!link_program(vert_shader, frag_shader, &program)) {
      exit(1);
    }

    glUseProgram(program);

    time_uniform = glGetUniformLocation(program, "time");
    resolution_uniform = glGetUniformLocation(program, "resolution");
    glUniform2f(resolution_uniform, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  // Initialize font texture
  {
    int width, height, n;
    const char *file_path = "./charmap-oldschool_white.png";
    unsigned char *pixels =
        stbi_load(file_path, &width, &height, &n, STBI_rgb_alpha);
    if (pixels == NULL) {
      fprintf(stderr, "ERROR: could not load image %s: %s\n", file_path,
              stbi_failure_reason());
      exit(1);
    }

    glActiveTexture(GL_TEXTURE0);

    GLuint font_texture = 0;
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);
  }

  // Initialize buffers
  {
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glyph_buffer), glyph_buffer,
                 GL_DYNAMIC_DRAW);

    for (Glyph_Attr attr = 0; attr < COUNT_GLYPH_ATTRS; ++attr) {
      glEnableVertexAttribArray(attr);
      glVertexAttribPointer(attr, glyph_attr_defs[attr].comps, GL_FLOAT,
                            GL_FALSE, sizeof(Glyph),
                            (void *)glyph_attr_defs[attr].offset);
      glVertexAttribDivisor(attr, 1);
    }
  }

  const char *text = "Hello, world";
  gl_render_text(text, strlen(text), vec2fs(0.0f), 5.0f,
                 vec4f(1.0f, 0.0f, 0.0f, 1.0f));
  glyph_buffer_sync();

  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        } break;

        case SDL_KEYUP: {
          if (event.key.keysym.sym == SDLK_ESCAPE) {
            quit = true;
          }
        } break;
      }
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform1f(time_uniform, (float)SDL_GetTicks() / 1000.0f);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, glyph_buffer_count);

    /* glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); */
    SDL_GL_SwapWindow(window);
  }
  return 0;
}
