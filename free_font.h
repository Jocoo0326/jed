#ifndef FREE_FONT_H
#define FREE_FONT_H

#include <GL/glew.h>
#include <assert.h>
#include <stdlib.h>
#include "editor.h"
#include "la.h"
#include "file.h"
#include "gl_extra.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define ASCII_DISPLAY_HIGH 128
#define ASCII_DISPLAY_LOW 32
#define FONT_SCALE 1.0f

typedef struct {
  Vec2f pos;
  Vec2f size;
  Vec2f uv_pos;
  Vec2f uv_size;
  Vec4f fg_color;
  Vec4f bg_color;
} Glyph;

typedef enum {
  GLYPH_ATTR_POS = 0,
  GLYPH_ATTR_SIZE,
  GLYPH_ATTR_UV_POS,
  GLYPH_ATTR_UV_SIZE,
  GLYPH_ATTR_FG_COLOR,
  GLYPH_ATTR_BG_COLOR,
  COUNT_GLYPH_ATTRS
} Glyph_Attr;

typedef struct {
  size_t offset;
  size_t comps;
  GLenum type;
} Glyph_Attr_Def;

static const Glyph_Attr_Def glyph_attr_defs[COUNT_GLYPH_ATTRS] = {
    [GLYPH_ATTR_POS] = {.offset = offsetof(Glyph, pos),
                        .comps = 2,
                        .type = GL_FLOAT},
    [GLYPH_ATTR_SIZE] = {.offset = offsetof(Glyph, size),
                         .comps = 2,
                         .type = GL_FLOAT},
    [GLYPH_ATTR_UV_POS] = {.offset = offsetof(Glyph, uv_pos),
                           .comps = 2,
                           .type = GL_FLOAT},
    [GLYPH_ATTR_UV_SIZE] = {.offset = offsetof(Glyph, uv_size),
                            .comps = 2,
                            .type = GL_FLOAT},
    [GLYPH_ATTR_FG_COLOR] = {.offset = offsetof(Glyph, fg_color),
                             .comps = 4,
                             .type = GL_FLOAT},
    [GLYPH_ATTR_BG_COLOR] = {.offset = offsetof(Glyph, bg_color),
                             .comps = 4,
                             .type = GL_FLOAT},
};
static_assert(COUNT_GLYPH_ATTRS == 6,
              "The amount of glyph vertex attributes have changed");

typedef struct {
  float ax;  // advance.x
  float ay;  // advance.y

  float bw;  // bitmap.width;
  float bh;  // bitmap.rows;

  float bl;  // bitmap_left;
  float bt;  // bitmap_top;

  float tx;  // x offset of glyph in texture coordinates
} Glyph_Metric;

typedef struct {
  Glyph_Metric glyph_metrics[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW];
  float tw;
  float th;
  float cw;
  float ch;
} Glyph_Info;

#define GLYPH_BUFFER_CAP 1024 * 640
typedef struct {
  Glyph_Info glyph_info;
  Glyph glyph_buffer[GLYPH_BUFFER_CAP];
  size_t glyph_buffer_count;
  FT_UInt font_pixel_size;
  GLuint time_uniform;
  GLuint resolution_uniform;
  GLuint scale_uniform;
  GLuint cursor_uniform;
  GLuint camera_uniform;
} Free_Render;

void fr_init(Free_Render* fr, const char *font_file, int sw, int sh);

void fr_glyph_buffer_clear(Free_Render* fr);

void fr_glyph_buffer_push(Free_Render* fr, Glyph glyph);

void fr_glyph_buffer_sync(Free_Render* fr);

void fr_init_font_texture(Free_Render* fr, const char *font_file);

void fr_render_text_sized(Free_Render* fr, const char* text,
                          size_t text_size, Vec2f pos, Vec4f fg_color,
                          Vec4f bg_color);

void fr_render_text(Free_Render* fr, const char* text, Vec2f tile,
                    Vec4f fg_color, Vec4f bg_color);

#endif /* FREE_FONT_H */
