R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

uniform vec3 u_color;
uniform vec3 u_shadowColor;
uniform vec2 u_shadowOffset;
uniform float u_opacity;
varying vec2 v_texCoord;
uniform sampler2D s_texture;

void main() {
	vec4 text = TEXTURE2D(s_texture, v_texCoord) * vec4(u_color, 1);
	vec4 shadow = TEXTURE2D(s_texture, v_texCoord + u_shadowOffset) * vec4(u_shadowColor, 1);
  gl_FragColor = mix(shadow, text, text.a) * vec4(vec3(1), u_opacity);
}

)"
