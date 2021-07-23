#include "free_font.h"

void fr_glyph_buffer_clear(Free_Render* fr)
{
  fr->glyph_buffer_count = 0;
}

void fr_glyph_buffer_push(Free_Render* fr, Glyph glyph)
{
  assert(fr->glyph_buffer_count < GLYPH_BUFFER_CAP);
  fr->glyph_buffer[(fr->glyph_buffer_count)++] = glyph;
}

void fr_glyph_buffer_sync(Free_Render* fr)
{
  glBufferSubData(GL_ARRAY_BUFFER, 0,
                  fr->glyph_buffer_count * sizeof(Glyph), fr->glyph_buffer);
}

void fr_init(Free_Render* fr, const char *font_file, int sw, int sh)
{
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

    fr->time_uniform = glGetUniformLocation(program, "time");
    fr->resolution_uniform = glGetUniformLocation(program, "resolution");
    fr->scale_uniform = glGetUniformLocation(program, "scale");
    fr->cursor_uniform = glGetUniformLocation(program, "cursor");
    fr->camera_uniform = glGetUniformLocation(program, "camera");

    glUniform2f(fr->resolution_uniform, sw, sh);
    glUniform1f(fr->scale_uniform, FONT_SCALE);
    glUniform2i(fr->cursor_uniform, 0, 0);
  }

  // Initialize buffers
  {
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fr->glyph_buffer), fr->glyph_buffer,
                 GL_DYNAMIC_DRAW);

    for (Glyph_Attr attr = 0; attr < COUNT_GLYPH_ATTRS; ++attr) {
      glEnableVertexAttribArray(attr);
      switch (glyph_attr_defs[attr].type) {
      case GL_FLOAT: {
        glVertexAttribPointer(
            attr, glyph_attr_defs[attr].comps, glyph_attr_defs[attr].type,
            GL_FALSE, sizeof(Glyph), (void*)glyph_attr_defs[attr].offset);
      } break;

      case GL_INT: {
        glVertexAttribIPointer(attr, glyph_attr_defs[attr].comps,
                               glyph_attr_defs[attr].type, sizeof(Glyph),
                               (void*)glyph_attr_defs[attr].offset);
      } break;
      }
      glVertexAttribDivisor(attr, 1);
    }
  }

	fr_init_font_texture(fr, font_file);
}

void fr_init_font_texture(Free_Render* fr, const char *font_file)
{
  fr->font_pixel_size = 26;
  FT_Library library = {0};
  FT_Face face;

  FT_Error error = FT_Init_FreeType(&library);
  if (error) {
    fprintf(stderr, "ERROR: Could not initialize FreeType2 library");
    exit(1);
  }

  error = FT_New_Face(library, font_file, 0, &face);
  if (error == FT_Err_Unknown_File_Format) {
    fprintf(stderr, "ERROR: `%s` has an unknown format", font_file);
    exit(1);
  } else if (error) {
    fprintf(stderr, "ERROR: could not load font file `%s`", font_file);
    exit(1);
  }

  if (FT_Set_Pixel_Sizes(face, 0, fr->font_pixel_size)) {
    fprintf(stderr, "ERROR: could not set pixel size to %u\n",
            fr->font_pixel_size);
    exit(1);
  }

  FT_GlyphSlot g = face->glyph;
  int w = 0, h = 0;
  for (int i = ASCII_DISPLAY_LOW; i < ASCII_DISPLAY_HIGH; ++i) {
    if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
      fprintf(stderr, "ERROR: could not load character %c\n", i);
      continue;
    }
    w += g->bitmap.width;
    h = h < (int)g->bitmap.rows ? (int)g->bitmap.rows : h;
  }
  fr->glyph_info.tw = w;
  fr->glyph_info.th = h;

  GLuint font_texture = 0;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &font_texture);
  glBindTexture(GL_TEXTURE_2D, font_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE,
               0);

  int x = 0;
  for (int i = ASCII_DISPLAY_LOW; i < ASCII_DISPLAY_HIGH; ++i) {
    if (FT_Load_Char(face, i, FT_LOAD_DEFAULT)) {
      continue;
    }

    if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP &&
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
      fprintf(stderr, "could not render glyph %c\n", (char)i);
      exit(1);
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows,
                    GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

    int p = i - ASCII_DISPLAY_LOW;
    Glyph_Metric* m = fr->glyph_info.glyph_metrics + p;
    m->ax = g->advance.x >> 6;
    m->ay = g->advance.y >> 6;

    m->bw = g->bitmap.width;
    m->bh = g->bitmap.rows;

    m->bl = g->bitmap_left;
    m->bt = g->bitmap_top;

    m->tx = (float)((float)x / w);

    x += g->bitmap.width;
  }
  fr->glyph_info.cw = fr->glyph_info.glyph_metrics['a' - 32].ax;
  fr->glyph_info.ch = h;
}

void fr_render_text_sized(Free_Render* fr, const char* text,
                          size_t text_size, Vec2f pos, Vec4f fg_color,
                          Vec4f bg_color)
{
  float x1 = 0.0f, y1 = 0.0f;
  float w = 0.0f, h = 0.0f;
  for (size_t i = 0; i < text_size; ++i) {
    const Glyph_Metric* m =
        fr->glyph_info.glyph_metrics + (text[i] - ASCII_DISPLAY_LOW);
    x1 = pos.x + m->bl;
    y1 = pos.y + m->bt;
    w = m->bw;
    h = m->bh;
    Glyph glyph = {.pos = vec2f(x1, y1),
                   .size = vec2f(w, -h),
                   .uv_pos = vec2f(m->tx, 0.0f),
                   .uv_size = vec2f((float)m->bw / fr->glyph_info.tw,
                                    (float)m->bh / fr->glyph_info.th),
                   .fg_color = fg_color,
                   .bg_color = bg_color};
    pos.x += m->ax;
    fr_glyph_buffer_push(fr, glyph);
  }
}

void fr_render_text(Free_Render* fr, const char* text, Vec2f tile,
                    Vec4f fg_color, Vec4f bg_color)
{
  fr_render_text_sized(fr, text, strlen(text), tile, fg_color, bg_color);
}
