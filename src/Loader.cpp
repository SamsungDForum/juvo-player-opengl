#include "../include/Loader.h"

Loader::Loader()
  : programObject(GL_INVALID_VALUE),
  param(0)
{
  initialize();
}

Loader::~Loader() {
}

bool Loader::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";
 /*
  const GLchar* fShaderTexStr =  
    "precision highp float;       \n"
    "uniform float u_time;          \n"
    "uniform float u_opacity;       \n"
    "void main()                    \n"
    "{                              \n"
    "   vec2 res = vec2(1920.0, 1080.0);  \n"
    "   vec2 uv = gl_FragCoord.xy / res; \n"
    "   vec3 col = 0.5 + 0.5 * cos(vec3(u_time) + uv.xyx + vec3(0,2,4)); \n"
    "   gl_FragColor = vec4(col, u_opacity);   \n"
    "}                              \n";

  const GLchar* fShaderTexStr1 =  
    "precision highp float;       \n"
    "uniform float u_time;          \n"
    "uniform float u_opacity;       \n"
    "void main()                    \n"
    "{                              \n"
    "   vec3 col = vec3(0.0);       \n"
    "   int AA = 2;                 \n"
    "   vec2 res = vec2(1920.0, 1080.0); \n"
    "   for(int m = 0; m < AA; m++)   \n"
    "     for(int n = 0; n < AA; n++) \n"
    "     {                         \n"
    "       vec2 p = -1.0 + 2.0 * (gl_FragCoord.xy + vec2(float(m), float(n)) / float(AA)) / res.xy; \n"
    "       p.x *= res.x / res.y;   \n"
    "       float zoo = 1.0 / 250.0;\n"
    "       zoo = 1.0 / (400.0 - 150.0 * sin(0.15 * u_time - 0.3)); \n"
    "       vec2 cc = vec2(-0.533516, 0.526141) + p * zoo; \n"
    "       vec2 t2c = vec2(-0.5, 2.0); \n"
    "       t2c += 0.5 * vec2(cos(0.13 * (u_time - 10.0)), sin(0.13 * (u_time - 10.0))); \n"
    "                               \n"
    "       vec2 z  = vec2(0.0);    \n"
    "       vec2 dz = vec2(0.0);    \n"
    "       float trap1 = 0.0;      \n"
    "       float trap2 = 1e20;     \n" // ???
    "       float co2 = 0.0;        \n"
    "       for(int i = 0; i < 150; i++) \n"
    "       {                       \n"
    "         if(dot(z, z) > 1024.0)\n"
    "           break;              \n"
    "         dz = 2.0 * vec2(z.x * dz.x - z.y * dz.y, z.x * dz.y + z.y * dz.x) + vec2(1.0, 0.0); \n"
    "         z = cc + vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y); \n"
    "         float d1 = abs(dot(z - vec2(0.0, 1.0), vec2(0.707)));  \n"
    "         float ff = step(d1, 1.0);                              \n"
    "         co2 += ff;            \n"
    "         trap1 += ff * d1;     \n"
    "         trap2 = min(trap2, dot(z - t2c, z - t2c)); \n"
    "       }                       \n"
    "       float d = sqrt(dot(z, z) / dot(dz, dz)) * log(dot(z, z)); \n"
    "       float c1 = pow(clamp(2.00 * d / zoo, 0.0, 1.0), 0.5);     \n"
    "       float c2 = pow(clamp(1.5 * trap1 / co2, 0.0, 1.0), 2.0);  \n"
    "       float c3 = pow(clamp(0.4 * trap2, 0.0, 1.0), 0.25);       \n"
    "       vec3 col1 = 0.5 + 0.5 * sin(3.0 + 4.0 * c2 + vec3(0.0, 0.5, 1.0)); \n"
    "       vec3 col2 = 0.5 + 0.5 * sin(4.1 + 2.0 * c3 + vec3(1.0, 0.5, 0.0)); \n"
    "       col += 2.0 * sqrt(c1 * col1 * col2);                      \n"
    "     }                        \n"
    "     col /= float(AA * AA);   \n"
    "   gl_FragColor = vec4(col, 1.0); \n"
    "}                              \n";

  const GLchar* fShaderTexStr2 =  
    "precision highp float;       \n"
    "uniform float u_time;          \n"
    "uniform float u_param;         \n"
    "uniform float u_opacity;       \n"
    "void main()                    \n"
    "{                              \n"
    "   vec2 res = vec2(1920.0, 1080.0);  \n"
    "   vec2 uv = gl_FragCoord.xy / res; \n"
    "   vec3 col1 = 0.5 + 0.5 * cos(vec3(u_time) + uv.xyx + vec3(0,2,4)); \n"
    "   float pc = 100.0 * gl_FragCoord.x / res.x; \n"
    "   vec3 col = pc <= u_param ? col1 : vec3((col1.x + col1.y + col1.z) / 3.0); \n"
    "   gl_FragColor = vec4(col, u_opacity);   \n"
    "}                              \n";
*/
  const GLchar* fShaderTexStr3 =  
    "precision mediump float;                                                    \n"
    "uniform float u_time;                                                       \n"
    "uniform float u_param;                                                      \n"
    "uniform float u_opacity;                                                    \n"
    "                                                                            \n"
    "#define LIM 0.001                                                           \n"
    "#define THICK 0.1                                                           \n"
    "#define BORDER 0.01                                                         \n"
    "                                                                            \n"
    "const vec4 barColor = vec4(1.0, 1.0, 1.0, 1.0);                             \n"
    "const vec4 borderColor = vec4(1.0, 1.0, 1.0, 1.0);                          \n"
    "                                                                            \n"
    "float udBox(vec2 p, vec2 b) {                                               \n"
    "  return length(max(abs(p) - b, 0.0));                                      \n"
    "}                                                                           \n"
    "                                                                            \n"
    "vec2 sincos(float x) {                                                      \n"
    "  return vec2(sin(x), cos(x));                                              \n"
    "}                                                                           \n"
    "                                                                            \n"
    "vec2 rotate2d(vec2 uv, float phi) {                                         \n"
    "  vec2 t = sincos(phi);                                                     \n"
    "  return vec2(uv.x * t.y - uv.y * t.x, uv.x * t.x + uv.y * t.y);            \n"
    "}                                                                           \n"
    "                                                                            \n"
    "                                                                            \n"
    "vec4 loader(vec2 uv, float t) {                                             \n"
    "  if(udBox(uv, vec2(1.0, THICK + BORDER)) < LIM                             \n"
    "     && udBox(uv, vec2(1.0 - BORDER, THICK - BORDER)) > LIM)                \n"
    "    return borderColor;                                                     \n"
    "  if(udBox(uv + vec2(1.0 - t, 0.0),                                         \n"
    "     vec2(t - BORDER * 2.0, THICK - BORDER * 3.0)) < LIM)                   \n"
    "    return barColor;                                                        \n"
    "  return vec4(0.0);                                                         \n"
    "}                                                                           \n"
    "                                                                            \n"
    "void main()                                                                 \n"
    "{                                                                           \n"
    "  vec2 res = vec2(1920.0, 1080.0);                                          \n"
    "  float rat = res.x / res.y;                                                \n"
    "                                                                            \n"
    "  vec2 uv = 2.0 * (gl_FragCoord.xy / res.y) - vec2(rat, 1.0);               \n"
    "  vec4 col = loader(uv * (1.8 / vec2(rat, 1.0)), u_param / 100.0);          \n"
