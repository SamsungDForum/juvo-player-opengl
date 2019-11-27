R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

uniform vec2 u_tileSize;
uniform vec2 u_tilePosition;
uniform vec4 u_frameColor;
uniform float u_frameWidth;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform float u_opacity;

float rect(vec2 uv, vec2 p, vec2 s) {
    vec2 stripe = min(step(p, uv), vec2(1., 1.) - step(p + s, uv));
    return min(stripe.x, stripe.y);
}

float rectEdge(vec2 uv, vec2 p, vec2 s, float b) {
    return clamp(rect(uv, p, s) - rect(uv, p + b, s - 2. * b), 0., 1.);
}

void main() {
    float border = rectEdge(gl_FragCoord.xy, u_tilePosition, u_tileSize, u_frameWidth);
    gl_FragColor = vec4(mix(TEXTURE2D(s_texture, v_texCoord).rgb, vec3(1.), border), u_opacity);
}

)"
