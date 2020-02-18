R"(

#if __VERSION__ < 130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

precision highp float;

varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform vec2 u_position;
uniform vec2 u_size;
uniform vec2 u_viewport;
uniform float u_opacity;

uniform float u_scale;
uniform vec4 u_storytileRect;

// uv - in range <0,1>
// viewport - in pixels
// position - of left-bottom corner, in pixels
// size - in pixels
// radius - internal radius of corner rounding, in pixels
float tile(vec2 uv, vec2 viewport, vec2 position, vec2 size, float radius, float smoothingradius) {
    vec2 distance = abs(uv * viewport - (position + size * .5)) - (size * .5 - radius);
    return smoothstep(radius, radius - smoothingradius, length(max(distance, vec2(0))));
}

vec4 tileWithShadow(vec2 uv, vec2 viewport, vec2 position, vec2 size, float radius, vec3 contentRgb, vec2 shadowOffset, vec4 shadowColor) {
    float tileAlpha = tile(uv, viewport, position, size, radius, 2.);
    float shadowAlpha = tile(uv, viewport, position + shadowOffset, size, radius * 1.25, 10.);
    return vec4(mix(contentRgb, shadowColor.rgb, 1. - tileAlpha), max(tileAlpha, shadowAlpha * shadowColor.a));
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_viewport;

    float shadowOffset = u_size.x * .025 * u_scale * u_scale;
	vec2 shadowOffset2d = shadowOffset * vec2(1, -1);
    vec2 position = u_position + max(-shadowOffset2d, 0.);
    vec2 tileSize = u_size - abs(shadowOffset2d);
    float radius = u_size.x * .05;
    vec4 shadowColor = vec4(.0, .0, .0, 1.);
    vec3 contentColor = TEXTURE2D(s_texture, v_texCoord * u_storytileRect.zw + u_storytileRect.xy).rgb;

    gl_FragColor = tileWithShadow(uv, u_viewport, position, tileSize, radius, contentColor, shadowOffset2d, shadowColor) * vec4(vec3(1), u_opacity);
}

)"
