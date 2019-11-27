R"(

precision mediump float;
uniform vec3 u_color;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
  gl_FragColor = vec4(u_color, texture2D(s_texture, v_texCoord).r);
}

)"
