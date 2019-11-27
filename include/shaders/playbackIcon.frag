R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

#define M_PI 3.14159265359
#define FLAT_SHADING_EDGE_WIDTH 8.0

uniform vec3 u_color;
uniform vec3 u_shadowColor;
uniform vec2 u_shadowOffset;
uniform float u_opacity;
varying vec2 v_texCoord;
uniform sampler2D s_texture;

uniform vec3 u_bloomColor;
uniform float u_bloomOpacity;
uniform vec4 u_bloomRect;

float point(vec2 p, vec2 a) {
    return length(p - a);
}

float shadeFlat(float d, float r) {
    return smoothstep(r, r - FLAT_SHADING_EDGE_WIDTH, d);
}

float rect(vec2 uv, vec2 p, vec2 s) {
    vec2 stripe = min(step(p, uv), vec2(1., 1.) - step(p + s, uv));
    return min(stripe.x, stripe.y);
}

float rectEdge(vec2 uv, vec2 p, vec2 s, float b) {
    return clamp(rect(uv, p, s) - rect(uv, p + b, s - 2. * b), 0., 1.);
}

vec4 circleBloom(vec2 center, vec2 size, vec4 color) {
    return .5 * color * shadeFlat(point(gl_FragCoord.xy, center), min(size.x, size.y) / 2.);
}

vec4 rectBloom(vec2 center, vec2 size, vec4 color) {
	return color * max(.25, rectEdge(gl_FragCoord.xy, center - size / 2., size, 2.));
}

vec4 bloom(vec2 center, vec2 size, vec4 color) {
	return rectBloom(center, size, color);
}

void main() {
	vec4 icon = TEXTURE2D(s_texture, v_texCoord) * vec4(u_color, 1);
	vec4 shadow = TEXTURE2D(s_texture, v_texCoord - u_shadowOffset * vec2(1, -1)) * vec4(u_shadowColor, 1);
	vec4 iconWithShadow = mix(shadow, icon, icon.a);
	vec4 bloomv = bloom(u_bloomRect.st, u_bloomRect.pq, vec4(u_bloomColor, u_bloomOpacity));
	gl_FragColor = mix(bloomv, iconWithShadow, iconWithShadow.a) * vec4(vec3(1), u_opacity);
}

)"
