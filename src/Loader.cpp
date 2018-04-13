#include "../include/Loader.h"

Loader::Loader()
  : programObject(GL_INVALID_VALUE),
    param(0)
{
  initialize();
}

Loader::~Loader() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

bool Loader::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";

  const GLchar* fShaderTexStr =  
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
    "  gl_FragColor = vec4(col.rgb, u_opacity);                                  \n"
    "                                                                            \n"
    "}                                                                           \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);
  checkShaderCompileError(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr, NULL);
  glCompileShader(fragmentShader);
  checkShaderCompileError(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  posLoc = glGetAttribLocation(programObject, "a_position");
  timeLoc = glGetUniformLocation(programObject, "u_time");
  paramLoc = glGetUniformLocation(programObject, "u_param");
  opacityLoc = glGetUniformLocation(programObject, "u_opacity");

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

    glDeleteShader(shader);
  }
}

void Loader::setValue(int value) {
  Animation::Easing easing = animation.isActive() ? Animation::Easing::CubicOut : Animation::Easing::CubicInOut;
  animation = Animation(std::chrono::high_resolution_clock::now(),
                        std::chrono::milliseconds(500),
                        std::chrono::milliseconds(0),
                        {static_cast<double>(param)},
                        {static_cast<double>(value)},
                        easing);
  param = value;
}

void Loader::render(Text &text) {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> timespan = now - time;

  float down  = -1.0f;
  float top   =  1.0f;
  float left  = -1.0f;
  float right =  1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);

  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  GLfloat t = static_cast<float>(timespan.count()) / 1000.0f;
  glUniform1f(timeLoc, t);

  float paramArg = param;
  if(animation.isActive()) {
    std::vector<double> updated = animation.update();
    if(!updated.empty())
      paramArg = static_cast<float>(updated[0]);
  }
  
  glUniform1f(paramLoc, paramArg);

  glUniform1f(opacityLoc, 1.0f);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glUseProgram(0);

  { // render text
    std::pair<int, int> viewport = {1920, 1080};
    int fontHeight = 48;
    int textLeft = (viewport.first - text.getTextSize("Loading... 0%", {0, fontHeight}, 0, viewport).first * viewport.first / 2.0) / 2;
    int textDown = viewport.second / 2 - 100;
    text.render(std::string("Loading... ") + std::to_string(static_cast<int>(param)) + std::string("%"),
                {textLeft, textDown},
                {0, fontHeight},
                viewport,
                0,
                {1.0, 1.0, 1.0, 1.0},
                true);

    fontHeight *= 1.5;
    textLeft = (viewport.first - text.getTextSize("JuvoPlayer", {0, fontHeight}, 0, viewport).first * viewport.first / 2.0) / 2;
    textDown = viewport.second / 2 + 100;

    float d = (int)t > 0 && ((int)t + 1) % 2 == 0 ? t - (int)t : 0;
    float a = (cos(d * 20 / M_PI) + 1.0) / 2.0;
    text.render("JuvoPlayer",
                {textLeft, textDown},
                {0, fontHeight},
                viewport,
                0,
                {1.0, 1.0, 1.0, a},
                true);
  }
}


