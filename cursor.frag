#version 330

uniform float time;
uniform float last_stroke;

#define PERIOD 1.0
#define BLINK_THRESHOLD 0.5

void main()
{
	float t = time - last_stroke;
	float r = float(mod(t, 1.5) < BLINK_THRESHOLD);
	gl_FragColor = vec4(1.0) * r;
}
