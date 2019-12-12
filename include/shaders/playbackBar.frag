R"(

precision highp float;

#define M_PI 3.14159265359
#define FLAT_SHADING_EDGE_WIDTH 0.002

uniform float u_param;
uniform float u_opacity;
uniform vec2 u_viewport;
uniform vec2 u_size;
uniform float u_margin;
uniform float u_dot_scale;

float point(vec2 p, vec2 a) {
    return length(p - a);
}

float segment(vec2 p, vec2 a, vec2 b) {
    vec2 ap = p - a;
    vec2 ab = b - a;
    float t = clamp(dot(ab, ap) / dot(ab, ab), 0., 1.);
    vec2 c = a + t * ab;
    return length(p - c);
}

float shadeFlat(float d, float r) {
    return smoothstep(r, r - FLAT_SHADING_EDGE_WIDTH, d);
}

float shade3d(float d, float r) {
    return sin(clamp(1. - d / r, 0., 1.) * M_PI * .5);
}

void main()
{
    vec2 p = gl_FragCoord.xy / u_viewport.y;
    float a = (u_viewport.x - u_size.x) * .5 / u_viewport.y;
    float b = a + u_size.x / u_viewport.y;
    float h = (u_margin + u_size.y * .5) / u_viewport.y;

    float barRadius = u_size.y * .5 / u_viewport.y;
    float dotRadius = barRadius * u_dot_scale;
    vec2 A = vec2(a + barRadius, h);
    vec2 B = vec2(b - barRadius, h);
    float dotPosition = A.x + u_param * (B.x - A.x);
    vec2 shadowOffset = vec2(-.001);

    float shadowBar = shadeFlat(segment(p, A + shadowOffset, B + shadowOffset), barRadius);
    float whiteBar = shadeFlat(segment(p, A, vec2(dotPosition, h)), barRadius);
    float greyBar = shadeFlat(segment(p, A, B), barRadius);
    float shadowDot = shadeFlat(point(p, vec2(dotPosition, h) + shadowOffset * .5), dotRadius);
    float whiteDot = shadeFlat(point(p, vec2(dotPosition, h)), dotRadius);

    vec3 greyBarColor = vec3(.5);
    vec3 whiteBarColor = vec3(1);
    vec3 shadowBarColor = vec3(0);
    vec3 shadowDotColor = vec3(0);
    vec3 whiteDotColor = vec3(1);

    vec4 c = mix(vec4(greyBar * greyBarColor, greyBar * .5), vec4(whiteBar * whiteBarColor, whiteBar), whiteBar);
    c = vec4(mix(shadowBar * shadowBarColor, c.rgb, step(.1, c.a)), max(c, shadowBar * .5));
    c = mix(c, vec4(shadowDot * shadowDotColor, shadowDot), shadowDot);
    c = mix(c, vec4(whiteDot * whiteDotColor, whiteDot), whiteDot);
    gl_FragColor = c * vec4(vec3(1), u_opacity);
}

)"
