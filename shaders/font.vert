#version 330 core

uniform vec2 resolution;
uniform float scale;
uniform vec2 camera;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 size;
layout(location = 2) in vec2 uv_pos;
layout(location = 3) in vec2 uv_size;
layout(location = 4) in vec4 fg_color;
layout(location = 5) in vec4 bg_color;

out vec2 uv;
out vec2 glyph_uv_size;
out vec2 glyph_uv_pos;
out vec4 glyph_fg_color;
out vec4 glyph_bg_color;

vec2 project_point(vec2 p)
{
  return 2.0 * (p - camera) / resolution;
}

void main()
{
  uv = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
  vec2 p = (uv * size + pos) * scale;
  gl_Position = vec4(project_point(p), 0.0, 1.0);
  // gl_Position = vec4(uv*vec2(1.0, -1.0), 0.0, 1.0);

	glyph_uv_pos = uv_pos;
	glyph_uv_size = uv_size;
  glyph_fg_color = fg_color;
  glyph_bg_color = bg_color;
}
