#include "LogConsole.h"
#include "log.h"

LogConsole::LogConsole()
    : programObject(GL_INVALID_VALUE),
    posALoc(GL_INVALID_VALUE),
    posLoc(GL_INVALID_VALUE),
    sizLoc(GL_INVALID_VALUE),
    colLoc(GL_INVALID_VALUE),
    opaLoc(GL_INVALID_VALUE) {
  initialize();
}

LogConsole::~LogConsole() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void LogConsole::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";

  const GLchar* fShaderTexStr =  
    "precision highp float;                                                            \n"
    "                                                                                  \n"
    "#define FG vec4(u_color, 0.75 * u_opacity)                                        \n"
    "#define BG vec4(0.0, 0.0, 0.0, 0.25 * u_opacity)                                  \n"
    "                                                                                  \n"
    "const int VALUES = 100;                                                           \n"
    "                                                                                  \n"
    "uniform vec2 u_position;                                                          \n"
    "uniform vec2 u_size;                                                              \n"
    "uniform vec3 u_color;                                                             \n"
    "uniform float u_opacity;                                                          \n"
    "                                                                                  \n"
    "void main()                                                                       \n"
    "{                                                                                 \n"
    "  gl_FragColor = BG;                                                              \n"
    "                                                                                  \n"
    "  if(gl_FragCoord.y <= u_position.y + 1.0                                         \n"
    "  || gl_FragCoord.y >= u_position.y + u_size.y - 1.0                              \n"
    "  || gl_FragCoord.x <= u_position.x + 1.0                                         \n"
    "  || gl_FragCoord.x >= u_position.x + u_size.x - 1.0                              \n"
    "  )                                                                               \n"
    "    gl_FragColor = vec4(u_color, 0.75 * u_opacity);                               \n"
    "}                                                                                 \n";

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

  posALoc = glGetAttribLocation(programObject, "a_position");
  posLoc = glGetUniformLocation(programObject, "u_position");
  sizLoc = glGetUniformLocation(programObject, "u_size");
  colLoc = glGetUniformLocation(programObject, "u_color");
  opaLoc = glGetUniformLocation(programObject, "u_opacity");
}

void LogConsole::render(Text &text, std::pair<int, int> viewport, std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize) {
  float down  = static_cast<float>(position.second) / static_cast<float>(viewport.second) * 2.0f - 1.0f;
  float top   = (static_cast<float>(position.second) + static_cast<float>(size.second)) / static_cast<float>(viewport.second) * 2.0f - 1.0f;
  float left  = static_cast<float>(position.first) / static_cast<float>(viewport.first) * 2.0f - 1.0f;
  float right = (static_cast<float>(position.first) + static_cast<float>(size.first)) / static_cast<float>(viewport.first) * 2.0f - 1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  glUseProgram(programObject);

  glEnableVertexAttribArray(posALoc);
  glVertexAttribPointer(posALoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  glUniform2f(posLoc, static_cast<float>(position.first), static_cast<float>(position.second));
  glUniform2f(sizLoc, static_cast<float>(size.first), static_cast<float>(size.second));
  glUniform3f(colLoc, 1.0f, 1.0f, 1.0f);

  glUniform1f(opaLoc, 1.0f);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  glUseProgram(0);

  renderLogs(text, viewport, position, size, fontId, fontSize);
}

void LogConsole::checkShaderCompileError(GLuint shader) {
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

void LogConsole::renderLogs(Text &text, std::pair<int, int> viewport, std::pair<int, int> position, std::pair<int, int> size, int fontId, int fontSize) {
  std::pair<int, int> margin = {4, 10};
  std::pair<int, int> textSizeLimits = {size.first - 2 * margin.first, fontSize};

  // cleanup
  std::deque<std::string>::reverse_iterator rit = logs.rbegin();
  for(int offset = margin.second; rit != logs.rend(); ++rit) {
    std::pair<float, float> textSize = text.getTextSize(*rit,
                                                    textSizeLimits,
                                                    fontId,
                                                    viewport);
    textSize = {textSize.first * static_cast<float>(viewport.first) / 2.0f,
                textSize.second * static_cast<float>(viewport.first) / 2.0f};
    if(offset + textSize.second + margin.second > size.second)
      break;
    offset += textSize.second + margin.second;
  }
  if(rit != logs.rend())
    logs.erase(logs.begin(), ++rit.base());

  // render
  std::deque<std::string>::iterator deqit = logs.begin();
  for(int offset = margin.second; deqit != logs.end(); ++deqit) {
    std::pair<float, float> textSize = text.getTextSize(*deqit,
                                                    textSizeLimits,
                                                    fontId,
                                                    viewport);
    textSize = {textSize.first * static_cast<float>(viewport.first) / 2.0f,
                textSize.second * static_cast<float>(viewport.first) / 2.0f};
    if(offset + textSize.second + margin.second > size.second)
      break;
    text.render(*deqit,
                {position.first + margin.first, position.second + size.second - offset - textSize.second},
                textSizeLimits,
                viewport,
                fontId,
                {1.0f, 1.0f, 1.0f, 1.0f},
                true);
    offset += textSize.second + margin.second;
  }
}

void LogConsole::pushLog(std::string log) {
  logs.push_back(log);
}