//    "  vec4 col = loader(rotate2d((uv - vec2(-0.403, 0.637)) * 2.32, -0.4347), u_param / 100.0);  \n"
//    "  vec4 col = loader(rotate2d((uv - vec2(0.0, 0.0)), sin(u_time) / 5.0), u_param / 100.0);  \n"
    "  gl_FragColor = vec4(col.rgb, u_opacity);                                  \n"
//    "  gl_FragColor = vec4(mix(gl_FragColor.rgb, col.rgb, col.a), u_opacity);    \n"
    "                                                                            \n"
//    "  // pulse bar                                                              \n"
//    "  float dist = length(gl_FragCoord.y / res.y - 0.3); // y coord - position  \n"
//    "  float power = 80.0 + 30.0 * sin(u_time * 3.0);                            \n"
//    "  float pulseCol = pow(1.0 - dist, power);                                  \n"
//    "  gl_FragColor = vec4(max(gl_FragColor.xyz, vec3(pulseCol)), 1.0);          \n"
    "}                                                                           \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);
  checkShaderCompileError(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr3, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompileError(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  //glBindAttribLocation(programObject, 0, "a_position");
  //glViewport(0, 0, viewport.first, viewport.second);

  time = std::chrono::high_resolution_clock::now();

  return true;
}

void Loader::checkShaderCompileError(GLuint shader) {
  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    _ERR("%s", (std::string(errorLog.begin(), errorLog.end()).c_str()));
    //_DBG("test dbg");
    //i_INFO("test info");

    glDeleteShader(shader); // Don't leak the shader.
  }
}

