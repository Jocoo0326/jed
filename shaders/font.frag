#version 330 core

#define FONT_ROWS 7
#define FONT_COLS 18
#define FONT_CHAR_WIDTH (1.0 / FONT_COLS)
#define FONT_CHAR_HEIGHT (1.0 / FONT_ROWS)

uniform sampler2D font;
uniform float time;

in vec2 uv;
in float glyph_ch;
in vec4 glyph_color;

void main() {
  int ch = int(glyph_ch);
  if (!(ch >= 32 && ch <= 127)) {
    ch = 63;
  }

  int index = ch - 32;

  float x = float(index % FONT_COLS) * FONT_CHAR_WIDTH;
  float y = float(index / FONT_COLS) * FONT_CHAR_WIDTH;

  vec2 t = vec2(x, y) + vec2(FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT) * uv;
  gl_FragColor = texture(font, t) * glyph_color;
}
