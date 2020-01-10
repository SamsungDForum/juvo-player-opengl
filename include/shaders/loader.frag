R"(

precision highp float;

uniform float u_time;
uniform float u_param;
uniform vec2 u_viewport;

float hash(vec2 n) {
    return fract(cos(dot(n, vec2(36.26, 73.12))) * 354.63);
}

float vnoise(vec2 n) {
    vec2 fn = floor(n);
    vec2 sn = smoothstep(0., 1., fract(n));
    return mix(mix(hash(fn), hash(fn + vec2(1, 0)), sn.x), mix(hash(fn + vec2(0, 1)), hash(fn + 1.), sn.x), sn.y);
}

float valueNoise(vec2 n) {
    return vnoise(n / 32.) * .5875 + vnoise(n / 16.) * .2 + vnoise(n / 8.) * .1 + vnoise(n / 4.) * .05 + vnoise(n / 2.) * .025 + vnoise(n) * .0125;
}

float segment(vec2 p, vec2 a, vec2 b) {
    vec2 ap = p - a;
    vec2 ab = b - a;
    float t = clamp(dot(ab, ap) / dot(ab, ab), 0., 1.);
    vec2 c = a + t * ab;
    return length(p - c);
}

void main() {
    const vec2 size = vec2(1000, 100);
    vec2 pp = gl_FragCoord.xy / u_viewport.y;
    float a = (u_viewport.x - size.x) * .5 / u_viewport.y;
    float b = a + size.x / u_viewport.y;
    const float h = .5;
    float barRadius = size.y * .25 / u_viewport.y;
    float dotRadius = barRadius * 1.5;
    vec2 A = vec2(a + barRadius, h);
    vec2 B = vec2(b - barRadius, h);
    float dotPosition = A.x + u_param * .01 * (B.x - A.x);

    vec3 lightColor = vec3(.1, .4, 1);
    float d = segment(pp, A, vec2(dotPosition, h));
    float cloudIntensity = smoothstep(.25, .0, d);
    float lightIntensity = smoothstep(.01, .0, d);

    vec3 lightColor2 = vec3(.2, .2, .2);
    float d2 = segment(pp, A, B);
    float cloudIntensity2 = smoothstep(.25, .0, d2);
    float lightIntensity2 = smoothstep(.01, .0, d2);

    float clouds = valueNoise((gl_FragCoord.xy + vec2(u_time * 100., 0.)) * .5);
    gl_FragColor = vec4(vec3((cloudIntensity * clouds + lightIntensity) * lightColor + (cloudIntensity2 * clouds + lightIntensity2) * lightColor2), 1);
}

)"
