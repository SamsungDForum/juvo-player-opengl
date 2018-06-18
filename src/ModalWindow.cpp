#include "ModalWindow.h"

ModalWindow::ModalWindow()
    : programObject(GL_INVALID_VALUE),
    posALoc(GL_INVALID_VALUE),
    posLoc(GL_INVALID_VALUE),
    sizLoc(GL_INVALID_VALUE),
    colLoc(GL_INVALID_VALUE),
    opaLoc(GL_INVALID_VALUE) {
  initialize();
}

ModalWindow::~ModalWindow() {
  if(programObject != GL_INVALID_VALUE)
    glDeleteProgram(programObject);
}

void ModalWindow::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";

  const GLchar* fShaderTexStr =  
    "precision highp float;                                                            \n"
    "                                                                                  \n"
    "#define FG vec4(u_color, 0.9 * u_opacity)                                        \n"
    "#define BG vec4(0.0, 0.0, 0.0, 0.9 * u_opacity)                                  \n"
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

void ModalWindow::render(Text &text, std::pair<int, int> viewport, int fontId) {
  if(!visible)
    return;

  int windowWidth = viewport.first / 2;
  std::pair<int,int> margin = {10, 10};

  std::pair<int, int> size = {windowWidth,
                              viewport.second / 2.5};
  std::pair<int, int> position = {(viewport.first - size.first) / 2,
                                  (viewport.second - size.second) / 2};

  renderRectangle(position, size, viewport);

  Params params {
    .text = text,
    .title = TextParams {
      .fontId = fontId,
      .fontSize = 52,
      .text = title,
      .position = {0, 0},
      .size = {1, 1},
      .color = {1.0f, 1.0f, 1.0f, 1.0f}
    },
    .body = TextParams {
      .fontId = fontId,
      .fontSize = 26,
      .text = body,
      .position = {0, 0},
      .size = {1, 1},
      .color = {1.0f, 1.0f, 1.0f, 1.0f}
    },
    .lineWidth = size.first - 2 * margin.first,

    .viewport = viewport,
    .window = WindowParams {
      .position = position,
      .size = size,
      .margin = margin
    },

    .buttonWindow = WindowParams {
      .position = {0, 0},
      .size = {1, 1},
      .margin = {0, 0}
    },
    .buttonText = TextParams {
      .fontId = fontId,
      .fontSize = 26,
      .text = button,
      .position = {0, 0},
      .size = {1, 1},
      .color = {1.0f, 1.0f, 1.0f, 1.0f}
    }
  };

  calculateElementsPositions(params);
  renderContent(params);
}

void ModalWindow::renderRectangle(std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport) {
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
}

void ModalWindow::renderContent(Params &params) {
  renderTitle(params);
  renderBody(params);
  renderButton(params);
}

void ModalWindow::calculateElementsPositions(Params &params) {
  params.title.size = getTextSize(params.text,
                                  params.title.text,
                                  params.lineWidth,
                                  params.title.fontSize,
                                  params.title.fontId,
                                  params.viewport);
  params.body.size = getTextSize(params.text,
                                 params.body.text,
                                 params.lineWidth,
                                 params.body.fontSize,
                                 params.body.fontId,
                                 params.viewport);
  params.title.position = {params.window.position.first + (params.window.size.first - params.title.size.first) / 2,
                           (params.window.position.second + params.window.size.second) - params.window.margin.second - params.title.fontSize};
  params.body.position = {params.window.position.first + (params.window.size.first - params.body.size.first) / 2,
                          params.window.position.second + (params.window.size.second + params.body.size.second) / 2 - params.body.fontSize};
  params.buttonText.size = getTextSize(params.text,
                                       params.buttonText.text,
                                       params.lineWidth,
                                       params.buttonText.fontSize,
                                       params.buttonText.fontId,
                                       params.viewport);
  params.buttonWindow.size = {params.buttonText.size.first + 4 * params.buttonText.fontSize,
                              params.buttonText.size.second + 2 * params.buttonText.fontSize};
  params.buttonWindow.position = {params.window.position.first + (params.window.size.first - params.buttonWindow.size.first) / 2,
                                  params.window.position.second + params.window.margin.second};
  params.buttonText.position = {params.buttonWindow.position.first + (params.buttonWindow.size.first - params.buttonText.size.first) / 2,
                                params.buttonWindow.position.second + (params.buttonWindow.size.second + params.buttonText.size.second) / 2 - params.buttonText.fontSize};
}

void ModalWindow::renderTitle(Params &params) {
  params.text.render(params.title.text,
                     params.title.position,
                     {params.lineWidth, params.title.fontSize},
                     params.viewport,
                     params.title.fontId,
                     params.title.color,
                     true);
}

void ModalWindow::renderBody(Params &params) {
  params.text.render(params.body.text,
                     params.body.position,
                     {params.lineWidth, params.body.fontSize},
                     params.viewport,
                     params.body.fontId,
                     params.body.color,
                     true);
}

void ModalWindow::renderButton(Params &params) {
  renderRectangle(params.buttonWindow.position,
                  params.buttonWindow.size,
                  params.viewport);
  params.text.render(params.buttonText.text,
                     params.buttonText.position,
                     {params.lineWidth, params.buttonText.fontSize},
                     params.viewport,
                     params.buttonText.fontId,
                     params.buttonText.color,
                     true);
}

std::pair<int, int> ModalWindow::getTextSize(Text &text, std::string s, int lineWidth, int fontHeight, int fontId, std::pair<int, int> viewport) {
  std::pair<float, float> size = text.getTextSize(s,
                                                  {lineWidth, fontHeight},
                                                  fontId,
                                                  viewport);
  return {size.first * static_cast<float>(viewport.first) / 2.0f,
          size.second * static_cast<float>(viewport.second) / 2.0f};
}

void ModalWindow::show(std::string title, std::string body, std::string button) {
  this->title = title;
  this->body = body;
  this->button = button;
  visible = true;
}

void ModalWindow::hide() {
  visible = false;
}

bool ModalWindow::isVisible() {
  return visible;
}

void ModalWindow::checkShaderCompileError(GLuint shader) {
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

