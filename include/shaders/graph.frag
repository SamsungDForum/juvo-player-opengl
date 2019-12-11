R"(

precision highp float;

const int VALUES = 100;

uniform vec2 u_position;
uniform vec2 u_size;
uniform float u_value[VALUES];
uniform vec3 u_color;
uniform float u_opacity;

float rect(vec2 uv, vec2 p, vec2 s) {
    vec2 stripe = min(step(p, uv), vec2(1., 1.) - step(p + s, uv));
    return min(stripe.x, stripe.y);
}

float rectEdge(vec2 uv, vec2 p, vec2 s, float b) {
    return clamp(rect(uv, p, s) - rect(uv, p + b, s - 2. * b), 0., 1.);
}

void main() {
    float f = (gl_FragCoord.x - u_position.x) / u_size.x; // global x position [0.0,1.0]
    int i = int(f * float(VALUES - 1)); // left value index [0, VALUES - 1]
    float k = f * float(VALUES - 1) - float(i); // local x pos (btwn samples) [0.0, 1.0]
    float v = mix(u_value[i], u_value[i + 1], k); // value for current position [0.0, 1.0]
    float border = rectEdge(gl_FragCoord.xy, u_position, u_size, 1.);
    gl_FragColor = vec4(vec3(mix(smoothstep(0., 4., v * u_size.y - (gl_FragCoord.y - u_position.y)), 1., border)), .75 * u_opacity);
}

)"
