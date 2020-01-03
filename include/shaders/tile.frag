R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

uniform vec2 u_tileSize;
uniform vec2 u_tilePosition;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform float u_opacity;
uniform vec2 u_viewport;
uniform float u_scale;
uniform vec4 u_storytileRect;

// uv - in range <0,1>
// viewport - in pixels
// position - of left-bottom corner, in pixels
// size - in pixels
// radious - internal radious of corner rounding, in pixels
float tile(vec2 uv, vec2 viewport, vec2 position, vec2 size, float radious, float smoothingRadious) {
    vec2 distance = abs(uv * viewport - (position + size * .5)) - (size * .5 - radious);
    return smoothstep(radious, radious - smoothingRadious, length(max(distance, vec2(0))));
}

vec4 tileWithShadow(vec2 uv, vec2 viewport, vec2 position, vec2 size, float radious, vec3 contentRgb, vec2 shadowOffset, vec4 shadowColor) {
    float tileAlpha = tile(uv, viewport, position, size, radious, 2.);
    float shadowAlpha = tile(uv, viewport, position + shadowOffset, size, radious * 1.25, 10.);
    return vec4(mix(contentRgb, shadowColor.rgb, 1. - tileAlpha), max(tileAlpha, shadowAlpha * shadowColor.a));
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_viewport;

    vec2 size = u_tileSize;
    float shadowOffset = u_tileSize.x * .025 * u_scale * u_scale;
	vec2 shadowOffset2d = shadowOffset * vec2(1, -1);
    vec2 position = u_tilePosition + max(-shadowOffset2d, 0.);
    vec2 tileSize = size - abs(shadowOffset2d);
    float radious = u_tileSize.x * .05;
    vec4 shadowColor = vec4(.0, .0, .0, 1.);
    vec3 contentColor = TEXTURE2D(s_texture, v_texCoord * u_storytileRect.zw + u_storytileRect.xy).rgb;

    gl_FragColor = tileWithShadow(uv, u_viewport, position, tileSize, radious, contentColor, shadowOffset2d, shadowColor) * vec4(vec3(1), u_opacity);
}

)"