void Loader::setValue(int value) {
  TileAnimation::Easing easing = animation.isActive() ? TileAnimation::Easing::CubicOut : TileAnimation::Easing::CubicInOut;
  animation = TileAnimation(std::chrono::high_resolution_clock::now(),
                            std::chrono::milliseconds(500),
                            std::chrono::milliseconds(0),
                            {0, 0},
                            {0, 0},
                            TileAnimation::Easing::Linear,
                            0,
                            0,
                            TileAnimation::Easing::Linear,
                            {0, 0},
                            {0, 0},
                            TileAnimation::Easing::Linear,
                            param,
                            value,
                            easing);

  param = value;
}

void Loader::render(Text &text) {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> timespan = now - time;

  float down = -1.0f;
  float top = 1.0f;
  float left = -1.0f;
  float right = 1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 }; // TODO: Move it to VBO... Or somewhene in GPU's memory.

  glUseProgram(programObject);

  //int factor = 16;

  //glViewport(0, 0, 1920, 1080);

  GLint posLoc = glGetAttribLocation(programObject, "a_position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  GLfloat t = static_cast<float>(timespan.count()) / 1000.0f;
  GLuint timeLoc = glGetUniformLocation(programObject, "u_time"); // TODO: Store the location somewhere.
  glUniform1f(timeLoc, t);

  float paramArg = param;
  if(animation.isActive()) {
    std::pair<int, int> fakePosition, fakeSize;
    float fakeZoom;
    animation.update(fakePosition, fakeZoom, fakeSize, paramArg);
  }
  GLuint paramLoc = glGetUniformLocation(programObject, "u_param"); // TODO: Store the location somewhere.
  glUniform1f(paramLoc, paramArg);

  GLuint opacityLoc = glGetUniformLocation(programObject, "u_opacity"); // TODO: Store the location somewhere.
  glUniform1f(opacityLoc, 1.0f);

  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glEnable(GL_BLEND);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  //glViewport(0, 0, 1920, 1080);

  glUseProgram(0);
  {
    std::pair<int, int> viewport = {1920, 1080};
    int fontHeight = 48;
    int textLeft = (viewport.first - text.getTextSize("Loading... 0%", {0, fontHeight}, 0, viewport).first * viewport.first / 2.0) / 2;
    int textDown = viewport.second / 2 - 100;
    text.render(std::string("Loading... ") + std::to_string(static_cast<int>(param)) + std::string("%"), {textLeft, textDown}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, 1.0}, true);

    fontHeight *= 1.5;
    textLeft = (viewport.first - text.getTextSize("JuvoPlayer", {0, fontHeight}, 0, viewport).first * viewport.first / 2.0) / 2;
    textDown = viewport.second / 2 + 100;

    float d = (int)t > 0 && ((int)t + 1) % 2 == 0 ? t - (int)t : 0;
    float a = (cos(d * 20 / M_PI) + 1.0) / 2.0;
    text.render("JuvoPlayer", {textLeft, textDown}, {0, fontHeight}, viewport, 0, {1.0, 1.0, 1.0, a}, true);
  }
}


