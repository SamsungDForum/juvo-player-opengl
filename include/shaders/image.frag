R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

varying vec2 v_texCoord;
uniform sampler2D s_texture;

void main() {
	gl_FragColor = TEXTURE2D(s_texture, v_texCoord);
}

)"
