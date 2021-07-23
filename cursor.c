#include "cursor.h"

void gl_render_cursor(Glyph_Info *glyph_info, Editor *editor)
{
  const char* c = editor_char_under_cursor(editor);
  Vec2f tile = vec2f(editor->cursor_col * glyph_info->cw,
                     -(int)editor->cursor_row * glyph_info->th);
  /* fr_render_text_sized(glyph_info, c ? c : " ", 1, tile, vec4f(1.0f, 0.0f, 0.0f, 1.0f), */
  /*                      vec4fs(1.0f)); */
}
