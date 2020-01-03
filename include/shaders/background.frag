R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform sampler2D s_texture2;
uniform float u_opacity;
uniform float u_mixing;
uniform vec2 u_viewport;

float tile(vec2 uv, vec2 viewport, vec2 position, vec2 size, float radious, float smoothingRadious) {
  vec2 distance = abs(uv * viewport - (position + size * .5)) - (size * .5 - radious);
  return smoothstep(radious, radious - smoothingRadious, length(max(distance, vec2(0))));
}

void main() {
	vec2 uv = gl_FragCoord.xy / u_viewport;
	float radius = 25.;
	float mask = tile(uv, u_viewport, vec2(0., 0.), u_viewport - vec2(0., 0.), radius, radius);
	float brightness = .5;
	vec3 texture = mix(TEXTURE2D(s_texture, v_texCoord).rgb, TEXTURE2D(s_texture2, v_texCoord).rgb, u_mixing);
	gl_FragColor = vec4(vec3(texture * brightness * mask), 1.);
}

)"
