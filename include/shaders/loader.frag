R"(

precision highp float;

uniform float u_percent;
uniform vec2 u_viewport;
uniform vec2 u_position;
uniform vec2 u_size;
uniform vec3 u_fgcolor;
uniform vec3 u_bgcolor;

float tile(vec2 uv, vec2 viewport, vec2 position, vec2 size, float radious, float smoothingRadious) {
    vec2 distance = abs(uv * viewport - (position + size * .5)) - (size * .5 - radious);
    return smoothstep(radious, radious - smoothingRadious, length(max(distance, vec2(0))));
}

void main() {
  vec2 uv = gl_FragCoord.xy / u_viewport;
	float pc = clamp(u_percent, 0., 1.);
	vec3 bg = u_bgcolor * tile(uv, u_viewport, u_position + vec2(u_size.x, 0.) * pc, u_size * vec2(1. - pc, 1.), 1e-9, 1e-9);
	vec3 fg = u_fgcolor * tile(uv, u_viewport, u_position, u_size * vec2(pc, 1.), 1e-9, 1e-9);
  gl_FragColor = vec4(bg + fg, 1.);
}

)"
