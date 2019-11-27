R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform float u_opacity;
uniform float u_black;
uniform vec2 u_viewport;

void main() {
    float mixFactor = clamp((-atan(gl_FragCoord.x / u_viewport.x - .5) / (1.57079632679) + .6) + pow(1. - gl_FragCoord.y / u_viewport.y, 6.) , 0., 1.);
    gl_FragColor = vec4(mix(TEXTURE2D(s_texture, v_texCoord).rgb, vec3(0), mixFactor), u_opacity);
    gl_FragColor.rgb = mix(gl_FragColor.rgb, vec3(0), u_black);
}

)"
