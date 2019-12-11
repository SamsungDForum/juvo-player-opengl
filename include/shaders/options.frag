R"(

precision highp float;

uniform vec2 u_size;
uniform vec2 u_position;
uniform vec3 u_color;
uniform float u_opacity;
uniform float u_frameWidth;
uniform vec3 u_frameColor;

float rect(vec2 uv, vec2 p, vec2 s) {
    vec2 stripe = min(step(p, uv), vec2(1., 1.) - step(p + s, uv));
    return min(stripe.x, stripe.y);
}

float rectEdge(vec2 uv, vec2 p, vec2 s, float b) {
    return clamp(rect(uv, p, s) - rect(uv, p + b, s - 2. * b), 0., 1.);
}

void main() {
    float border = rectEdge(gl_FragCoord.xy, u_position, u_size, u_frameWidth);
    gl_FragColor = vec4(mix(u_color, u_frameColor, border), .75 * u_opacity);
}

)"
