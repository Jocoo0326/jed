#version 330

uniform vec2 pos;
uniform vec2 size;
uniform vec2 resolution;
uniform vec2 camera;

vec2 project_point(vec2 p)
{
  return (2.0 * (p - camera) / resolution);
}

void main()
{
  vec2 uv = vec2((gl_VertexID & 1), ((gl_VertexID >> 1) & 1));
  gl_Position = vec4(project_point(pos + uv * size), 0.0, 1.0);
}
