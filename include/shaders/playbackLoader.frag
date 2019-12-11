R"(

precision highp float;

#define M_PI 3.14159265359

uniform float u_param;
uniform float u_opacity;
uniform vec2 u_viewport;
uniform vec2 u_size;

void main() {
  float r1 = u_size.y * .333;
  float r2 = u_size.y * .5;
  vec2 d = gl_FragCoord.xy - u_viewport.xy * .5;
  float r = length(d);
  float t = fract(.5 * (1. - atan(d.y, d.x) / M_PI) - u_param);
  float c2 = smoothstep(r2, r2 - 4., r);
  float c1 = smoothstep(r1, r1 - 4., r);
  gl_FragColor = vec4(t * (c2 - c1) * u_opacity);
}

)"
